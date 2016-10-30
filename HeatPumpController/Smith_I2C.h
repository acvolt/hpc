#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <stdio.h>
#include <wiringPiI2C.h>

#include <stdint.h>



#ifndef I2C_SLAVE
#define I2C_SLAVE			0X0703
#endif
#ifndef I2C_SMBUS
#define I2C_SMBUS			0x0720
#endif

#define I2C_SMBUS_READ				1
#define I2C_SMBUS_WRITE				0

#define I2C_SMBUS_QUICK				0
#define I2C_SMBUS_BYTE				1
#define I2C_SMBUS_BYTE_DATA			2
#define I2C_SMBUS_WORD_DATA			3
#define I2C_SMBUS_PROC_CALL			4
#define I2C_SMBUS_BLOCK_DATA		5
#define I2C_SMBUS_I2C_BLOCK_BROKEN	6
#define I2C_SMBUS_BLOCK_PROC_CALL	7
#define I2C_SMBUS_I2C_BLOCK_DATA	8

#define I2C_SMBUS_BLOCK_MAX			32
#define I2C_SMBUS_I2C_BLOCK_MAX		32

/*

union i2c_smbus_data
{
	__uint8_t byte;
	__uint16_t word;
	__uint8_t block[I2C_SMBUS_BLOCK_MAX + 2];
}; 

struct i2c_smbus_ioctl_data
{
	char read_write;
	__uint8_t command;
	int size;
	union i2c_smbus_data *data;
};

static inline int i2c_smbus_access(int fd, char rw, __uint8_t command, int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = rw;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(fd, I2C_SMBUS, &args);
}
*/





class Smith_I2C
{
public:
	Smith_I2C(void);
	~Smith_I2C();
	int i2cSetup();
	unsigned char i2cReadByte(int fd, __uint8_t addr);
	unsigned char i2cReadRegByte(int, __uint8_t, __uint8_t);
	void i2cReadBlock(__uint8_t addr,  void* buffer, int length);
	void i2cReadBlock(__uint8_t addr, __uint8_t* reg, void* buffer, int length);
	void i2cWriteBlock(__uint8_t addr, void* buffer, int length);
	void i2cWriteBlock(__uint8_t addr, __uint8_t* reg, void* buffer, int length);
	void i2cMultiWriteOneTwo(__uint8_t addr, __uint8_t* reg, __uint8_t reglength);
	void i2cWriteByte(__uint8_t addr, __uint8_t value);
	void i2cWriteByteData(__uint8_t addr, __uint8_t reg, __uint8_t value);
	void directWriteByte(__uint8_t addr, uint8_t value);

	unsigned char i2cReadRegByte(int fd, __uint8_t reg);

protected:
	unsigned int i2cDevice;



};

