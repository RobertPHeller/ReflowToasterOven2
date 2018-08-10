// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 15:41:16 2018
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
#include "Settings.h"
#include <stdlib.h>
#include "dtostrg.h"

static const char rcsid[] = "@(#) : $Id$";

void Settings::Default() {
    pid_p = 6000.0;
    pid_i = 20.00;
    pid_d = -0.00;
    max_temp = 225.0;
    time_to_max = 300.0;
}

bool Settings::Valid() {
    return (max_temp > 0.0 && time_to_max > 0.0);
}

void Settings::Load() {
    if (!filesystem.begin()) return;
    // load the data block from EEPROM
    char *filebuffer,*p,*p2;
    File fp = fatfs.open("Settings.dat",FILE_READ);
    if (fp) {
        size_t len = fp.size();
        filebuffer = (char *)calloc(sizeof(char),len);
        fp.read(filebuffer,len);
        fp.close();
        p = filebuffer;
        pid_p = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        pid_i = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        pid_d = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        max_temp = strtod(p,&p2);
        p = p2;
        while (*p && *p != '\n') p++;
        time_to_max = strtod(p,&p2);
        p = p2;
        free(filebuffer);
        // verify data validity
        if (!Valid())  {
            // data not valid, reset to defaults
            Default();
            Save();
        }
    }
}

void Settings::Save() {
    //Serial.println("*** Settings::Save() entered");
    if (!filesystem.begin()) return;
    //Serial.println("*** Settings::Save(): after filesystem.begin()");
    // save the data block into EEPROM
    File fp = fatfs.open("Settings.dat",FILE_READ);
    if (fp) {
        fp.close();
        filesystem.DeleteFile("Settings.dat");
    }
    fp = fatfs.open("Settings.dat",FILE_WRITE);
    //Serial.println("*** Settings::Save(): file opened");
    if (fp) {
        static char buffer[16];
        dtostrg(pid_p,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(pid_i,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(pid_d,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(max_temp,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        dtostrg(time_to_max,10,5,buffer);
        fp.write(buffer,strlen(buffer));fp.write("\n",1);
        fp.close();
    }
}


void Settings::DownloadFromSerial()
{
    char LineBuffer[64],*p2;
    int line = 0;
    Serial.println("Settings?");
    while (1) {
        delay(100);
        int bytes = Serial.readBytesUntil('\n',LineBuffer,sizeof(LineBuffer));
        //Serial.print("*** Settings::DownloadFromSerial(): bytes = ");
        //Serial.println(bytes);
        if (bytes == 0) {
            if (line < 5) continue;
            else break;
        }
        line++;
        //Serial.print("*** Settings::DownloadFromSerial(): line = ");
        //Serial.println(line);
        switch (line) {
        case 1:
            pid_p = strtod(LineBuffer,&p2);
            break;
        case 2:
            pid_i = strtod(LineBuffer,&p2);
            break;
        case 3:
            pid_d = strtod(LineBuffer,&p2);
            break;
        case 4:
            max_temp = strtod(LineBuffer,&p2);
            break;
        case 5:
            time_to_max = strtod(LineBuffer,&p2);
            break;
        default:
            break;
        }
        Serial.println(line);
    }
    if (!Valid())  {
        Default();
        Save();
    }
}

void Settings::UploadToSerial()
{
    static char buffer[16];
    Serial.println("Settings>");
    dtostrg(pid_p,10,5,buffer);
    Serial.println(buffer);
    dtostrg(pid_i,10,5,buffer);
    Serial.println(buffer);
    dtostrg(pid_d,10,5,buffer);
    Serial.println(buffer);
    dtostrg(max_temp,10,5,buffer);
    Serial.println(buffer);
    dtostrg(time_to_max,10,5,buffer);
    Serial.println(buffer);
    Serial.println("Settings<");
}
