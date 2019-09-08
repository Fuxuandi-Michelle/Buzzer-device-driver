#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

unsigned long freq;
int bz_fd;
int bt_fd;
int main()
{	
	char current_values[1];
	int  ret;	
	bt_fd = open("/dev/button", O_RDONLY);
	bz_fd = open("/dev/buzzer", O_WRONLY);
	if(bz_fd == -1)
	{
		printf("Fail to open device buzzer!\n");
		goto finish;	
	}
	while(1)
	{
		ret = read(bt_fd, current_values, sizeof(current_values) );
		if (ret != sizeof(current_values) ) 
		{
			printf("Read key error:\n");
			goto finish;
		}
		
		if(current_values[0] == '1') freq = 1000;
		else if(current_values[0] == '2') freq = 1200;
		else if(current_values[0] == '3') freq = 1400;
		else if(current_values[0] == '4') freq = 0;
		
		write(bz_fd, &freq, sizeof(unsigned long));
		if(freq > 0) printf("Frequency is %d \n", freq);
		else printf("Buzzer is turned off.\n");
	}
	close(bz_fd);
	close(bt_fd);
finish:	
	return 0;
}