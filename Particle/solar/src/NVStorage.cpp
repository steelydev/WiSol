#include "NVStorage.h"

NVStorage::NVStorage(void)
{
  this->get();
} // -----------------------------

void NVStorage::save()
{
  EEPROM.put(0,_store);
} // -----------------------------

void NVStorage::get()
{
  EEPROM.get(0,_store);
} // -----------------------------
