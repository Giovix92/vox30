#ifndef  _SC_SFP_H_
#define  _SC_SFP_H_

#define MAX_TRANSACTION_SIZE  31

#define DEV_NAME "/proc/sc_sfp"

#define EEPROM_A2_ADDR 0x51
#define REBOOT_COMMAND_EEPROM_ADDR 0xc2

/* ioctl interface */
enum{
        CMD_GET_GPIO = 0x100,
        SET_TX_DISABLE,
        CMD_I2C_READ,
        CMD_I2C_WRITE,

}I2C_OPS_CMD;

/*when get gpio value, val[0-1] ito stor pin number,  thr reurn value is save vval[0]*/
typedef struct cmdset_param
{
    unsigned char ops;
    unsigned char flag;
    unsigned int addr;
    unsigned char reg;
    unsigned char val[MAX_TRANSACTION_SIZE];
    int count;
}cmdset_param_t;


#endif

