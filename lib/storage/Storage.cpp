#include "Storage.h"
#include "internal/FlashStorage.h"  // présent dans le core CubeCell officiel

void StorageClass::read(uint32_t addr, uint8_t *buf, size_t len) {
  FlashStorage.read(STORAGE_BASE_ADDR + addr, buf, len);
}

void StorageClass::write(uint32_t addr, const uint8_t *buf, size_t len) {
  FlashStorage.write(STORAGE_BASE_ADDR + addr, buf, len);
}

StorageClass Storage;
