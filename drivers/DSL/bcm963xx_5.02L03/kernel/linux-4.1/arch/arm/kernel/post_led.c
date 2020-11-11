#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/wait.h>

#ifdef CONFIG_MIPS
#include <bcm_map_part.h>
#include <bcm_cpu.h>
#include <bcm_intr.h>
#include <board.h>
#include <boardparms.h>
#endif

//#define DEBUG
#ifdef DEBUG
#define PRINTK(format,argument...) printk(format,##argument)
#else
#define PRINTK(format,argument...)
#endif

#ifdef ESSENTIAL
#define POSTING_LED_INTERVAL	(HZ/3)			
#else
#define POSTING_LED_INTERVAL	HZ			
#endif
#define POSTING_TIME		(HZ * 60 * 2)//2 minutes
#define POSTING_INIT_TIME	(HZ * 30 * 2)//1 minutes

#define BIT_POSTING		    (0x1)
#define BIT_POSTING_BEFORE_INIT	(0x1 << 1)
#define BIT_POSTING_ERROR	(0x1 << 2)
#define BIT_POSTING_SUCCESS	(0x1 << 3)
#define BIT_POSTING_END		(0x1 << 4)

#ifdef VOX30_SFP
#define BP_GPIO_22_AH (22)
#endif

extern void setLed_ex (short led_gpio, unsigned short led_state);
#ifdef CONFIG_MIPS
extern void bcm63xx_init_gpios(void);
#endif
int hal_output_set_power_led_state(int state, int color)
{
#ifdef VOX30_SFP
	short led_gpio;
	if(color == 0)
		led_gpio = BP_GPIO_22_AH;
	setLed_ex(led_gpio, state);
#else
#ifdef CONFIG_MIPS
    short led_gpio;
#ifdef CONFIG_BCM963268
#ifdef FD1018
    if(color == 0)//Green 
        led_gpio = BP_SERIAL_GPIO_0_AL;
    else
        led_gpio = BP_SERIAL_GPIO_1_AL;
#else
#ifdef ESSENTIAL
    if(color == 0)//Green 
        led_gpio = BP_GPIO_17_AH;
    else
        led_gpio = BP_GPIO_8_AH;
#else
    if(color == 0)//Green 
        led_gpio = BP_SERIAL_GPIO_0_AL;
    else
        led_gpio = BP_SERIAL_GPIO_1_AL;
#endif
#endif
#else
    if(color == 0)//Green 
        led_gpio = BP_SERIAL_GPIO_8_AL;
    else
        led_gpio = BP_SERIAL_GPIO_9_AL;
#endif
#ifndef VOX25
    setLed_ex(led_gpio, state);
#endif
#endif
#endif
    return 0;
}
int hal_output_set_dsl_led_state(int state, int color)
{
#ifdef CONFIG_MIPS
    short led_gpio;
#ifdef CONFIG_BCM963268
#ifdef FD1018
    return 0;
#else
    if(color == 0)//Green 
        led_gpio = BP_SERIAL_GPIO_2_AL;
    else
        led_gpio = BP_SERIAL_GPIO_3_AL;
#endif
#else
    if(color == 0)//Green 
        led_gpio = BP_SERIAL_GPIO_10_AL;
    else
        led_gpio = BP_SERIAL_GPIO_11_AL;
#endif
#ifndef VOX25
    setLed_ex(led_gpio, state);
#endif
#endif
    return 0;
}

int hal_output_set_internet_led_state(int state, int color)
{
#ifdef CONFIG_MIPS
    short led_gpio;
#ifdef CONFIG_BCM963268
#ifdef FD1018
    if(color == 0)//Green 
        led_gpio = BP_GPIO_8_AL;
    else
        led_gpio = BP_GPIO_9_AL;
#else
#ifdef ESSENTIAL
    if(color == 0)//Green 
        led_gpio = BP_GPIO_15_AH;
    else
        led_gpio = BP_GPIO_14_AH;
#else
    if(color == 0)//Green 
        led_gpio = BP_GPIO_8_AH;
    else
        led_gpio = BP_GPIO_13_AH;
#endif
#endif
#else
    if(color == 0)//Green 
        led_gpio = BP_GPIO_1_AL;
    else
        led_gpio = BP_GPIO_0_AL;
#endif
#ifndef VOX25
    setLed_ex(led_gpio, state);
#endif
#endif
    return 0;
}

int hal_output_set_lan1_led_state(int state, int color)
{
#ifdef CONFIG_MIPS
    short led_gpio;
#ifdef CONFIG_BCM963268
#ifdef FD1018
    if(color == 0)//Green 
        led_gpio = BP_GPIO_12_AL;
    else
        led_gpio = BP_SERIAL_GPIO_11_AL;
#else
    if(color == 0)//Green 
        led_gpio = BP_GPIO_9_AL;
//    else
//        led_gpio = BP_GPIO_9_AH;
#endif
#else
    if(color == 0)//Green 
        led_gpio = BP_GPIO_25_AH;
    else
    {
        led_gpio = BP_GPIO_17_AH;
    }
#endif
#ifndef VOX25
    kerSysSetGpioState(led_gpio, state);
#endif
#endif
    return 0;
}
int hal_output_set_wifi_led_state(int state, int color)
{
#ifdef CONFIG_MIPS
    short led_gpio;
    if(color == 0) 
    {
#ifdef CONFIG_BCM963268
#ifdef FD1018
        led_gpio = BP_SERIAL_GPIO_6_AL;
#else
#ifdef ESSENTIAL
        led_gpio = BP_GPIO_9_AH;
#else
        led_gpio = BP_SERIAL_GPIO_6_AL;
#endif
#endif
#else
        led_gpio = BP_SERIAL_GPIO_14_AL;
#endif
#ifndef VOX25
    setLed_ex(led_gpio, state);
#endif
    }
#endif
    return 0;
}

int hal_output_set_wps_led_state(int state, int color)
{
#ifdef CONFIG_MIPS
    short led_gpio;
    if(color == 0)
    {
#ifdef CONFIG_BCM963268
#ifdef FD1018
        led_gpio = BP_SERIAL_GPIO_5_AL;
#else
        led_gpio = BP_SERIAL_GPIO_5_AL;
#endif
#else
        led_gpio = BP_SERIAL_GPIO_13_AL;
#endif
#ifndef VOX25
    setLed_ex(led_gpio, state);
#endif
    }
#endif
    return 0;
}

int hal_output_set_phone_led_state(int state, int color)
{
#ifdef CONFIG_MIPS
    short led_gpio;
    if(color == 0)
    {
#ifdef CONFIG_BCM963268
#ifdef FD1018
        led_gpio = BP_SERIAL_GPIO_4_AL;
#else
#ifdef ESSENTIAL
        led_gpio = BP_GPIO_16_AH;
#else
        led_gpio = BP_SERIAL_GPIO_4_AL;
#endif
#endif
#else
        led_gpio = BP_SERIAL_GPIO_12_AL;
#endif
#ifndef VOX25
     setLed_ex(led_gpio, state);
#endif
    }
#endif
    return 0;
}

int hal_output_set_usb_led_state(int state, int color)
{
#ifdef CONFIG_MIPS
    short led_gpio;
    if(color == 0)
    {
#ifdef CONFIG_BCM963268
#ifdef FD1018
        led_gpio = BP_SERIAL_GPIO_7_AL;
#else
#ifdef ESSENTIAL
        led_gpio = BP_GPIO_1_AH;
#else
        led_gpio = BP_SERIAL_GPIO_7_AL;
#endif
#endif
#else
        led_gpio = BP_SERIAL_GPIO_16_AL;
#endif
#ifndef VOX25
      setLed_ex(led_gpio, state);
#endif
    }
#endif
    return 0;
}

void post_error(void);

static int get_post_status(void);

static int postingStatus = BIT_POSTING;
static int power_led_rstate = 0;
static unsigned long posting_led_jiffies = INITIAL_JIFFIES;
static void posting_led(void)
{
#ifdef VOX_LED_SPEC
    power_led_rstate = !power_led_rstate;
    hal_output_set_power_led_state(power_led_rstate, 0);
#else
#ifdef ESSENTIAL
    if(power_led_rstate == 0)
        hal_output_set_power_led_state(1, 0);
    else if(power_led_rstate == 3)
        hal_output_set_power_led_state(0, 0);
    power_led_rstate = (power_led_rstate + 1)%5;
#else
    power_led_rstate = !power_led_rstate;
    hal_output_set_power_led_state(power_led_rstate, 0);
#endif
#endif
}
static void post_success_led(void)
{
    hal_output_set_power_led_state(1, 0);
#ifdef ESSENTIAL
    hal_output_set_power_led_state(0, 1);
#endif
}

static void post_error_led(void)
{
    power_led_rstate = !power_led_rstate;
#ifdef ESSENTIAL
    hal_output_set_power_led_state(power_led_rstate, 1);
#else
    hal_output_set_power_led_state(power_led_rstate, 0);
#endif
#ifndef ESSENTIAL
    hal_output_set_dsl_led_state(power_led_rstate, 0);
#endif
    hal_output_set_internet_led_state(power_led_rstate, 0);
#ifndef ESSENTIAL
    hal_output_set_lan1_led_state(power_led_rstate, 0);
#endif
    hal_output_set_wifi_led_state(power_led_rstate, 0);
#ifndef ESSENTIAL
    hal_output_set_wps_led_state(power_led_rstate, 0);
#endif
    hal_output_set_phone_led_state(power_led_rstate, 0);
    hal_output_set_usb_led_state(power_led_rstate, 0);
}
void hal_output_init_internet_led(void)
{
#ifndef ESSENTIAL
#ifndef VOX25
#ifdef CONFIG_MIPS
#ifdef CONFIG_BCM963268
#ifdef FD1018
#else
    
    kerSysSetGpioDir(BP_GPIO_9_AL);
    kerSysSetGpioDir(BP_GPIO_8_AH);
    kerSysSetGpioDir(BP_GPIO_13_AH);
#endif
#else
    kerSysSetGpioDir(BP_GPIO_25_AH);
    kerSysSetGpioDir(BP_GPIO_0_AH);
    kerSysSetGpioDir(BP_GPIO_1_AH);
#endif
#endif
#endif
#endif
}

void hal_output_init_lan1_led(void)
{
#ifndef VOX25
#ifdef CONFIG_MIPS
#ifdef CONFIG_BCM963268
    kerSysSetGpioDir(BP_GPIO_9_AH);
#else
    kerSysSetGpioDir(BP_GPIO_25_AH);
#endif
#endif
#endif
}

void do_post_without_interrupt(int first)
{
    if(first)
    {
#ifdef CONFIG_MIPS
#ifndef VOX25
     //   bcm63xx_init_gpios();
#endif
#else
    ;
#endif
#ifndef VOX_LED_SPEC
        hal_output_init_internet_led();
        hal_output_init_lan1_led();
#endif
    }
    posting_led();
}
long do_post_in_panic(long count)
{
    if(!(count % 1000))
    {
        if(!(get_post_status() & BIT_POSTING_END))
        {
            post_error();
            post_error_led();
        }
    }
    return 0;
}

void post_error(void)
{
	postingStatus = 0;
	postingStatus |= BIT_POSTING_ERROR;
}
static unsigned long int post_before_init_jiffies = 0;
void post_before_init_process(void)
{
	post_before_init_jiffies  = jiffies;
	postingStatus = 0;
	postingStatus |= BIT_POSTING_BEFORE_INIT;	
}
void post_success(void)
{
	postingStatus = 0;
	postingStatus |= (BIT_POSTING_END | BIT_POSTING_SUCCESS);
        post_success_led();
}
static void post_end(void)
{
	postingStatus = 0;
	postingStatus |= BIT_POSTING_END;
}
static int get_post_status(void)
{
	return postingStatus;
}
int check_post_time_end(void)
{
	return postingStatus == BIT_POSTING_END;
}
void do_post_interrupt(void)
{
	if(get_post_status() & BIT_POSTING_END)
	{
		
	}
	else if(get_post_status() & BIT_POSTING)
	{
		if(time_after(jiffies, (INITIAL_JIFFIES + POSTING_TIME)))
		{
			post_end();
			post_error_led();
		}
		else
		{
                    if(time_after(jiffies, posting_led_jiffies) && (jiffies%POSTING_LED_INTERVAL == 0))
                    {
                        posting_led_jiffies = jiffies;
			posting_led();
		    }
		}	
	
	}
	else if(get_post_status() & BIT_POSTING_BEFORE_INIT)
	{
		if(time_after(jiffies, (post_before_init_jiffies + POSTING_INIT_TIME)))
		{
			post_end();
			post_success_led();
		}
		else
		{
//                        if(time_after(jiffies, posting_led_jiffies) && (jiffies%POSTING_LED_INTERVAL == 0))
			{
                            posting_led_jiffies = jiffies;
		            posting_led();
			}
		
		}	
	}
	else if(get_post_status() & BIT_POSTING_ERROR)
	{
		post_end();
		post_error_led();
	}
	
	
}
EXPORT_SYMBOL(do_post_interrupt);
EXPORT_SYMBOL(post_before_init_process);
EXPORT_SYMBOL(post_success);
EXPORT_SYMBOL(post_error);
EXPORT_SYMBOL(do_post_without_interrupt);
EXPORT_SYMBOL(do_post_in_panic);
EXPORT_SYMBOL(check_post_time_end);
