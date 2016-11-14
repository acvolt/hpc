#include "Smith_I2C.h"
#include <stdio.h>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cerrno>
#include <string.h>



Smith_I2C::Smith_I2C()
{

}

Smith_I2C::Smith_I2C(uint8_t addr)
{
	i2cAddr = addr;
}


Smith_I2C::~Smith_I2C()
{
	close(i2cDevice);
}

int Smith_I2C::i2cSetup(uint8_t addr)
{
	char *filename = (char*)"/dev/i2c-1";
	if ((i2cDevice = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
		return -1;
	}
	i2cAddr = addr;
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. \n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return -1;
	}
	return 0;
}

int Smith_I2C::i2cSetup()
{
	char *filename = (char*)"/dev/i2c-1";
	if ((i2cDevice = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
		return -1;
	}

	if (ioctl(i2cDevice, I2C_SLAVE, i2cAddr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. \n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return -1;
	}


	//Next 2 commands are to program the I2C Multiplexer.  
//	if (ioctl(i2cDevice, I2C_SLAVE, 0x70) < 0)
//	{
//		printf("Failed to acquire bus access and/or talk to slave. Smith_I2C::i2cSetup() \n");
//		//ERROR HANDLING; you can check errno to see what went wrong
//		return -1;
//	}
//	i2cWriteByte(0x70, 0);
	
//	if (write(i2cDevice, buffer, 1) != 1) {
//		//ERROR HANDLING: i2c transaction failed
//		printf("Failed to read from the i2c bus. Smith_I2C::i2cSetup()\n");
//		return -1;
//	}

	return 0;
}

unsigned char Smith_I2C::i2cReadRegByte(int fd, __uint8_t addr, __uint8_t reg)
{

	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cReadRegByte \n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return -1;
	}

//	__s32 result;
//	result = ;
	

	return i2c_smbus_read_byte_data(i2cDevice, reg);
}
unsigned char Smith_I2C::i2cReadByte(int fd, __uint8_t addr)
{

	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cReadByte \n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return 255;
	}
	/*
	if (read(i2cDevice, buffer, 1) != 1) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
		return 255;
	}
	*/

	return i2c_smbus_read_byte(i2cDevice);
}


void Smith_I2C::i2cReadBlock(__uint8_t addr, void* buffer, int length)
{
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cReadBlock \n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	/*
	if (read(i2cDevice, buffer, length) != length) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	*/
//	i2c_smbus_read_block_data(i2cDevice, 0, (__u8*)buffer);


}

void Smith_I2C::i2cReadBlock(__uint8_t addr, __uint8_t* reg,  void* buffer, int length)
{
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cReadBlock\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	/*
	if (read(i2cDevice, buffer, length) != length) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	} */

	i2c_smbus_read_block_data(i2cDevice, *reg, (__uint8_t*)buffer);

}

void Smith_I2C::i2cWriteBlock(__uint8_t addr, void* buffer, int length)
{
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cWriteBlock\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	/*
	if (write(i2cDevice, buffer, length) != length) {
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	} */

	int err = i2c_smbus_write_block_data(i2cDevice, 0, length, (__uint8_t*)buffer);
	if (err < 0)
	{
		printf("Failed in i2c_smbus_write_block_data in i2cWriteBlock (noreg) error is %d errorno %d \n", err, errno);
		char buffer[256];
		char * errorMessage = strerror_r(errno, buffer, 256);
		printf("Error Buffer is %s \n", buffer);
	}

}

void Smith_I2C::i2cWriteByte(__uint8_t addr, uint8_t value)
{
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cWriteBlock\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	i2c_smbus_write_byte(i2cDevice, value);
}

void Smith_I2C::i2cWriteByteData(uint8_t addr, uint8_t reg, uint8_t value)
{
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cWriteBlock\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	int temp;
	temp = i2c_smbus_write_byte_data(i2cDevice, reg, value);
//	printf("Write Byte Data return is %d \n", temp);
}

void Smith_I2C::directWriteByte(__uint8_t addr, uint8_t value)
{
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cWriteBlock\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	write(i2cDevice, &value, 1);
}

void Smith_I2C::i2cMultiWriteOneTwo(__uint8_t addr, __uint8_t* reg, __uint8_t reglength)
{
	int result;

	struct i2c_msg rdwr_msgs[2] = {
		{
			addr,
			0, //Write
			1, //Length
			reinterpret_cast<char*>(reg), //message buffer
		},
		{
			addr,
			0, //2nd Write
			2,
			reinterpret_cast<char*>(reg+1),
		}
	};

//	printf("Reg1=%x Reg2=%x Reg3=%x\n", *reinterpret_cast<char*>(reg), *reinterpret_cast<char*>(reg + 1), *reinterpret_cast<char*>(reg + 2));
	struct i2c_rdwr_ioctl_data rdwr_data = {};
	rdwr_data.msgs = rdwr_msgs;
	rdwr_data.nmsgs = 2;

	result = ioctl(i2cDevice, I2C_RDWR, &rdwr_data);

	if (result < 0) {
		printf("rdwr ioctl error: %d\n", errno);
		perror("reason");
	}
//	else {
//		printf("rdwr ioctl OK\n");
//		for (i = 0; i < 16; ++i) {
//			printf("%x", buffer[i]);
//		}
//		printf("\n");
//	}

}

void Smith_I2C::i2cWriteBlock(__uint8_t addr, __uint8_t* reg, void* buffer, int length)
{
	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave. i2cWriteBlock (reg) %d \n", *reg);
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	
	//if (write(i2cDevice, reg, 1) != 1) {
	//	//ERROR HANDLING: i2c transaction failed
	//	printf("Failed to read reg from the i2c bus. errno is %d \n", errno);
	//}
	/*
uint8_t newBuffer[length + 1];
	newBuffer[0] = length;
	for (int z = 1; z < length; z++)
	{
		newBuffer[z] = *static_cast<uint8_t*>(buffer + z);
	}
	*/
	uint8_t buf = *static_cast<uint8_t*>(buffer);
	int err;
//	if (write(i2cDevice, buffer, length+1) != length+1) {
//		//ERROR HANDLING: i2c transaction failed
//		printf("Failed to read buffer from the i2c bus.errno is %d \n", errno);
//	}
	if (length > 1)
		err = i2c_smbus_write_i2c_block_data(i2cDevice, *reg, length, (uint8_t*)buffer);
	else
	{
//		ioctl(i2cDevice, I2C_PEC, 1);
		err = i2c_smbus_write_byte_data(i2cDevice, *reg, buf);
//		ioctl(i2cDevice, I2C_PEC, 0);

	}
	if (err < 0)
	{
//		printf("Failed in i2c_smbus_write_block_data in i2cWriteBlock Register is %d, length is %d,  error is %d errorno is %d  \n", *reg, length, err, errno);
		for (int z = 0; z < length+1; z++)
		{
		//	printf("Buffer Contents at Position %d are %x \n", z, *static_cast<__uint8_t*>(buffer + z));
		}
	}

}



uint8_t Smith_I2C::smbus_read_reg_byte(uint8_t addr, uint8_t reg)
{
//	printf("In Smbus_read_reg_byte... \n");

	int temp;
	union i2c_smbus_data data;
//	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
//	{
//		printf("Failed to acquire bus access and/or talk to slave.\n");
//		//ERROR HANDLING; you can check errno to see what went wrong
//		return 255;
//	}
	temp = i2c_smbus_access(i2cDevice, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data);
//	printf("return is %d read byte is %x \n", temp, data.byte);
	if (temp)
		return -1;
	else
	{
		return data.byte & 0xFF;
	}

}

int Smith_I2C::smbus_write_reg_byte(uint8_t addr, uint8_t reg, uint8_t value)
{
//	printf("In Smbus_write_reg_byte... \n");

	union i2c_smbus_data data;
//	if (ioctl(i2cDevice, I2C_SLAVE, addr) < 0)
//	{
//		printf("Failed to acquire bus access and/or talk to slave. \n");
//		//ERROR HANDLING; you can check errno to see what went wrong
//		return -1;
//	}

	data.byte = value;
	return i2c_smbus_access(i2cDevice, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE_DATA, &data);
}
