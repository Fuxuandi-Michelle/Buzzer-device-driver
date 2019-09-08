# Buzzer-device-driver

Instruction for compile is provided for reference.

Please note that the following is only for reference, 
changes may need to be done if any difference.


1. copy the folder buzzer_application, the file buzzer_driver.c and button_driver.c to linux-3.0.8/drivers/char
(please not the folder buzzer_application is copied, not buzzer_application.c)

2. adjust the Makefile

‘’‘
obj-$(CONFIG_BUZZER) += buzzer_driver.o
obj-$(CONFIG_BUTTON) += button_driver.o
’‘’

3. adjust the Kconfig file
‘’‘
config BUZZER
	tristate "BUZZER DRIVER"
	depends on CPU_S5PV210

config BUTTON
	tristate "BUTTON DRIVER"
	depends on CPU_S5PV210
’‘’
4. change mode to M

cd /tiny6410/linux-3.0.8
make menuconfig
make

5. With Putty: load driver to kernel
‘’‘
mount -t nfs  -o  nolock,rsize=4096,wsize=4096 192.168.1.145:/tiny6410  /mnt/nfs
cd /mnt/nfs/linux-3.0.8/drivers/char
insmod buzzer_driver.ko
insmod button_driver.ko
mknod /dev/buzzer c 250 1
mknod /dev/button c 249 1
’‘’
(Note: adjust the number if different)

6. compile the application progam
‘’‘
cd drivers/char/buzzer_application
arm-linux-gcc -o buzzer_application buzzer_application.c
’‘’
7. run the executable program
‘’‘
cd buzzer_application
./buzzer_application
’‘’
8. test by press KEY1 - 4
