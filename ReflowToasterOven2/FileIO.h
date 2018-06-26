// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sun Jun 24 19:35:11 2018
//  Last Modified : <180624.1955>
//
//  Description	
//
//  Notes
//
//  History
//	
/////////////////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2018  Robert Heller D/B/A Deepwoods Software
//			51 Locke Hill Road
//			Wendell, MA 01379-9728
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// 
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __FILEIO_H
#define __FILEIO_H

#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>

#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV

#if defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS)
#define FLASH_SS       SS                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI                   // What SPI port is Flash on?
#define NEOPIN         8
#else
#define FLASH_SS       SS1                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
#define NEOPIN         40
#endif

class FileIO {
public:
    FileIO() {flashstarted = mounted = false;}
    bool begin();
    int WriteFile(const char *name,uint8_t *data,size_t length);
    int ReadFile(const char *name,uint8_t *data,size_t length);
private:
    bool flashstarted;
    bool mounted;
};

#endif // __FILEIO_H

