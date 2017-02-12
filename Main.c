
#include "AT89C51RC2.h"
#include "stdio.h"
#include "main.h"
#include "PORT.H"
#include "UART.h"
#include "print_bytes.h"
#include "SPI.h"
#include "SDcard.h"
#include "Long_Serial_In.h"
#include "LCD_routines.h"
#include "hardware_delay_T1_1ms.h"
#include "Directory_Functions_struct.h"
#include "Read_Sector.h"
#include "FileSystem.h"

uint8_t code SD_start[]="SD Card Init...";
uint8_t code SDSC[]="Std. Capacity";
uint8_t code SDHC[]="High Capacity";

xdata uint8_t buf1[512];
xdata uint8_t buf2[512];


main()
{
	uint32_t clus_num;
	uint16_t i, Num_of_entries, input_entry, Current_Directory;
	uint8_t error_flag;
	uint8_t SD_stat;
	FS_values_t * pFSval;

	AUXR=0x0c;   // make all of XRAM available, ALE always on
	if(OSC_PER_INST==6)  // sets the x2 bit according to the OSC_PER_INST value
	{
		CKCON0=0x01;  // set X2 clock mode
	}
	else if(OSC_PER_INST==12)
	{
		CKCON0=0x00;  // set standard clock mode
	}


	AMBERLED=1;
	YELLOWLED=1;
	GREENLED=1;
	REDLED = 0;
	i=0;
	while(i<=60000) i++;
	REDLED = 1;
	uart_init();
	LCD_Init();


	printf("SD Card File System Program\n\r\n\n");
	LCD_Write(COMMAND,set_ddram_addr|line1);
	DELAY_1ms_T1(1);
	LCD_Print(0,SD_start);   
	error_flag=SPI_Master_Init(400000UL);
	if(error_flag!=no_errors)
	{
		REDLED=0;  // An error causes the program to stop
		UART_Transmit('S');
		while(1);
	}
	AMBERLED=0;
	// SD Card Initialization
	error_flag=SD_card_init();
	if(error_flag!=no_errors)
	{
		REDLED=0;  // An error causes the program to stop
		UART_Transmit('C');
		while(1);
	}
	AMBERLED=1;
	error_flag=SPI_Master_Init(20000000UL);
	if(error_flag!=no_errors)
	{
		REDLED=0;  // An error causes the program to stop
		UART_Transmit('P');
		while(1);
	}
	for(i=0;i<512;i++)
	{
		buf1[i]=0xff;  // erase valout for debug
		//      buf2[i]=0xff;
	}
	LCD_Write(COMMAND,set_ddram_addr|line2);
	DELAY_1ms_T1(1);
	SD_stat=Return_SD_Card_Type();
	if(SD_stat==Standard_Capacity)
	{
		LCD_Print(0,SDSC);
	}
	else if(SD_stat==High_Capacity)
	{
		LCD_Print(0,SDHC);
	}

	// mount_drive
	mount_drive(buf2);
	
	pFSval = Export_Drive_values();
	Current_Directory = pFSval->FirstRootDirSec;
	Num_of_entries = Print_Directory(pFSval->FirstRootDirSec, buf1);
	
	while(1)
	{
		
		printf("\nEnter Input: ");
		input_entry = long_serial_input();
		
		if(input_entry <= Num_of_entries)
		{
			clus_num = Read_Dir_Entry(Current_Directory, input_entry, buf1);
				
			if(clus_num & 0x10000000){
				
				
				clus_num &= 0x0FFFFFFF; //masking top nibble
				Current_Directory = First_Sector(clus_num);
				Num_of_entries = Print_Directory(Current_Directory, buf1);
			}
			else 
			{
				print_file(clus_num, buf1);
				Num_of_entries = Print_Directory(Current_Directory, buf1);
			}
		}
		else
		{
			printf("\n Not Valid ");
		}
	}
} 



