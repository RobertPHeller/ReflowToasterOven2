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
//  Last Modified : <180624.2006>
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
    uint8_t* ptr = (uint8_t*)this;
    uint8_t len = sizeof(profile_t);
    size_t l = filesystem.ReadFile("Profile.dat",ptr,len);
    if (l < len || !Valid()) {
        // data not valid, reset to defaults
        Default();
        Save();
    }
}

void Profile::Save() {
    // save the data block into EEPROM
    uint8_t* ptr = (uint8_t*)this;
    uint8_t len = sizeof(profile_t);
    size_t l = filesystem.WriteFile("profile.dat",ptr,len);
}

