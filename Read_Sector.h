#ifndef _Read_Sector_H
#define _Read_Sector_H
#include "main.h"
#include "SDCard.h"

uint8_t Read_Sector(uint32_t sector_number, uint16_t sector_size, uint8_t xdata * array_for_data);
#endif






