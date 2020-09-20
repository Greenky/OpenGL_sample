#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/mman.h> 
#include <strings.h>
#include "hwlib.h" 
#include "socal/socal.h"
#include "socal/hps.h" 
#include "socal/alt_gpio.h"
//#include "hps_0.h"


#define REG_BASE 0xFF200000 // LW - 0xFF200000  AXI - 0xC0000000
#define REG_SPAN 0x00200000 // LW - 0x00200000  AXI - 0x00800000
#define OUT_BASE 0x00
#define IN_BASE 0x80
#define REG_WIDTH 0x20
#define READING_FLAG 0x100
#define WRITING_FLAG 0x120
#define KEY_FLAG 0x140
#define KEY_BASE 0x160
#define SWICH_FLAG 0x1E0

void* virtual_base;
void* input_addr;
void* output_addr;
void* key_addr;
void* read_flag_addr;
void* write_flag_addr;
void* switch_flag_addr;
void* key_flag_addr;
int mem_fd;
int fd_config;
int fd_data;
int fd_w;

void write_byte(uint32_t b)
{
	uint8_t byte;
	for (int j = 0; j < 4; j++)
	{
		byte = ((b >> (j * 8)) & 0xFF);
		printf("%.2x  ", byte);
	}
	printf("\n");
}

float read_number(int fd_r)
{
	int red = 0;
	int Red_char = 0;
	float output;
	// Read number of data packets
	int index = 0;
	char number_str[10];
	bzero(number_str, 10);

	//Skip spaces
	while((red = read(fd_r, &Red_char, 1)) > 0)
	{
		if (Red_char > 32)
		{
			number_str[index] = Red_char;
			index++;
			break;
		}
	}
	//Read to space
	while((red = read(fd_r, &Red_char, 1)) > 0 && Red_char > 32 && index < 10)
	{
		number_str[index] = Red_char;
		index++;
	}
	output = atof(number_str);
	return(output);
}

void readKey(int fd_r, uint8_t key[])
{
	char hex_str[2];
	bzero(hex_str, 2);

	//Read to space
	for (int i = 0; i < 16; i++)
	{
		read(fd_r, &hex_str, 2);
		key[i] = (uint8_t)strtol(hex_str, NULL, 16);
	}
}

int main (int argc, char** argv)
{
	
	mem_fd = open("/dev/mem",(O_RDWR|O_SYNC));
	virtual_base=mmap(NULL, REG_SPAN, (PROT_READ|PROT_WRITE), MAP_SHARED, mem_fd, REG_BASE);
	input_addr			= virtual_base + IN_BASE;
	output_addr			= virtual_base + OUT_BASE;
	key_addr			= virtual_base + KEY_BASE;
	read_flag_addr		= virtual_base + READING_FLAG;
	write_flag_addr		= virtual_base + WRITING_FLAG;
	switch_flag_addr 	= virtual_base + SWICH_FLAG;
	key_flag_addr 		= virtual_base + KEY_FLAG;

	int regs_number = ((IN_BASE > OUT_BASE) ? IN_BASE : OUT_BASE) / REG_WIDTH;
	uint8_t key[] = {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80};
	char bytes_red[regs_number * 4];
	uint32_t input;

	//Parameters
	int Keys_repeats = 0;
	int packets_num = 0;
	int packet_repeats = 0;
	float repeatDeley = 0;
	float packetsDeley = 0;

	int showInfo = 0;

	//usleep(2000000);
	if (argc > 2 && (fd_config = open(argv[1], O_RDONLY)) != -1 && (fd_data = open(argv[2], O_RDONLY)) != -1)
	{
		fd_w = open("encrypted_text.txt", (O_RDWR|O_CREAT));
		if (argc > 3) showInfo = atoi(argv[3]);
//CONFIG FILE
		// Read number of Keys_repeats
		Keys_repeats = read_number(fd_config);
		// Read number of packets in input file
		packets_num = read_number(fd_config);
		if (packets_num == 0) packets_num = 1000000;
		// Read number of repeats of packet
		packet_repeats = read_number(fd_config);
		// keyDeley
		repeatDeley = read_number(fd_config);
		// packetsDeley
		packetsDeley = read_number(fd_config);

		if (showInfo == 1)
		{
			printf("[N] - %i\n", Keys_repeats);
			printf("[C] - %i\n", packets_num);
			printf("[K] - %i\n", packet_repeats);
			printf("[Time K1] - %f\n", repeatDeley);
			printf("[Time K2] - %f\n", packetsDeley);
		}
//----------

//DATA FILE
		//0 iteration  
		*(unsigned char *)key_flag_addr = 1;
		*(unsigned char *)key_flag_addr = 0;
		*(unsigned char *)write_flag_addr = 0;
		// Clear all memory
		*(unsigned char *)read_flag_addr = 0;
		*(unsigned char *)key_flag_addr = 0;
		*(unsigned char *)write_flag_addr = 0;
		for(int i = 0; i < regs_number * 2; i++)
		{
			*(uint32_t *)(virtual_base + (REG_WIDTH * i)) = 0;
		}
		while(Keys_repeats > 0)
		{
			//Input Key
			read(fd_config, &input, 1);
			readKey(fd_config, key);
			if (showInfo == 1)
			{
				for (int i = 0; i < 16; ++i)
				{
					printf("%02X", (unsigned int)(key[i] & 0xFF));
				}
				printf("\n");
			}
			for (int regIndex = 0; regIndex < 4; regIndex++)
			{
				input = 0;
				for(int i = 4; i > 0; i--)
				{
					input = input << 8;
					input += key[(regIndex * 4) + i - 1];
				}
				*((uint32_t *)(key_addr + (REG_WIDTH * regIndex))) = input;
			}
			read(fd_config, &input, 1);

			//AES ----------
			//------ reading file cycle
			
			//usleep(2000000);
			bzero(bytes_red, regs_number * 4);
			int red;
			int trace = 0;
			while((red = read(fd_data, bytes_red, regs_number * 4)) > 0)
			{
				int packet_index = 0;
				while(packet_index++ < packet_repeats)
				{
					// input data inside registers
					for (int regIndex = 0; regIndex < regs_number; regIndex++)
					{
						input = 0;
						for(int i = 4; i > 0; i--)
						{
							input = input << 8;
							input += bytes_red[(regIndex * 4) + i - 1];
						}
						*((uint32_t *)(output_addr + (REG_WIDTH * regIndex))) = input;
					}
					//Set flag to FPGA to read data
					*(unsigned char *)key_flag_addr = 1;
					*(unsigned char *)key_flag_addr = 0;
					//Set flag to FPGA to calculate
					*(unsigned char *)write_flag_addr = 0;
					//Wait
					usleep(20 * 1000);
					if (showInfo == 1) printf("Encrypting...\n");
					//Reset the flags 
					*(unsigned char *)write_flag_addr = 0;
					if (showInfo == 1) printf("Repeat N %d\n", packet_index);
					if (repeatDeley > 0) usleep(1000000 * repeatDeley);
				}
				// output data from registers
				uint32_t output;
				for (int regIndex = 0; regIndex < regs_number; regIndex++)
				{
					output = *((uint32_t *)(input_addr + (REG_WIDTH * regIndex)));
					for(int i = 0; i < 4; i++)
					{
						bytes_red[(regIndex * 4) + i] = output & 255;
						output = output >> 8;
					}
				}
				//write encrypted data to encrypted_text.txt
				write(fd_w, bytes_red, 4 * regs_number);

				//clear the array before new cycle
				bzero(bytes_red, regs_number * 4);

				*(unsigned char *)switch_flag_addr = 1;
				// if (packet_repeats > 1)
				// 	usleep(200 * 1000);
				*(unsigned char *)switch_flag_addr = 0;

				if (packetsDeley > 0) usleep(1000000 * packetsDeley);
				if (showInfo == 1) printf("Trace N %d\n", trace);
				trace++;
				if (trace >= packets_num)
				{
					break;
				}
			}
			Keys_repeats--;
		}
// -------------
	}
	else
		write(1, "Usage:\n ./FPGAHPS config.file input_data.file [Show info(0, 1)]\n", 64);
	return 0;
}
