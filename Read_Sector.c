

#include "Read_Sector.h"
#include "SDCard.h"
#include "PORT.h"

uint8_t Read_Sector(uint32_t sector_number, uint16_t sector_size, uint8_t xdata * array_for_data)
{	
	uint8_t error_flag = no_errors;
	
	nCS0 = 0;
	error_flag = SEND_COMMAND (17, sector_number);
	if(error_flag==no_errors)
	{read_block (sector_size, array_for_data);}
	else
	{error_flag = Disk_error;}
	nCS0 = 1;

	return error_flag;
}


