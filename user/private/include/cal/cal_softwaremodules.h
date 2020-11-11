#ifndef __CAL_SOFTWAREMODULES_H__
#define __CAL_SOFTWAREMODULES_H__
#define NODE_SOFTWAREMODULES "InternetGatewayDevice.SoftwareModules."
struct execenv
{
    int index;
    int enable;
};
int cal_softwaremodules_execenv_get_all_entries(struct execenv **ee);
char * cal_softwaremodules_execenv_get_enable(int index);
char * cal_softwaremodules_execenv_get_memorylimit(int index);
char * cal_softwaremodules_execenv_get_cpus(int index);
#endif
