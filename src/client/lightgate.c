#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int file;
char *bus = "/dev/i2c-1";
char config[2] = {0};

// See http://www.getelectronics.net/raspberry-pi-tcs34725-color-sensor-example-c.php#codesyntax_1

void lightgate_setup(){
	if ((file = open(bus, O_RDWR)) < 0){
		printf("Failed to open the bus. \n");
		exit(1);
	}	

	ioctl(file, I2C_SLAVE, 0x29);
	// Select enable register(0x80)
	// Power ON, RGBC enable, wait time disable(0x03)
	char config[2] = {0};
	config[0] = 0x80;
	config[1] = 0x03;
	write(file, config, 2);
	// Select ALS time register(0x81)
	// Atime = 700 ms(0x00)
	config[0] = 0x81;
	config[1] = 0x00;
	write(file, config, 2);
	// Select Wait Time register(0x83)
	// WTIME : 2.4ms(0xFF)
	config[0] = 0x83;
}

int lightgate_read(){
	if(read(file, data, 8) != 8) {
		printf("Erorr : Input/output Erorr \n");
	} else {
		// Convert the data
		int cData = (data[1] * 256 + data[0]);
		int red = (data[3] * 256 + data[2]);
		int green = (data[5] * 256 + data[4]);
		int blue = (data[7] * 256 + data[6]);
		// Calculate luminance
		float luminance = (-0.32466) * (red) + (1.57837) * (green) + (-0.73191) * (blue);
		if(luminance < 0)
		{
			luminance = 0;
		}
	}
}
