#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "Main.h"							

uint8_t read8(uint16_t offset, uint8_t *array_name);

uint16_t read16(uint16_t offset, uint8_t *array_name);

uint32_t read32(uint16_t offset, uint8_t *array_name);

uint8_t mount_drive(uint8_t xdata * in_array);

uint32_t First_Sector(uint32_t Cluster_Num);

uint32_t Find_Next_Clus(uint32_t Cluster_num, uint8_t xdata * in_array);
#endif

