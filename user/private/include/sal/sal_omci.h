#ifndef __SAL_OMCI_H__
#define __SAL_OMCI_H__

#define OMCI_CATV_STATE_ENABLE      "1"
#define OMCI_CATV_STATE_DISABLE     "0"

#define OMCI_CATV_FILTER_LOWPASS    "1"
#define OMCI_CATV_FILTER_BYPASS     "0"

char *sal_omci_get_catv_state(void);
int sal_omci_set_catv_state(char *value);

char *sal_omci_get_catv_filter(void);
void sal_omci_set_catv_filter(char *value);
#endif

