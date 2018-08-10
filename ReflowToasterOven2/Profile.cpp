// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 15:41:47 2018
//  Last Modified : <180810.1114>
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
//    (Portions Copyright (C) 2011 Frank Zhao)
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

#include <Arduino.h>
#include "Profile.h"
#include <stdlib.h>
#include "dtostrg.h"
#include "u16tostru.h"
#include "strtou16.h"

static const char rcsid[] PROGMEM = "@(#) : $Id$";


void Profile::Default() {
    start_rate = 1;
    soak_temp1 = 150.0;
    soak_temp2 = 185.0;
    soak_length = 70;
    peak_temp = 217.5;
    time_to_peak = 45;
    cool_rate = 2.0;
}

bool Profile::Valid() {
    return (start_rate > 0.0 &&
            soak_temp1 > 0.0 &&
            soak_temp2 >= soak_temp1 &&
            peak_temp >= soak_temp2 &&
            cool_rate > 0.0);
}

void Profile::Load() {
    // load the data block from EEPROM
    char *filebuffer,*p,*p2;
    File fp = fatfs.open("Profile.dat",FILE_READ);
    if (fp) {
                size_t len = fp.size();
        filebuffer = (char *)calloc(sizeof(char),len);
        fp.read(filebuffer,len);
        fp.close();
        p = filebuffer;
        start_rate = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        soak_temp1 = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        soak_temp2 = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        soak_length = strtou16(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        peak_temp = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        time_to_peak = strtou16(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        cool_rate = strtod(p,&p2);
        p = p2;
        free(filebuffer);
        // verify data validity
        if (!Valid()) {
            // data not valid, reset to defaults
            Default();
            Save();
        }
    }
}

void Profile::Save() {
    
    if (!filesystem.begin()) return;
    File fp = fatfs.open("Profile.dat",FILE_READ);
    if (fp) {
        fp.close();
        filesystem.DeleteFile("Profile.dat");
    }
    fp = fatfs.open("Profile.dat",FILE_WRITE);
    if (fp) {
        static char buffer[16];
        dtostrg(start_rate,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(soak_temp1,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(soak_temp2,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        u16tostru(soak_length,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(peak_temp,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        u16tostru(time_to_peak,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(cool_rate,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        fp.close();
    }
}
void Profile::DownloadFromSerial()
{
    char LineBuffer[64],*p2;
    int line = 0;
    Serial.println("Profile?");
    while (1) {
        delay(100);
        int bytes = Serial.readBytesUntil('\n',LineBuffer,sizeof(LineBuffer));
        if (bytes == 0) {
            if (line < 7) continue;
            else break;
        }
        line++;
        switch (line) {
        case 1:
            start_rate = strtod(LineBuffer,&p2);
            break;
        case 2:
            soak_temp1 = strtod(LineBuffer,&p2);
            break;
        case 3:
            soak_temp2 = strtod(LineBuffer,&p2);
            break;
        case 4:
            soak_length = strtou16(LineBuffer,&p2);
            break;
        case 5:
            peak_temp = strtod(LineBuffer,&p2);
            break;
        case 6:
            time_to_peak = strtou16(LineBuffer,&p2);
            break;
        case 7:
            cool_rate = strtod(LineBuffer,&p2);
            break;
        default:
            break;
        }
        Serial.println(line);
    }
    // verify data validity
    if (!Valid()) {
        // data not valid, reset to defaults
        Default();
        Save();
    }
}

void Profile::UploadToSerial()
{
    static char buffer[16];
    Serial.println("Profile>");
    dtostrg(start_rate,10,5,buffer);
    Serial.println(buffer);
    dtostrg(soak_temp1,10,5,buffer);
    Serial.println(buffer);
    dtostrg(soak_temp2,10,5,buffer);
    Serial.println(buffer);
    u16tostru(soak_length,5,buffer);
    Serial.println(buffer);
    dtostrg(peak_temp,10,5,buffer);
    Serial.println(buffer);
    u16tostru(time_to_peak,5,buffer);
    Serial.println(buffer);
    dtostrg(cool_rate,10,5,buffer);
    Serial.println(buffer);
    Serial.println("Profile<");
}

