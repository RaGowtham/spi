#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "W25Q64.h"

void dump(uint8_t *dt, uint32_t n) {
  uint16_t clm = 0;
  uint8_t data;
  uint8_t sum;
  uint8_t vsum[16];
  uint8_t total =0;
  uint32_t saddr =0;
  uint32_t eaddr =n-1;
  
  printf("---------------------------------------------------------------\n");
  uint16_t i;
  for(i = 1; i < n + 1; i++)
  {
	  printf("%02x  ",dt[i -1]);
	  if(i % 16 == 0)
	  {
		  printf("\n");
	  }
  }
  if(i % 17 != 0)
	  printf("\n");
  printf("---------------------------------------------------------------\n");
}

int main(int argc, char *argv[]) {
	uint8_t buf[256];  

	char *device;
	int speed, mode;
	int i, option;

	uint16_t n;      

	if(argc != 2)
	{
		printf("Usage : main /dev/spidev*.*\n");
		return -1;
	}

	if (wiringPiSPISetup(argv[1], 2000000, 0) < 0) {
		printf("SPISetup failed:\n");
	}

	// JEDEC ID Get
	W25Q64_readManufacturer(buf);
	printf("JEDEC ID : ");
	for (i=0; i< 3; i++) {
		printf("%x ",buf[i]);
	}
	printf("\n");

	// Unique ID Get
	W25Q64_readUniqieID(buf);
	printf("Unique ID : ");
	for (i=0; i< 7; i++) {
		printf("%x ",buf[i]);
	}
	printf("\n");

	printf("Enter your option:\n");
	printf("1. Read data\n");
	printf("1. Read data using fast read command\n");
	printf("3. Write data\n");
	printf("4. Erase data by sector\n");
	printf("5. Erase all\n");
	printf("6. Print status register status\n");
	printf("1 - 6 : ");
	scanf("%d",&option);

	// Read 256 byte data from Address=0
	if(option == 1)
	{
		long sector, address, size;

		printf("Enter sector no: ");
		scanf("%d", &sector);

		printf("Enter address : ");
		scanf("%d", &address);

		printf("Enter the read size : ");
		scanf("%d", &size);

		if(size < 0 || size > 256)
		{
			printf("Valid size is 0 - 256\n");
			return -1;
		}

		memset(buf,0,size);
		uint32_t addr = sector;
		addr<<=12;
		addr += address;
		n =  W25Q64_read(addr, buf, size);
		printf("Read Data: n=%d\n",n);
		dump(buf, size);
	}

	// First read 256 byte data from Address=0
	else if(option == 2)
	{
		long sector, address, size;

		printf("Enter sector no: ");
		scanf("%d", &sector);

		printf("Enter address : ");
		scanf("%d", &address);

		printf("Enter the read size : ");
		scanf("%d", &size);

		if(size < 0 || size > 256)
		{
			printf("Valid size is 0 - 256\n");
			return -1;
		}

		memset(buf,0, size);
		uint32_t addr = sector;
		addr<<=12;
		addr += address;
		n =  W25Q64_fastread(addr, buf, size);
		printf("Fast Read Data: n=%d\n",n);
		dump(buf,size);
	}

	// Erase data by Sector
	else if(option == 4)
	{
		int sector;

		printf("Enter the sector : ");
		scanf("%d",&sector);
		if(sector < 0)
		{
			printf("Invalid input sector\n");
			return -1;
		}

		n = W25Q64_eraseSector(sector, true);

		printf("Erase Sector(%d):", sector);
		memset(buf,0,256);
		n =  W25Q64_read (0, buf, 256);
		dump(buf,256);
	}

	else if(option == 3)
	{
		long sector, address, size;
		printf("Note This tool will erace the sectors which is going to written\n");

		printf("Enter sector no: ");
		scanf("%d", &sector);

		printf("Enter addtess : ");
		scanf("%d", &address);

		printf("Enter the write size : ");
		scanf("%d", &size);

		unsigned char wdata[size];
		int data;

		printf("Enter write data : \n");

		for (i=0; i < size;i++) {
			printf("%d -> ", i);
			scanf("%x",&data);
			wdata[i]= data & 0xFF;
		}  

		n = W25Q64_eraseSector(sector,true);

		n =  W25Q64_pageWrite(sector, address, wdata, size);

		memset(buf,0,size);
		uint32_t addr = sector;
		addr<<=12;
		addr += address;
		n =  W25Q64_read(addr, buf, size);
		printf("Read Data: n=%d\n",n);
		dump(buf, size);
	}

	else if(option == 5)
	{
		printf("Erasing all sectors\n");
		W25Q64_eraseAll(1);
		printf("Done\n");
	}
	else if (option == 6)
	{

		// Get fron Status Register1
		buf[0] = W25Q64_readStatusReg1();
		printf("Status Register-1: %x\n",buf[0]);

		// Get fron Status Register2
		buf[0] = W25Q64_readStatusReg2();
		printf("Status Register-2: %x\n",buf[0]);
	}
	else
	{
		printf("Invalid option\n");
	}
	return 0;
}
