#ifndef __SAL_PROCESS_H__
#define __SAL_PROCESS_H__
typedef struct pro_t
{
	char pid[10];
	char size[20];
	char cputime[20];
	char prio[20];
	char cmd[255];
	char state[50];
}pro_t;

int sal_get_process_table(pro_t **,int *);
int sal_update_process_table(void);

#endif
