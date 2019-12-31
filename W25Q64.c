//
// Flash Memory W25Q64 Access Library for RaspberryPi
//

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "W25Q64.h"

#define MAX_BLOCKSIZE        128    // ブロック総数
#define MAX_SECTORSIZE       2048   // 総セクタ数

#define CMD_WRIRE_ENABLE      0x06
#define CMD_WRITE_DISABLE     0x04
#define CMD_READ_STATUS_R1    0x05
#define CMD_READ_STATUS_R2    0x35
#define CMD_WRITE_STATUS_R    0x01 // 未実装
#define CMD_PAGE_PROGRAM      0x02
#define CMD_QUAD_PAGE_PROGRAM 0x32 // 未実装
#define CMD_BLOCK_ERASE64KB   0xd8
#define CMD_BLOCK_ERASE32KB   0x52
#define CMD_SECTOR_ERASE      0x20
#define CMD_CHIP_ERASE        0xC7
#define CMD_ERASE_SUPPEND     0x75 // 未実装
#define CMD_ERASE_RESUME      0x7A // 未実装
#define CMD_POWER_DOWN        0xB9
#define CMD_HIGH_PERFORM_MODE 0xA3 // 未実装
#define CMD_CNT_READ_MODE_RST 0xFF // 未実装
#define CMD_RELEASE_PDOWN_ID  0xAB // 未実装
#define CMD_MANUFACURER_ID    0x90
#define CMD_READ_UNIQUE_ID    0x4B
#define CMD_JEDEC_ID          0x9f

#define CMD_READ_DATA         0x03
#define CMD_FAST_READ         0x0B
#define CMD_READ_DUAL_OUTPUT  0x3B // 未実装
#define CMD_READ_DUAL_IO      0xBB // 未実装
#define CMD_READ_QUAD_OUTPUT  0x6B // 未実装
#define CMD_READ_QUAD_IO      0xEB // 未実装
#define CMD_WORD_READ         0xE3 // 未実装

#define SR1_BUSY_MASK	0x01
#define SR1_WEN_MASK	0x02


#define UNUSED(a) ((void)(a))

static uint8_t _spich;
static int fds[5];
static int lspeed[5];

int wiringPiSPISetup(char *device, int speed, int mode)
{
	int fd;
	int bps = 8;

	printf("%s\n",device);

	if((fd = open(device, O_RDWR)) < 0)
	{
		printf("Error : %s Device is not exist\n",device);
		return -1;
	}

	if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0)
	{
		printf("Error in setting mode\n");
		return -1;
	}

	if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
	{
		printf("Error in setting speed\n");
		return -1;
	}

	if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bps) < 0)
	{
		printf("Error in setting bps\n");
		return -1;
	}

	lspeed[_spich] = speed;
	fds[_spich] = fd;

	return _spich;
}

int wiringPiSPIDataRW(int channel, unsigned char *data, int len)
{
	struct spi_ioc_transfer spi;

	memset(&spi, 0, sizeof(spi));

	spi.tx_buf = (unsigned long)data;
	spi.rx_buf = (unsigned long)data;
	spi.len = len;
	spi.delay_usecs = 0;
	spi.speed_hz = 2000000;
	spi.bits_per_word = 8;

	return ioctl(fds[channel], SPI_IOC_MESSAGE(1), &spi);
}

void spcDump(char *id,int rc, uint8_t *data,int len) {
    int i;
    printf("[%s] = %d\n",id,rc);
    for(i=0;i<len;i++) {
      printf("%0x ",data[i]);
      if ( (i % 10) == 9) printf("\n");
    }
    printf("\n");
}

//
// フラッシュメモリ W25Q64の利用開始
// 
void W25Q64_begin(uint8_t spich) {
    _spich = spich;
}

//
// ステータスレジスタ1の値取得
// 戻り値: ステータスレジスタ1の値
//
uint8_t W25Q64_readStatusReg1(void) {
  unsigned char data[2];
  int rc;
  UNUSED(rc);
  data[0] = CMD_READ_STATUS_R1;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("readStatusReg1",rc,data,2);
  return data[1];
}

//
// ステータスレジスタ2の値取得
// 戻り値: ステータスレジスタ2の値
//
uint8_t W25Q64_readStatusReg2(void) {
  unsigned char data[2];
  int rc;
  UNUSED(rc);
  data[0] = CMD_READ_STATUS_R2;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("readStatusReg2",rc,data,2);
  return data[1];
}

//
// JEDEC ID(Manufacture, Memory Type,Capacity)の取得
// d(out) :Manufacture, Memory Type,Capacityの３バイトを格納する
//
void W25Q64_readManufacturer(uint8_t* d) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  memset(data,0,sizeof(data));
  data[0] = CMD_JEDEC_ID;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("readManufacturer",rc,data,4);
  memcpy(d,&data[1],3);
}

//
// Unique IDの取得
// d(out): Unique ID 7バイトを返す  
//
void W25Q64_readUniqieID(uint8_t* d) {
  unsigned char data[12];
  int rc;
  UNUSED(rc);
  memset(data,0,sizeof(data));
  data[0] = CMD_READ_UNIQUE_ID;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("readUniqieID",rc,data,12);
  memcpy(d,&data[5],7);
}

//
// 書込み等の処理中チェック
// 戻り値: true:作業 、false:アイドル中
//
bool W25Q64_IsBusy(void) {
  unsigned char data[2];
  int rc;
  UNUSED(rc);
  data[0] = CMD_READ_STATUS_R1;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("IsBusy",rc,data,2);
  uint8_t r1;
  r1 = data[1];
  if(r1 & SR1_BUSY_MASK) return true;
  return false;
}

//
//　パワーダウン指定 
//
void W25Q64_powerDown(void) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);
  data[0] = CMD_POWER_DOWN;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("powerDown",rc,data,1);
}

//
// 書込み許可設定
//
void W25Q64_WriteEnable(void) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);
  data[0] = CMD_WRIRE_ENABLE;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("WriteEnable",rc,data,1);
}

//
// 書込み禁止設定
//
void W25Q64_WriteDisable(void) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);
  data[0] = CMD_WRITE_DISABLE;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
  //spcDump("WriteDisable",rc,data,1);
}

//
// データの読み込み
// addr(in): 読込開始アドレス (24ビット 0x000000 - 0xFFFFFF)
// n(in):読込データ数
//
uint16_t W25Q64_read(uint32_t addr,uint8_t *buf,uint16_t n){ 
  unsigned char *data;
  int rc;

  data = (unsigned char*)malloc(n+4);
  data[0] = CMD_READ_DATA;
  data[1] = (addr>>16) & 0xFF;     // A23-A16
  data[2] = (addr>>8) & 0xFF;      // A15-A08
  data[3] = addr & 0xFF;           // A07-A00
  rc = wiringPiSPIDataRW (_spich,data,n+4);
  //spcDump("read",rc,data,rc);
  memcpy(buf,&data[4],n);
  free(data);
  return rc-4;
}

//
// 高速データの読み込み
// addr(in): 読込開始アドレス (24ビット 0x00000 - 0xFFFFF)
// n(in):読込データ数
//
uint16_t W25Q64_fastread(uint32_t addr,uint8_t *buf,uint16_t n) {
  unsigned char *data;
  int rc;

  data = (unsigned char*)malloc(n+5);
  data[0] = CMD_FAST_READ;
  data[1] = (addr>>16) & 0xFF;     // A23-A16
  data[2] = (addr>>8) & 0xFF;      // A15-A08
  data[3] = addr & 0xFF;           // A07-A00
  data[4] = 0;
  rc = wiringPiSPIDataRW (_spich,data,n+5);
  //spcDump("fastread",rc,data,rc);
  memcpy(buf,&data[5],n);
  free(data);
  return rc-5;
}

//
// セクタ単位消去(4kb空間単位でデータの消去を行う)
// sect_no(in) セクタ番号(0 - 2048)
// flgwait(in) true:処理待ちを行う false:待ち無し
// 戻り値: true:正常終了 false:失敗
//  補足： データシートでは消去に通常 30ms 、最大400msかかると記載されている
//         アドレス23ビットのうち上位 11ビットがセクタ番号の相当する。下位12ビットはセクタ内アドレスとなる。
//
bool W25Q64_eraseSector(uint16_t sect_no, bool flgwait) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  uint32_t addr = sect_no;
  addr<<=12;

  W25Q64_WriteEnable();
  data[0] = CMD_SECTOR_ERASE;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xff;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
 
  // 処理待ち
  while(W25Q64_IsBusy() & flgwait) {
    sleep(1);
  }
  return true;
}

//
// 64KBブロック単位消去(64kb空間単位でデータの消去を行う)
// blk_no(in) ブロック番号(0 - 127)
// flgwait(in) true:処理待ちを行う false:待ち無し
// 戻り値: true:正常終了 false:失敗
//   補足: データシートでは消去に通常 150ms 、最大1000msかかると記載されている
//         アドレス23ビットのうち上位 7ビットがブロックの相当する。下位16ビットはブロック内アドレスとなる。
//
bool W25Q64_erase64Block(uint16_t blk_no, bool flgwait) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  uint32_t addr = blk_no;
  addr<<=16;

  // 書込み許可設定
  W25Q64_WriteEnable();

  data[0] = CMD_BLOCK_ERASE64KB;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xff;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
 
  // 処理待ち
  while(W25Q64_IsBusy() & flgwait) {
    sleep(1);
  }
  return true;
}

//
// 32KBブロック単位消去(32kb空間単位でデータの消去を行う)
// blk_no(in) ブロック番号(0 - 255)
// flgwait(in) true:処理待ちを行う false:待ち無し
// 戻り値: true:正常終了 false:失敗
//   補足: データシートでは消去に通常 120ms 、最大800msかかると記載されている
//         アドレス23ビットのうち上位 8ビットがブロックの相当する。下位15ビットはブロック内アドレスとなる。
//
bool W25Q64_erase32Block(uint16_t blk_no, bool flgwait) {
  unsigned char data[4];
  int rc;
  UNUSED(rc);
  uint32_t addr = blk_no;
  addr<<=15;

  // 書込み許可設定
  W25Q64_WriteEnable();  

  data[0] = CMD_BLOCK_ERASE32KB;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xff;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));
 
  // 処理待ち
  while(W25Q64_IsBusy() & flgwait) {
    sleep(1);
  }
  return true;
}

//
// 全領域の消去
// flgwait(in) true:処理待ちを行う false:待ち無し
// 戻り値: true:正常終了 false:失敗
//   補足: データシートでは消去に通常 15s 、最大30sかかると記載されている
//
bool W25Q64_eraseAll(bool flgwait) {
  unsigned char data[1];
  int rc;
  UNUSED(rc);

  // 書込み許可設定
  W25Q64_WriteEnable();  

  data[0] = CMD_CHIP_ERASE;
  rc = wiringPiSPIDataRW (_spich,data,sizeof(data));

  // 処理待ち
  while(W25Q64_IsBusy() & flgwait) {
    sleep(1);
  }
  return true;
}

//
// データの書き込み
// sect_no(in) : セクタ番号(0x00 - 0x7FF) 
// inaddr(in)  : セクタ内アドレス(0x00-0xFFF)
// data(in)    : 書込みデータ格納アドレス
// n(in)       : 書込みバイト数(0～256)
//
uint16_t W25Q64_pageWrite(uint16_t sect_no, uint16_t inaddr, uint8_t* buf, uint16_t n) {
  if (n > 256) return 0;
  unsigned char *data;
  int rc;

  uint32_t addr = sect_no;
  addr<<=12;
  addr += inaddr;

  // 書込み許可設定
  W25Q64_WriteEnable();  
  if (W25Q64_IsBusy()) return 0;  

  data = (unsigned char*)malloc(n+4);
  data[0] = CMD_PAGE_PROGRAM;
  data[1] = (addr>>16) & 0xff;
  data[2] = (addr>>8) & 0xff;
  data[3] = addr & 0xFF;
  memcpy(&data[4],buf,n);
  rc = wiringPiSPIDataRW (_spich,data,n+4);
  //spcDump("pageWrite",rc,buf,n);

  // 処理待ち
  while(W25Q64_IsBusy()) ;
  free(data);
  return rc;
}
