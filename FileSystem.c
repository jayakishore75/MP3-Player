#include <stdio.h>
#include "Read_Sector.h"
#include "FileSystem.h"
#include "Directory_Functions_struct.h"
#include "SDCard.h"


uint8_t read8(uint16_t offset, uint8_t * array_name)
{
	uint8_t return_val;

	
	return_val = *(offset+array_name);
	
	return return_val;
}


uint16_t read16(uint16_t offset, uint8_t * array_name)
{
	uint16_t return_val = 0;
	uint8_t i, temp;

	return_val = 0;

	for(i=0;i<2;i++)
	{
		temp=*(offset+array_name+1-i);							
		return_val=return_val<<8;
		return_val=return_val|temp;
	}


	return return_val;
}



uint32_t read32(uint16_t offset, uint8_t * array_name)
{
	uint32_t return_val;
	uint8_t temp,i;

	return_val = 0;

	for(i=0;i<4;i++)
	{
		temp=*(offset+array_name+3-i);							
		return_val=return_val<<8;
		return_val=return_val|temp;
	}

	
	return return_val;
}


/*
mount drive Initialistaion
*/

uint8_t mount_drive(uint8_t xdata * array_in)
{	
	uint8_t error_flag;
	uint8_t value, Num_FAT;
	uint32_t MBR_Rel_Sec, FAT_Sz, Tot_Sec, Root_Clus, Num_Data_Sec, Cluster_Count;
	uint16_t Rsvd_Sec_Cnt, Root_Ent_Cnt;
	FS_values_t * Drive_Value;

	printf("\n Mount Drive initialised ");
	Drive_Value = Export_Drive_values();

	error_flag = Read_Sector(0, 512, array_in);	

	if(error_flag == no_errors)
	{
		value = read8(0, array_in);
		if(value != 0xEB && value != 0xE9)
		{
			
			MBR_Rel_Sec = read32(0x01c6, array_in);
			Read_Sector(MBR_Rel_Sec, 512, array_in);
			value = read8(0,array_in);
			
			if(value != 0xEB && value != 0xE9)
			{
				printf("\nMount Drive: sector offset:%lu, first8bit:%bx", MBR_Rel_Sec, value);
				error_flag = Disk_Error;
				return error_flag;
			}
		}	
	}		
	
	Drive_Value->BytesPerSec = read16(0x0B, array_in);
	
	Drive_Value->SecPerClus = read8(0x0D, array_in);
	Rsvd_Sec_Cnt = read16(0x0E, array_in);
	Num_FAT = read8(0x10, array_in);
	Root_Ent_Cnt = read16(0x11, array_in);

	Tot_Sec = read16(0x13, array_in);
	FAT_Sz = read16(0x16, array_in);

	if(Tot_Sec == 0) Tot_Sec = read32(0x20, array_in);
	if(FAT_Sz == 0) 
	{
		FAT_Sz = read32(0x24, array_in);
		Root_Clus = read32(0x2c, array_in);
	}

	Drive_Value->RootDirSecs = ((Root_Ent_Cnt * 32) + (Drive_Value->BytesPerSec - 1)) / Drive_Value->BytesPerSec;

	Num_Data_Sec = Tot_Sec - (Rsvd_Sec_Cnt + (Num_FAT * FAT_Sz) + Drive_Value->RootDirSecs);
	Cluster_Count = Num_Data_Sec / Drive_Value->SecPerClus;

	if(Cluster_Count < 65525)
		Drive_Value->FATtype=FAT16;
	else
		Drive_Value->FATtype=FAT32;

	Drive_Value->StartofFAT = Rsvd_Sec_Cnt + MBR_Rel_Sec;

 	Drive_Value->FirstDataSec = (Num_FAT * FAT_Sz) + Drive_Value->RootDirSecs + Drive_Value->StartofFAT;
	
	if(Drive_Value->FATtype == FAT16)
	{
		Drive_Value->FirstRootDirSec = Drive_Value->StartofFAT + (Num_FAT * FAT_Sz);
	}
	else
	{
		Drive_Value->FirstRootDirSec = ((Root_Clus - 2) * Drive_Value->SecPerClus) + Drive_Value->FirstDataSec;
	}
	printf("\nBytesPerSec:%x", Drive_Value->BytesPerSec);
	printf("\nFATtype:%bx", Drive_Value->FATtype);
	printf("\nSecPerClus:%bx", Drive_Value->SecPerClus );
	printf("\nuRsvdSecCnt:%x", Rsvd_Sec_Cnt);
	printf("\nNumFAT:%bx", Num_FAT);
	printf("\nRootEntCnt:%x", Root_Ent_Cnt);
	printf("\nTotSec:%lx",Tot_Sec);
	printf("\nFATSz:%lx", FAT_Sz);
	printf("\nRootDirSecs:%lu", Drive_Value->RootDirSecs);
	printf("\nNumDataSec:%lu", Num_Data_Sec);
	printf("\nCountOfCluster:%lu", Cluster_Count);
	printf("\nStartofFAT:%lu", Drive_Value->StartofFAT);
	printf("\nFirstDataSec:%lu",Drive_Value->FirstDataSec );
	printf("\nRootClus:%lu", Root_Clus);
	printf("\nFirstRootDirSec:%lu\n\n", Drive_Value->FirstRootDirSec);
	
	return error_flag;
}

/*
Finding Starting sector of cluster
*/
uint32_t First_Sector (uint32_t Cluster_Num)
{
	
	FS_values_t * Drive_Value = Export_Drive_values();

	
	return (Cluster_Num == 0) ? Drive_Value->FirstRootDirSec : ((Cluster_Num - 2) * Drive_Value->SecPerClus) + Drive_Value->FirstDataSec;

}


uint32_t Find_Next_Clus(uint32_t Cluster_num, uint8_t xdata * in_array)
{
   uint32_t ui32SecNum, clus_val, FAToffset;
   uint16_t ThisFATOffset;
   
   
   FS_values_t * Drive_Value = Export_Drive_values();

   printf("\nNext_Clus_Locate\n");
   ui32SecNum = ((Cluster_num * Drive_Value->FATtype) / Drive_Value->BytesPerSec) + Drive_Value->StartofFAT;
   Read_Sector(ui32SecNum, Drive_Value->BytesPerSec, in_array);

   FAToffset = Cluster_num * Drive_Value->FATtype;

   ThisFATOffset = FAToffset % Drive_Value->BytesPerSec;

   if(Drive_Value->FATtype == FAT32)  
   {
       clus_val = read32(ThisFATOffset, in_array);
	   clus_val &= 0x0FFFFFFF;
   }
   else if(Drive_Value->FATtype == FAT16)   
   {
       clus_val = (uint32_t)(read16(ThisFATOffset, in_array));
   }

   return clus_val;
}