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
//  Last Modified : <180808.1520>
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
    Serial.print("*** FileIO::begin() entered: ");
    Serial.print("flashstarted = "); Serial.println(flashstarted);
    if (!flashstarted) {
        if (!flash.begin(FLASH_TYPE)) {
            return false;
        }
        flashstarted = true;
    }
    Serial.print("*** FileIO::begin(): (before while) mounted = ");Serial.println(mounted);
    while (!mounted) {
        Serial.println("*** FileIO::begin(): Mounting file system");
        mounted = fatfs.begin();
        Serial.println("*** FileIO::begin(): in while: mounted = ");Serial.println(mounted);
        if (!mounted) {
            Serial.println("*** FileIO::begin(): Formatting File system...");
            fatfs.activate();
            DWORD plist[] = {100, 0, 0, 0};
            uint8_t buf[512] = {0}; 
            FRESULT r = f_fdisk(0, plist, buf);
            if (r != FR_OK) return false;
            r = f_mkfs("", FM_ANY, 0, buf, sizeof(buf));
            if (r != FR_OK) return false;
        }
    }
    Serial.print("*** FileIO::begin(): (after while) mounted = ");Serial.println(mounted);
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

void FileIO::DumpToSerial(const char *name)
{
    char buffer[8];
    File file = fatfs.open(name,FILE_READ);
    if (file) {
        Serial.println(name);
        for (int i = 0; i < file.size(); i++) {
            if ((i % 16) == 0) {
                if (i > 0) Serial.println();
                sprintf(buffer,"%04X:",i);
                Serial.print(buffer);
            }
            sprintf(buffer," %02X",file.read());
            Serial.print(buffer);
        }
        Serial.println();
        file.close();
    }
}

void FileIO::ListToSerial(const char *name)
{
    File file = fatfs.open(name,FILE_READ);
    if (file) {
        Serial.println(name);
        for (int i = 0; i < file.size(); i++) {
            Serial.print((char)file.read());
        }
        file.close();
    }
}

int FileIO::DeleteFile(const char *name)
{
    fatfs.remove(name);
}

int FileIO::EraseDisk()
{
    File root = fatfs.open("/");
    if (!root) return -1;
    File child = root.openNextFile();
    while (child) {
        if (!child.isDirectory()) {
            if (!remove(child.name())) {
                return -1;
            } else {
                if (!fatfs.rmdir(child.name())) {
                    return -1;
                }
            }
        }
        child = root.openNextFile();
    }
    root.close();
    return 0;
}

int FileIO::ListFiles(void(* callback_func) (File file,void *userdata),
                      void *userdata)
{
    File root = fatfs.open("/");
    if (!root) return -1;
    File child = root.openNextFile();
    while (child) {
        (*callback_func)(child,userdata);
        child = root.openNextFile();
    }
    root.close();
    return 0;
}
    
                
