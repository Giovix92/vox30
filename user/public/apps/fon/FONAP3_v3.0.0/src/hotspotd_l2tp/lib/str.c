#include <stdio.h>
#include <ctype.h>

char *strupper(char* str)
{
	char *p = str;
	while ((*p = toupper(*p))) { 
		p++;
	}
	return str;
}

char *strlwr(char* str)
{
	char *p = str;
	while ((*p = tolower(*p))) {
		p++;
	}
	return str;
}
