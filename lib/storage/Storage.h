#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <Arduino.h>

#define FLASH_SIZE 4096  // 4Ko suffisent pour stocker quelques valeurs
#define STORAGE_BASE_ADDR 0x0000

class StorageClass {
public:
  void read(uint32_t addr, uint8_t *buf, size_t len);
  void write(uint32_t addr, const uint8_t *buf, size_t len);
};

extern StorageClass Storage;

#endif
