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
//  Last Modified : <180624.2003>
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
    uint8_t* ptr = (uint8_t*)this;
    size_t len = sizeof(settings_t);
    size_t l = filesystem.ReadFile("Settings.dat",ptr,len);
    // verify data validity
    if (l < len || !Valid())  {
        // data not valid, reset to defaults
        Default();
        Save();
    }
}

void Settings::Save() {
    // save the data block into EEPROM
    uint8_t* ptr = (uint8_t*)this;
    uint8_t len = sizeof(settings_t);
    size_t l = filesystem.WriteFile("Settings.dat",ptr,len);
}


