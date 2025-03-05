/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <fs/fat/ff.h>			/* Obtains integer types */
#include <fs/fat/diskio.h>		/* Declarations of disk functions */
#include <drivers/storage/ahci/ahci.h>
#include <string/string.h>
#include <logging/logging.h>

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	ahci_port_t *port = dev_ahci_get_port(0);
	int res = dev_ahci_read(port, sector, count);
	if (res)
		return RES_ERROR;

	memcpy(buff, port->buffer, count * 512);

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	ahci_port_t *port = dev_ahci_get_port(0);
	memcpy(port->buffer, buff, count * 512);
	int res = dev_ahci_write(port, sector, count);
	if (res)
		return RES_ERROR;
	
	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	return RES_OK;
}

uint32_t get_fattime() {
	/*time_t time = rtcGetTime();

	return (DWORD)(time.year + 20) << 25 |
           (DWORD)(time.month) << 21 |
           (DWORD)time.dayOfMonth << 16 |
           (DWORD)time.hour << 11 |
           (DWORD)time.minute << 5 |
           (DWORD)time.second >> 1;*/
	return 0;
}

