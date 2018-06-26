// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sun Jun 24 19:37:16 2018
//  Last Modified : <180624.1954>
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

static const char rcsid[] = "@(#) : $Id$";

#include "FileIO.h"
#include "utility/ff.h"

Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI
Adafruit_W25Q16BV_FatFs fatfs(flash);


bool FileIO::begin() {
    if (!flashstarted) {
        if (!flash.begin(FLASH_TYPE)) {
            return false;
        }
        flashstarted = true;
    }
    if (!mounted) {
        if (!fatfs.begin()) {
            fatfs.activate();
            DWORD plist[] = {100, 0, 0, 0};
            uint8_t buf[512] = {0}; 
            FRESULT r = f_fdisk(0, plist, buf);
            if (r != FR_OK) return false;
            r = f_mkfs("", FM_ANY, 0, buf, sizeof(buf));
            if (r != FR_OK) return false;
            mounted = fatfs.begin();
        }
    }
    return mounted;
}


int FileIO::WriteFile(const char *name,uint8_t *data,size_t length)
{
    File file = fatfs.open(name,FILE_WRITE);
    if (file) {
        int w = file.write(data,length);
        file.close();
        return w;
    } else {
        return -1;
    }
}

int FileIO::ReadFile(const char *name,uint8_t *data,size_t length)
{
    File file = fatfs.open(name,FILE_READ);
    if (file) {
        int r = file.read(data,length);
        file.close();
        return r;
    } else {
        return -1;
    }
}
