/*
 * Copyright 1988,1990,1993,1994 by Paul Vixie
 * All rights reserved
 */

/*
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1997,2000 by Internet Software Consortium, Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Id: entry.c,v 1.17 2004/01/23 18:56:42 vixie Exp $";
#endif
#define __SC_BUILD__
/* vix 26jan87 [RCS'd; rest of log is in RCS file]
 * vix 01jan87 [added line-level error recovery]
 * vix 31dec86 [added /step to the from-to range, per bob@acornrc]
 * vix 30dec86 [written]
 */

#include <string.h>
//#include <utility.h>
#include "cron.h"
#include "funcs.h"

typedef	enum ecode {
	e_none, e_minute, e_hour, e_dom, e_month, e_dow,
	e_cmd, e_timespec, e_username, e_option, e_memory
} ecode_e;

typedef	struct _once_config {
	struct _once_config	*next;
	s_entry		*se;
} once_config;

typedef	struct _lock_event {
	struct _lock_event *next;
	s_entry		*se;
} lock_event;

static once_config *oc_head = NULL, *oc_tail = NULL;
static lock_event *le_head = NULL, *le_tail = NULL;

static const char *ecodes[] =
	{
		"no error",
		"bad minute",
		"bad hour",
		"bad day-of-month",
		"bad month",
		"bad day-of-week",
		"bad command",
		"bad time specifier",
		"bad username",
		"bad option",
		"out of memory"
	};

static int	get_list(bitstr_t *, int, int, const char *[], int, FILE *),
		get_range(bitstr_t *, int, int, const char *[], int, FILE *),
		get_number(int *, int, const char *[], int, FILE *, const char *),
		set_element(bitstr_t *, int, int, int);

void
free_entry(entry *e) {
	free(e->cmd);
	free(e->pwd);
	env_free(e->envp);
	free(e);
}

static int
sentry_get_list(bitstr_t *bits, int low, int high, 
                        const char *names[], char *buf)
{
	int i, num1, num2, num3;
	bit_nclear(bits, 0, (high-low+1));

	/* process all ranges
	 */
    while (*buf)
    {
        if (*buf == ',') /* , blank[not * num] */
            buf++;
        else
        {  
	        if (*buf == '*') 
            {   
        		num1 = low;
        		num2 = high;
                buf++;
    	    }
            else
            {
                num1 = atoi(buf);
                while (isdigit(*buf))
                {
                    buf++;
                }  	

        		if (*buf != '-')/*, blank \0*/
                {
        			set_element(bits, low, high, num1);
                    continue;
    		    }
                else
                { 
                    buf++;
                    num2 = atoi(buf);
                    while (isdigit(*buf))
                    {
                        buf++;
                    }
                    if (num1 > num2)
                        continue;
    		    }                
            }

        	if (*buf == '/') 
            {
        		buf++;
        		num3 = atoi(buf);
                while (isdigit(*buf))
                    buf++;
        		if (num3 == 0)
        			continue;
        	}
            else 
            {
        		num3 = 1;
        	}

        	for (i = num1;  i <= num2;  i += num3)
        		set_element(bits, low, high, i);                
        }
    }
    return 0;
}


static long convert_t2l(char *time)
{
    char *p = NULL;
    int hh, mm;
    int ret = 0;
    
    p = strchr(time, ':');
    if (p)
    {
        hh = atoi(time);
        mm = atoi(p+1);
        ret = hh * 60 + mm;
    }

    return ret;     
}

/* compare time, format hh:mm */
static int compare_hm(char *ta, char *tb)
{
    long la, lb;
    la = convert_t2l(ta);
    lb = convert_t2l(tb);

    if (la == lb)
        return 0;
    else if (la > lb)
        return 1;
    else
        return -1;
}

static int is_in_range(char *now, char *stime, char *etime)
{
    if ( 
        ( (compare_hm(stime, etime) <= 0) 
            && (compare_hm(now, stime) >= 0) 
            && (compare_hm(now, etime) <= 0) ) /* 01:00 < now < 02:00 */ 
        || 
        ( (compare_hm(stime, etime) >= 0) && 
            !((compare_hm(now, stime) < 0) &&
            (compare_hm(now, etime) > 0) ) ) /* 02:00 < now < 01:00 */ 
       )
        return 1;
    else
        return 0;

}

static int copy_entry(entry *e, struct passwd *pw, char **envp,
                        char *group, char *cmd)
{
    if (envp)
        e->envp = env_copy(envp);

    if (e->envp == NULL)
    {
        Debug(DPARS, ("copy_entry() envp dup fail\n"))
        goto err;    
    }

    if ((e->cmd = strdup(cmd)) == NULL) 
    {
        Debug(DPARS, ("copy_entry() cmd dup fail\n"))
        goto err;    
    }

    if ((e->group = strdup(group)) == NULL)
    {
        Debug(DPARS, ("copy_entry() dup group fail\n"))
        goto err;    
    }

    if ((e->pwd = pw_dup(pw)) == NULL) 
    {
        Debug(DPARS, ("copy_entry() pw_dup fail\n"))
        goto err;    
	}
	bzero(e->pwd->pw_passwd, strlen(e->pwd->pw_passwd));
    return 0;    
    
err:
    return -1;
}

/* delete group  & add command to jobs */
entry *load_flush_entry(s_entry *sentry, user *u, struct passwd *pw, char **envp)
{
    entry *e, *ne, *pe;
    entry *flush_e = NULL;
    
    Debug(DPARS, ("load_flush_entry() enter\n"))
        
    /* delete group */
    
    for (e = u->crontab, pe = u->crontab; e != NULL; e = ne)
    {
        ne = e->next;
        if (!strcmp(e->group, sentry->group))
        {
            Debug(DPARS, ("delete e cmd=%s\n", e->cmd))
            if (e == u->crontab)
            {
                u->crontab = ne;
                pe = ne;
            }
            else
            {
                pe->next = ne;
            }

            free_entry(e);            
        }
        else
        {
            pe = e;
        }
    }

    /* add command to jobs, if cmd is null, maybe not need to add command to jobs */
    if (strcmp(sentry->data.flush.cmd, ""))
    {
        flush_e = (entry *) calloc(sizeof(entry), sizeof(char));
        if (copy_entry(flush_e, pw, envp, sentry->group, sentry->data.flush.cmd) < 0)
        {
            free_entry(flush_e);
            flush_e = NULL;
        }
    }   

    return flush_e;
}



entry *load_once_entry(s_entry *sentry, struct passwd *pw, char **envp)
{
    entry *e = NULL;
    time_t now;
    struct tm *tm;
    char tmp[16] = "";    
    char *cmd = NULL;
    char *group = NULL;

    now = time(NULL);
 
#ifdef __SC_BUILD__
    tm = localtime(&now);
#else 
    tm = gmtime(&now);
#endif   
    if (tm->tm_wday == 0)
        snprintf(tmp, sizeof(tmp), "7"); /* because sunday is 0, format convert */
    else
        snprintf(tmp, sizeof(tmp), "%d", tm->tm_wday); /* day of week */
    
    if (strcmp(sentry->data.once.dow, "*") && !strstr(sentry->data.once.dow, tmp))
        return NULL;
    sprintf(tmp, "%d:%d", tm->tm_hour, tm->tm_min);

    /* check sentry stime & etime format */
    if (is_in_range(tmp, sentry->data.once.stime, sentry->data.once.etime))
    {
        if (*(sentry->data.once.in_cmd) != '\0')
            cmd  = sentry->data.once.in_cmd;
    }
    else
    {
        if (*(sentry->data.once.out_cmd) != '\0')
            cmd = sentry->data.once.out_cmd;
    }

    group = sentry->group;

    if (cmd)
    {
        e = (entry *) calloc(sizeof(entry), sizeof(char));
        if (copy_entry(e, pw, envp, group, cmd) < 0)
        {
            free_entry(e);
            e = NULL;
        }
    }

    return e;
}
entry *load_circle_entry(s_entry *sentry, struct passwd *pw, char **envp)
{
    entry *e;
    e = (entry *) calloc(sizeof(entry), sizeof(char));    

    if (sentry->data.circle.min[0] == '*')
        	e->flags |= MIN_STAR;    
	sentry_get_list(e->minute, FIRST_MINUTE, LAST_MINUTE,
		      PPC_NULL, sentry->data.circle.min);


    if (sentry->data.circle.hour[0] == '*')
        	e->flags |= HR_STAR;
	sentry_get_list(e->hour, FIRST_HOUR, LAST_HOUR,
			      PPC_NULL, sentry->data.circle.hour);

    
    if (sentry->data.circle.dom[0] == '*')
        	e->flags |= DOM_STAR;    
    sentry_get_list(e->dom, FIRST_DOM, LAST_DOM,
			      PPC_NULL, sentry->data.circle.dom);

    /* [M] */
    sentry_get_list(e->month, FIRST_MONTH, LAST_MONTH,
			      MonthNames, sentry->data.circle.month);

    if (sentry->data.circle.dow[0] == '*')
        	e->flags |= DOW_STAR;
    sentry_get_list(e->dow, FIRST_DOW, LAST_DOW,
			      DowNames, sentry->data.circle.dow);

    if (bit_test(e->dow, 0) || bit_test(e->dow, 7)) {
		bit_set(e->dow, 0);
		bit_set(e->dow, 7);
	}

    if (copy_entry(e, pw, envp, sentry->group, sentry->data.circle.cmd) < 0)
    {
        free_entry(e);
        e = NULL;
    }
    return e;    
}


/* return NULL if eof or syntax error occurs;
 * otherwise return a pointer to a new entry.
 */
entry *
load_entry(FILE *file, void (*error_func)(), struct passwd *pw, char **envp) {
	/* this function reads one crontab entry -- the next -- from a file.
	 * it skips any leading blank lines, ignores comments, and returns
	 * NULL if for any reason the entry can't be read and parsed.
	 *
	 * the entry is also parsed here.
	 *
	 * syntax:
	 *   user crontab:
	 *	minutes hours doms months dows cmd\n
	 *   system crontab (/etc/crontab):
	 *	minutes hours doms months dows USERNAME cmd\n
	 */

	ecode_e	ecode = e_none;
	entry *e;
	int ch;
	char cmd[MAX_COMMAND];
	char envstr[MAX_ENVSTR];
	char **tenvp;

	Debug(DPARS, ("load_entry()...about to eat comments\n"))

	skip_comments(file);

	ch = get_char(file);
	if (ch == EOF)
		return (NULL);

	/* ch is now the first useful character of a useful line.
	 * it may be an @special or it may be the first character
	 * of a list of minutes.
	 */

	e = (entry *) calloc(sizeof(entry), sizeof(char));

	if (ch == '@') {
		/* all of these should be flagged and load-limited; i.e.,
		 * instead of @hourly meaning "0 * * * *" it should mean
		 * "close to the front of every hour but not 'til the
		 * system load is low".  Problems are: how do you know
		 * what "low" means? (save me from /etc/cron.conf!) and:
		 * how to guarantee low variance (how low is low?), which
		 * means how to we run roughly every hour -- seems like
		 * we need to keep a history or let the first hour set
		 * the schedule, which means we aren't load-limited
		 * anymore.  too much for my overloaded brain. (vix, jan90)
		 * HINT
		 */
		ch = get_string(cmd, MAX_COMMAND, file, " \t\n");
		if (!strcmp("reboot", cmd)) {
			e->flags |= WHEN_REBOOT;
		} else if (!strcmp("yearly", cmd) || !strcmp("annually", cmd)){
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_set(e->dom, 0);
			bit_set(e->month, 0);
			bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
			e->flags |= DOW_STAR;
		} else if (!strcmp("monthly", cmd)) {
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_set(e->dom, 0);
			bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
			bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
			e->flags |= DOW_STAR;
		} else if (!strcmp("weekly", cmd)) {
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_nset(e->dom, 0, (LAST_DOM-FIRST_DOM+1));
			bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
			bit_set(e->dow, 0);
			e->flags |= DOW_STAR;
		} else if (!strcmp("daily", cmd) || !strcmp("midnight", cmd)) {
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_nset(e->dom, 0, (LAST_DOM-FIRST_DOM+1));
			bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
			bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
		} else if (!strcmp("hourly", cmd)) {
			bit_set(e->minute, 0);
			bit_nset(e->hour, 0, (LAST_HOUR-FIRST_HOUR+1));
			bit_nset(e->dom, 0, (LAST_DOM-FIRST_DOM+1));
			bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
			bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
			e->flags |= HR_STAR;
		} else {
			ecode = e_timespec;
			goto eof;
		}
		/* Advance past whitespace between shortcut and
		 * username/command.
		 */
		Skip_Blanks(ch, file);
		if (ch == EOF || ch == '\n') {
			ecode = e_cmd;
			goto eof;
		}
	} else {
		Debug(DPARS, ("load_entry()...about to parse numerics\n"))

		if (ch == '*')
			e->flags |= MIN_STAR;
		ch = get_list(e->minute, FIRST_MINUTE, LAST_MINUTE,
			      PPC_NULL, ch, file);
		if (ch == EOF) {
			ecode = e_minute;
			goto eof;
		}

		/* hours
		 */

		if (ch == '*')
			e->flags |= HR_STAR;
		ch = get_list(e->hour, FIRST_HOUR, LAST_HOUR,
			      PPC_NULL, ch, file);
		if (ch == EOF) {
			ecode = e_hour;
			goto eof;
		}

		/* DOM (days of month)
		 */

		if (ch == '*')
			e->flags |= DOM_STAR;
		ch = get_list(e->dom, FIRST_DOM, LAST_DOM,
			      PPC_NULL, ch, file);
		if (ch == EOF) {
			ecode = e_dom;
			goto eof;
		}

		/* month
		 */

		ch = get_list(e->month, FIRST_MONTH, LAST_MONTH,
			      MonthNames, ch, file);
		if (ch == EOF) {
			ecode = e_month;
			goto eof;
		}

		/* DOW (days of week)
		 */

		if (ch == '*')
			e->flags |= DOW_STAR;
		ch = get_list(e->dow, FIRST_DOW, LAST_DOW,
			      DowNames, ch, file);
		if (ch == EOF) {
			ecode = e_dow;
			goto eof;
		}
	}

	/* make sundays equivalent */
	if (bit_test(e->dow, 0) || bit_test(e->dow, 7)) {
		bit_set(e->dow, 0);
		bit_set(e->dow, 7);
	}

	/* check for permature EOL and catch a common typo */
	if (ch == '\n' || ch == '*') {
		ecode = e_cmd;
		goto eof;
	}

	/* ch is the first character of a command, or a username */
	unget_char(ch, file);

	if (!pw) {
		char		*username = cmd;	/* temp buffer */

		Debug(DPARS, ("load_entry()...about to parse username\n"))
		ch = get_string(username, MAX_COMMAND, file, " \t\n");

		Debug(DPARS, ("load_entry()...got %s\n",username))
		if (ch == EOF || ch == '\n' || ch == '*') {
			ecode = e_cmd;
			goto eof;
		}

		pw = getpwnam(username);
		if (pw == NULL) {
			ecode = e_username;
			goto eof;
		}
		Debug(DPARS, ("load_entry()...uid %ld, gid %ld\n",
			      (long)pw->pw_uid, (long)pw->pw_gid))
	}

	if ((e->pwd = pw_dup(pw)) == NULL) {
		ecode = e_memory;
		goto eof;
	}
	bzero(e->pwd->pw_passwd, strlen(e->pwd->pw_passwd));

	/* copy and fix up environment.  some variables are just defaults and
	 * others are overrides.
	 */
	if ((e->envp = env_copy(envp)) == NULL) {
		ecode = e_memory;
		goto eof;
	}
	if (!env_get("SHELL", e->envp)) {
		if (glue_strings(envstr, sizeof envstr, "SHELL",
				 _PATH_BSHELL, '=')) {
			if ((tenvp = env_set(e->envp, envstr)) == NULL) {
				ecode = e_memory;
				goto eof;
			}
			e->envp = tenvp;
		} else
			log_it("CRON", getpid(), "error", "can't set SHELL");
	}
	if (!env_get("HOME", e->envp)) {
		if (glue_strings(envstr, sizeof envstr, "HOME",
				 pw->pw_dir, '=')) {
			if ((tenvp = env_set(e->envp, envstr)) == NULL) {
				ecode = e_memory;
				goto eof;
			}
			e->envp = tenvp;
		} else
			log_it("CRON", getpid(), "error", "can't set HOME");
	}
#ifndef LOGIN_CAP
	/* If login.conf is in used we will get the default PATH later. */
	if (!env_get("PATH", e->envp)) {
		if (glue_strings(envstr, sizeof envstr, "PATH",
				 _PATH_DEFPATH, '=')) {
			if ((tenvp = env_set(e->envp, envstr)) == NULL) {
				ecode = e_memory;
				goto eof;
			}
			e->envp = tenvp;
		} else
			log_it("CRON", getpid(), "error", "can't set PATH");
	}
#endif /* LOGIN_CAP */
	if (glue_strings(envstr, sizeof envstr, "LOGNAME",
			 pw->pw_name, '=')) {
		if ((tenvp = env_set(e->envp, envstr)) == NULL) {
			ecode = e_memory;
			goto eof;
		}
		e->envp = tenvp;
	} else
		log_it("CRON", getpid(), "error", "can't set LOGNAME");
#if defined(BSD) || defined(__linux)
	if (glue_strings(envstr, sizeof envstr, "USER",
			 pw->pw_name, '=')) {
		if ((tenvp = env_set(e->envp, envstr)) == NULL) {
			ecode = e_memory;
			goto eof;
		}
		e->envp = tenvp;
	} else
		log_it("CRON", getpid(), "error", "can't set USER");
#endif

	Debug(DPARS, ("load_entry()...about to parse command\n"))

	/* If the first character of the command is '-' it is a cron option.
	 */
	while ((ch = get_char(file)) == '-') {
		switch (ch = get_char(file)) {
		case 'q':
			e->flags |= DONT_LOG;
			Skip_Nonblanks(ch, file)
			break;
		default:
			ecode = e_option;
			goto eof;
		}
		Skip_Blanks(ch, file)
		if (ch == EOF || ch == '\n') {
			ecode = e_cmd;
			goto eof;
		}
	}
	unget_char(ch, file);

	/* Everything up to the next \n or EOF is part of the command...
	 * too bad we don't know in advance how long it will be, since we
	 * need to malloc a string for it... so, we limit it to MAX_COMMAND.
	 */ 
	ch = get_string(cmd, MAX_COMMAND, file, "\n");

	/* a file without a \n before the EOF is rude, so we'll complain...
	 */
	if (ch == EOF) {
		ecode = e_cmd;
		goto eof;
	}

	/* got the command in the 'cmd' string; save it in *e.
	 */
	if ((e->cmd = strdup(cmd)) == NULL) {
		ecode = e_memory;
		goto eof;
	}

	Debug(DPARS, ("load_entry()...returning successfully\n"))

	/* success, fini, return pointer to the entry we just created...
	 */
	return (e);

 eof:
	if (e->envp)
		env_free(e->envp);
	if (e->pwd)
		free(e->pwd);
	if (e->cmd)
		free(e->cmd);
	free(e);
	while (ch != '\n' && !feof(file))
		ch = get_char(file);
	if (ecode != e_none && error_func)
		(*error_func)(ecodes[(int)ecode]);
	return (NULL);
}

static int
get_list(bitstr_t *bits, int low, int high, const char *names[],
	 int ch, FILE *file)
{
	int done;

	/* we know that we point to a non-blank character here;
	 * must do a Skip_Blanks before we exit, so that the
	 * next call (or the code that picks up the cmd) can
	 * assume the same thing.
	 */

	Debug(DPARS|DEXT, ("get_list()...entered\n"))

	/* list = range {"," range}
	 */
	
	/* clear the bit string, since the default is 'off'.
	 */
	bit_nclear(bits, 0, (high-low+1));

	/* process all ranges
	 */
	done = FALSE;
	while (!done) {
		if (EOF == (ch = get_range(bits, low, high, names, ch, file)))
			return (EOF);
		if (ch == ',')
			ch = get_char(file);
		else
			done = TRUE;
	}

	/* exiting.  skip to some blanks, then skip over the blanks.
	 */
	Skip_Nonblanks(ch, file)
	Skip_Blanks(ch, file)

	Debug(DPARS|DEXT, ("get_list()...exiting w/ %02x\n", ch))

	return (ch);
}


static int
get_range(bitstr_t *bits, int low, int high, const char *names[],
	  int ch, FILE *file)
{
	/* range = number | number "-" number [ "/" number ]
	 */

	int i, num1, num2, num3;

	Debug(DPARS|DEXT, ("get_range()...entering, exit won't show\n"))

	if (ch == '*') {
		/* '*' means "first-last" but can still be modified by /step
		 */
		num1 = low;
		num2 = high;
		ch = get_char(file);
		if (ch == EOF)
			return (EOF);
	} else {
		ch = get_number(&num1, low, names, ch, file, ",- \t\n");
		if (ch == EOF)
			return (EOF);

		if (ch != '-') {
			/* not a range, it's a single number.
			 */
			if (EOF == set_element(bits, low, high, num1)) {
				unget_char(ch, file);
				return (EOF);
			}
			return (ch);
		} else {
			/* eat the dash
			 */
			ch = get_char(file);
			if (ch == EOF)
				return (EOF);

			/* get the number following the dash
			 */
			ch = get_number(&num2, low, names, ch, file, "/, \t\n");
			if (ch == EOF || num1 > num2)
				return (EOF);
		}
	}

	/* check for step size
	 */
	if (ch == '/') {
		/* eat the slash
		 */
		ch = get_char(file);
		if (ch == EOF)
			return (EOF);

		/* get the step size -- note: we don't pass the
		 * names here, because the number is not an
		 * element id, it's a step size.  'low' is
		 * sent as a 0 since there is no offset either.
		 */
		ch = get_number(&num3, 0, PPC_NULL, ch, file, ", \t\n");
		if (ch == EOF || num3 == 0)
			return (EOF);
	} else {
		/* no step.  default==1.
		 */
		num3 = 1;
	}

	/* range. set all elements from num1 to num2, stepping
	 * by num3.  (the step is a downward-compatible extension
	 * proposed conceptually by bob@acornrc, syntactically
	 * designed then implemented by paul vixie).
	 */
	for (i = num1;  i <= num2;  i += num3)
		if (EOF == set_element(bits, low, high, i)) {
			unget_char(ch, file);
			return (EOF);
		}

	return (ch);
}

static int
get_number(int *numptr, int low, const char *names[], int ch, FILE *file,
    const char *terms) {
	char temp[MAX_TEMPSTR], *pc;
	int len, i;

	pc = temp;
	len = 0;

	/* first look for a number */
	while (isdigit((unsigned char)ch)) {
		if (++len >= MAX_TEMPSTR)
			goto bad;
		*pc++ = ch;
		ch = get_char(file);
	}
	*pc = '\0';
	if (len != 0) {
		/* got a number, check for valid terminator */
		if (!strchr(terms, ch))
			goto bad;
		*numptr = atoi(temp);
		return (ch);
	}

	/* no numbers, look for a string if we have any */
	if (names) {
		while (isalpha((unsigned char)ch)) {
			if (++len >= MAX_TEMPSTR)
				goto bad;
			*pc++ = ch;
			ch = get_char(file);
		}
		*pc = '\0';
		if (len != 0 && strchr(terms, ch)) {
			for (i = 0;  names[i] != NULL;  i++) {
				Debug(DPARS|DEXT,
					("get_num, compare(%s,%s)\n", names[i],
					temp))
				if (!strcasecmp(names[i], temp)) {
					*numptr = i+low;
					return (ch);
				}
			}
		}
	}

bad:
	unget_char(ch, file);
	return (EOF);
}

static int
set_element(bitstr_t *bits, int low, int high, int number) {
	Debug(DPARS|DEXT, ("set_element(?,%d,%d,%d)\n", low, high, number))

	if (number < low || number > high)
		return (EOF);

	bit_set(bits, (number-low));
	return (OK);
}

void enqueue_once_config(s_entry *se)
{
    once_config *oc = NULL;
    s_entry *p_se = NULL;

    oc = (once_config*)malloc(sizeof(once_config));
    if (!oc)
        return;
    p_se = (s_entry*)malloc(sizeof(s_entry));
    if (!p_se)
    {
        free(oc);
        return;
    }
    memcpy(p_se, se, sizeof(s_entry));
    
    oc->next = NULL;
    oc->se = p_se;

    if (oc_head == NULL)
        oc_head = oc;
    else
        oc_tail->next = oc;

    oc_tail = oc;
}

void dequeue_once_config(s_entry *se)
{
    once_config *oc_j, *oc_tmp = NULL, *oc_jp = NULL;

    for (oc_j=oc_head; oc_j;)
    {
        oc_tmp = oc_j;
        oc_j = oc_j->next;
        
        if (!strcmp(oc_tmp->se->group, se->group))
        {
            if (oc_jp)
                oc_jp->next = oc_tmp->next;
            else
                oc_head = oc_tmp->next;
            
            free(oc_tmp->se);
            free(oc_tmp);
        }
        else
            oc_jp = oc_tmp;
    }

    oc_tail = oc_jp;
}

void reload_once_config(char *group, cron_db *db)
{
    user *u = NULL;
    entry *e = NULL;
    struct passwd* pw = NULL;
    once_config *oc_j;

    
    pw = getpwuid(0);           
    u = find_user(db, "*system*");            

    for (oc_j=oc_head; oc_j; oc_j=oc_j->next)
    {
        if (!strcmp(group, "*") || !strcmp(oc_j->se->group, group))
        {
            e = load_once_entry(oc_j->se, pw, sys_envp);
            if (e)
            {
                job_add(e, u);
                job_runqueue();
                free_entry(e);
            }
        } 
    }   
}
/*
1: in range
0: out range
 */
int query_job_status(char *group, char *id)
{
    int ret = 0;
    time_t t;
    struct tm *tm;
    char dow[16] = "";    
    char hm[16] = "";
    once_config *oc_j;

    t = time(NULL);
#ifdef __SC_BUILD__
    tm = localtime(&t);
#else
    tm = gmtime(&t);
#endif

    snprintf(dow, sizeof(dow), "%d", tm->tm_wday);
    snprintf(hm, sizeof(hm), "%d:%d", tm->tm_hour, tm->tm_min);
    
    for (oc_j=oc_head; oc_j; oc_j=oc_j->next)
    {
        if (!strcmp(oc_j->se->group, group))
        {
            if ( !strcmp(oc_j->se->data.once.dow, "*") || 
                    strstr(oc_j->se->data.once.dow, dow) )
            {
                if (is_in_range(hm, oc_j->se->data.once.stime, oc_j->se->data.once.etime))
                    ret = 1;
            }
            break;    
        }
    }
    return ret;
}

void enqueue_lock_event(s_entry *se)
{
    lock_event *le = NULL;
    s_entry *p_se = NULL;

    le = (lock_event*)malloc(sizeof(lock_event));
    if (!le)
        return;
    p_se = (s_entry*)malloc(sizeof(s_entry));
    if (!p_se)
    {
        free(le);
        return;
    }
    memcpy(p_se, se, sizeof(s_entry));
    
    le->next = NULL;
    le->se = p_se;

    if (le_head == NULL)
        le_head = le;
    else
        le_tail->next = le;

    le_tail = le;
}

void dequeue_lock_event(s_entry *se)
{
    lock_event *le_j, *le_tmp = NULL, *le_jp = NULL;

    for (le_j=le_head; le_j;)
    {
        le_tmp = le_j;
        le_j = le_j->next;
        if (!strcmp(le_tmp->se->group, se->group))
        {
            if (le_jp)
                le_jp->next = le_tmp->next;
            else
                le_head = le_tmp->next;

            free(le_tmp);
        }
        else
            le_jp = le_tmp;
    }

    le_tail = le_jp;
}

int is_job_lock(char *group)
{
    int ret = 0;
    lock_event *le_j;

    for (le_j=le_head; le_j; le_j=le_j->next)
    {
        if (!strcmp(le_j->se->group, "*") || !strcmp(le_j->se->group, group))
        {
            ret = 1;
            break;
        }
    }
    return ret;
}


