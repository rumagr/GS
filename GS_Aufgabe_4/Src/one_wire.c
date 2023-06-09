#include "one_wire.h"
#include "gpio_basefunct.h"
#include "time.h"
#include <stdio.h>

#define OK 0
#define ERR 1
#define READ_ROM_COMMAND 0x33
#define MATCH_ROM_COMMAND 0x55
#define CONVERT_T_COMMAND 0x44
#define READ_SCRATCHPAD_COMMAND 0xBE
#define BIT_MASK 0xFF
#define NUM_BITS_PER_BYTE 8
#define BIT_SET 1 
#define ROM_CRC_SHIFT 56
#define BYTES_PER_ROM 8
#define SCRATCHPAD_BYTES 9


uint32_t io_reset(void)
{
	uint8_t var = 0; 
	GPIO_Low(); //bus low 
	wait(480); 
	GPIO_High(); //bus freigeben 
	wait(70); 
	if(read_bit(&var,0))
	{
		return ERR; 
	}
	wait(410); 
	return OK; 
}

void write_one(void)
{
	GPIO_Low(); 
	wait(6); 
	GPIO_High(); 
	wait(64); 
}

void write_zero(void)
{
	GPIO_Low(); 
	wait(60); 
	GPIO_High(); 
	wait(10); 
}

uint32_t read_bit(uint8_t* var, uint32_t shift)
{
	GPIO_Low(); 
	wait(6); 
	GPIO_High(); 
	wait(9); 
	GPIO_In();
	GPIO_Read(var,shift);
	wait(55);
	GPIO_Out();
	return OK; 
}

uint32_t read_byte(uint8_t* var)
{
	for(uint32_t i = 0; i < NUM_BITS_PER_BYTE; ++i)
	{
		read_bit(var, i); 
	}

	return 0; 
}

uint32_t readRom(uint64_t* var)
{
	uint8_t bytes[BYTES_PER_ROM] = {0};
	
	io_reset(); 
	
	writeByte(READ_ROM_COMMAND);
	
	for(uint32_t i = 0; i < BYTES_PER_ROM; ++i)
	{
		read_byte(&bytes[i]); 
	} 
	
	uint64_t temp = 0; 
	
	for(uint32_t i = 0; i < BYTES_PER_ROM; ++i)
	{
		temp |= ((uint64_t) bytes[i]) << (i*NUM_BITS_PER_BYTE); 
	}
	
	*var = temp; 
	
	return OK; 
}

void writeByte(uint8_t byte) //sendet ein byte  
{
	for(uint32_t i = 0; i < NUM_BITS_PER_BYTE; ++i) //f�r jedes bit 
	{
		if((byte >> i) & BIT_SET) //wenn bit == 1 -> sende 1 
		{
			write_one(); 
		}
		else //wenn bit 0 -> sende 0 
		{
			write_zero(); 
		}
	}
}

uint8_t checkCRC(uint64_t rom, uint32_t bytecount)
{
	uint8_t crc = (uint8_t)(rom >> ROM_CRC_SHIFT); 
	uint8_t rescrc = 0; 
	uint8_t num[7] = {0}; //7 l�sst sich nicht durch bytecount ersetzen 
	
	for(uint32_t i = 0; i < bytecount; ++i)
	{ 
		num[i] = (uint8_t)((rom >> ((i) * NUM_BITS_PER_BYTE)) & BIT_MASK);
	}
	
	const uint8_t table[256] = 
	{
		0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31,
65,
157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130,
220,
35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60,
98,
190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161,
255,
70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89,
7,
219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196,
154,
101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122,
36,
248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231,
185,
140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147,
205,
17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14,
80,
175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176,
238,
50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45,
115,
202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213,
139,
87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72,
22,
233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246,
168,
116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107,
53
	}; 
	
	for(uint32_t i = 0; i < bytecount; ++i)
	{
		rescrc = table[rescrc ^ num[i]]; 
	}
	
	return (rescrc == crc); 
}
	

uint32_t readTemp(uint64_t rom, uint8_t* res)
{
	//w�hle Slave aus
	io_reset(); 
	writeByte(MATCH_ROM_COMMAND); 
	
	for(uint32_t i = 0; i < BYTES_PER_ROM; ++i)
	{
		uint8_t temp = (uint8_t)((rom >> ((i) * NUM_BITS_PER_BYTE)) & BIT_MASK); 
		writeByte(temp); 
	}
	
	writeByte(CONVERT_T_COMMAND); 
	//f�hre messung durch
	push_pull();
	GPIO_High();	
	wait(750000);
	GPIO_Low();
	open_drain(); 
	GPIO_Out();
	//w�hle slave aus 
	io_reset();
	writeByte(MATCH_ROM_COMMAND); 
	for(uint32_t i = 0; i < BYTES_PER_ROM; ++i)
	{
		uint8_t temp = (uint8_t)((rom >> ((i) * NUM_BITS_PER_BYTE)) & BIT_MASK); 
		writeByte(temp); 
	}
	
	writeByte(READ_SCRATCHPAD_COMMAND); 
	//lese ergebnis 
	for(uint32_t i = 0; i < SCRATCHPAD_BYTES; ++i)
	{
		read_byte(&res[i]); 
	} 	
	return OK;
}


	