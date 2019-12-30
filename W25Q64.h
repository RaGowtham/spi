void W25Q64_begin(uint8_t cs);

uint8_t W25Q64_readStatusReg1(void);

uint8_t W25Q64_readStatusReg2();

void W25Q64_readManufacturer(uint8_t* d);

void W25Q64_readUniqieID(uint8_t* d);

bool W25Q64_IsBusy(void);

void W25Q64_powerDown(void);

void W25Q64_WriteEnable(void);

void W25Q64_WriteDisable(void);

uint16_t W25Q64_read(uint32_t addr,uint8_t *buf,uint16_t n);

uint16_t W25Q64_fastread(uint32_t addr,uint8_t *buf,uint16_t n);

bool  W25Q64_eraseSector(uint16_t sect_no, bool flgwait);

bool  W25Q64_erase64Block(uint16_t blk_no, bool flgwait);

bool  W25Q64_erase32Block(uint16_t blk_no, bool flgwait);

bool  W25Q64_eraseAll(bool flgwait);

uint16_t W25Q64_pageWrite(uint16_t sect_no, uint16_t inaddr, uint8_t* data, uint16_t n);


int wiringPiSPISetup(char *device, int speed, int mode);
