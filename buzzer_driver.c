/*comp3438_buzzer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-timer.h>
#include <linux/clk.h>

#define DEVICE_NAME				"buzzer"
#define N_D         1                   /*Number of Devices*/
#define S_N         1                   /*The start minor number*/

#define GPD0_CON_ADDR	0xE02000A0 
#define GPD0_CON_DATA	0Xe02000A4

#define TCFG0	0xE2500000 
#define TCFG1	0xE2500004 
#define TCNTB0  0xE250000C
#define TCMPB0  0xE2500010
#define TCON	0xE2500008



static unsigned long tcfg0;
static unsigned long tcfg1;
static unsigned long tcntb0;
static unsigned long tcmpb0;
static unsigned long tcon;

static void *ZL_GPD0_CON_ADDR;
static void *ZL_GPD0_CON_DATA;
static void *XD_TCFG0;
static void *XD_TCFG1;
static void *XD_TCNTB0;
static void *XD_TCMPB0;
static void *XD_TCON;



//static struct semaphore lock;

static int          major;
static dev_t        devno;
static struct cdev  dev_buzzer;

unsigned long pclk;
struct clk *clk_p;

static void buzzer_start(unsigned long frequency) {

	unsigned long freq;
	unsigned long data;
	clk_p = clk_get(NULL, "pclk");
	pclk = clk_get_rate(clk_p);
	
	//set output mode, 0010
	data = readl(ZL_GPD0_CON_ADDR);	
    data = data&~0xF; 
    data = data|0x02; 
    writel(data, ZL_GPD0_CON_ADDR);	
    			   	  
    	
    //set up TCFG0, 65
	tcfg0 = readl(XD_TCFG0);
	tcfg0 &= ~0xFF; 
	writel(tcfg0, XD_TCFG0);
	tcfg0 |= 0x41;  
	writel(tcfg0, XD_TCFG0);

	
	//set up TCFG1, 0100
	tcfg1 = readl(XD_TCFG1); 
	tcfg1 &= ~0xF; 
	writel(tcfg0, XD_TCFG1);
	tcfg1 |= 0x4;  
	writel(tcfg1, XD_TCFG1);
	
	//set up TCNTB0
	freq=pclk/(0x41+1)/0x10;
	freq=freq/frequency;

	tcntb0 = readl(XD_TCNTB0);
	tcntb0 &= ~0xFFFFFFFF;   
	writel(tcntb0, XD_TCNTB0);
	tcntb0 |= freq;  	
	writel(tcntb0, XD_TCNTB0);
	
	//set up TCMPB0
	tcmpb0 = readl(XD_TCMPB0);
	tcmpb0 &= ~0xFFFFFFFF;  
	writel(tcmpb0, XD_TCMPB0);
	tcmpb0 = tcntb0/2;	
	writel(tcmpb0, XD_TCMPB0);
	
	/*set up TCON
	 * TCON[4], 0
	 * TCON[3], 1
	 * TCON[2], 0
	 * TCON[1], 0
	 * TCON[0], 0 */
	tcon = readl(XD_TCON);
	tcon &= ~0x1F;   
	writel(tcon, XD_TCON);
	tcon |= 0x8;	
	tcon &= ~0x10;	
	tcon &= ~0x4;	
	tcon |= 0x1;	
	writel(tcon, XD_TCON);	
	tcon &= ~0x2;	
	writel(tcon, XD_TCON);	

}


void buzzer_stop(void) {  

    unsigned int data;  
    
    //set output mode, 0001
    data = readl(ZL_GPD0_CON_ADDR);   
    data&= ~0xF; 
    data|= 0x1;  
    writel(data, ZL_GPD0_CON_ADDR); 
    
    data = readl(ZL_GPD0_CON_DATA);  
    data&=~0x1; 
    writel(data, ZL_GPD0_CON_DATA);
    
}  


static int zili_demo_char_buzzer_open(struct inode *inode, struct file *file) {

	//map the IO physical memory address to virtual address

	ZL_GPD0_CON_ADDR = ioremap(GPD0_CON_ADDR, 0x00000004);
	ZL_GPD0_CON_DATA = ioremap(GPD0_CON_DATA, 0x00000004);
	/*IO remap*/

	XD_TCFG0 = ioremap(TCFG0, 0x00000004);
	XD_TCFG1 = ioremap(TCFG1, 0x00000004);
	XD_TCNTB0 = ioremap(TCNTB0, 0x00000004);
	XD_TCMPB0 = ioremap(TCMPB0, 0x00000004);
	XD_TCON = ioremap(TCON, 0x00000004);	 	
	
	buzzer_stop();
	printk("Device " DEVICE_NAME " open.\n");

	return 0;

}



static int zili_demo_char_buzzer_close(struct inode *inode, struct file *file) {
	buzzer_stop();
	return 0;

}

static ssize_t zili_demo_char_buzzer_write(struct file *fp, const char *buf, size_t count, loff_t *position){
	unsigned long buzzer_status;
	int  ret;
	ret = copy_from_user(&buzzer_status, buf, sizeof(buzzer_status) );
	if(ret){
		printk("Fail to copy data from the user space to the kernel space!\n");
	}

	if( buzzer_status > 0 ){
		printk("Write Buzzuer Status: %lu\n",buzzer_status);
		buzzer_start(buzzer_status);
	}

	else{
		buzzer_stop();
	}

	return sizeof(buzzer_status);

}

static struct file_operations zili_mini210_pwm_ops = {

	.owner			= THIS_MODULE,
	.open			= zili_demo_char_buzzer_open,
	.release		= zili_demo_char_buzzer_close, 
	.write			= zili_demo_char_buzzer_write,

};



static int __init zili_demo_char_buzzer_dev_init(void) {

	int ret;
	/*Register a major number*/
	ret = alloc_chrdev_region(&devno, S_N, N_D, DEVICE_NAME);
	if(ret < 0){
		printk("Device " DEVICE_NAME " cannot get major number.\n");
		return ret;
	}	
	major = MAJOR(devno);
	printk("Device " DEVICE_NAME " initialized (Major Number -- %d).\n", major);	
	/*Register a char device*/
	cdev_init(&dev_buzzer, &zili_mini210_pwm_ops);
	dev_buzzer.owner = THIS_MODULE;
	dev_buzzer.ops   = &zili_mini210_pwm_ops;
	ret = cdev_add(&dev_buzzer, devno, N_D);
	if(ret){
		printk("Device " DEVICE_NAME " register fail.\n");
		return ret;
	}
	return 0;	
}



static void __exit zili_demo_char_buzzer_dev_exit(void) {
	buzzer_stop();
	cdev_del(&dev_buzzer);
	unregister_chrdev_region(devno, N_D);
	printk("Device " DEVICE_NAME " unloaded.\n");
}



module_init(zili_demo_char_buzzer_dev_init);
module_exit(zili_demo_char_buzzer_dev_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michelle FU");
MODULE_DESCRIPTION("S5PV210 Buzzer Driver");
