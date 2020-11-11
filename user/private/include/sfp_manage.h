/*************************************************************************
 *  Copyright - 2016 Sercomm Corporation.
 *  All Rights Reserved.
 *  Sercomm Corporation reserves the right to make changes to this document without 
 *  notice. Sercomm Corporation makes no warranty, representation or guarantee regarding 
 *  the suitability of its products for any particular purpose. Sercomm Corporation assumes
 *  no liability arising out of the application or use of any product or circuit. Sercomm
 *  Corporation specifically disclaims any and all liability, including without limitation
 *  consequential or incidental damages; neither does it convey any license under its patent 
 *  rights, nor the rights of others.
 * 
 ************************************************************************/
 /*The max length of SFP CLI command and result(include  CRC value, 4 bytes)*/
#define CMD_MAX      118
#define RESULT_MAX  2048
/*The max length of username and password of loging SFP*/
#define USER_PW_LEN  50

/*this  is i2c read/write function pointer

 *@index: equipment number, in sfp_manage lib, this value is 0; 
 *@sc_bank: equipment address, in sfp_manage lib ,the variable have 2 value(0 and 2).
 *       0 represnts A0 page in FGS202 SFP. 2 represents A2 page in FGS202 SFP memory.  
 *@address:data adress(from 0x00 to 0xff)
 *@length:the length of data to be read/writen
 *@buffer:the buffer of the read/write data from/into EEPROM
 return 0 if ok, othersize -1.
 * */
typedef int (* eeprom_wr)(unsigned char index, unsigned char sc_bank, unsigned long address, unsigned long length, unsigned char *buffer);

/*this  is get crc_value function pointer
*@buffer the char pointer of string 
*@dwSize the former dwSize char of string to be calculated
*return - CRC value
 * */
typedef unsigned long (* CRC)(char *buffer, int length);

/**
*this struct is used to store some data about executing SFP CLI command.

*@Length: the command's length(when input SFP CLI command) or the result's length(when return result).
*@cmd: char array is used to store SFP CLI command
*@result: char array is used to store the result of SFP CLI command
**/ 
struct sfp_operate
{
	unsigned long length;
	char cmd[CMD_MAX];
	char result[RESULT_MAX];
};

/**
*this function is used to register underlying read/write function and getting crc value function . 
* @write_n: a pointer of underlying write function
* @read_n: a  pointer of underlying read function
* @crc_n : a poointer of getting CRC function
**/
void sc_sfpi2c_register(eeprom_wr read_n,eeprom_wr write_n, CRC crc_n);

/**
*this function is used to register usename and password of loging SFP.
* @user: username of SFP
* @pw:  password of SFP
**/
void sfp_user_pw_register(char *user, char * pw );

/** 
*this function is used to send SFP CLI command to SFP ,after executed, return the result of 
*SFP CLI command.
* @sfp_info: a sfp_operate struct pointer 
*return 0 if CLI command has been execuated, othersize  -1.
**/
int sc_sfpi2c_cmd(struct sfp_operate* sfp_info);

