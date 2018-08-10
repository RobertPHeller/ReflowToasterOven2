// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 15:41:39 2018
//  Last Modified : <180810.0950>
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

#ifndef __PROFILE_H
#define __PROFILE_H

#include <Arduino.h>

#include "Settings.h"


typedef struct Profile
{
    double start_rate;
    double soak_temp1;
    double soak_temp2;
    uint16_t soak_length;
    double peak_temp;
    uint16_t time_to_peak;
    double cool_rate;
    void Default();
    bool Valid();
    void Load();
    void Save();
    Profile() {Default();}
    void DownloadFromSerial();
    void UploadToSerial();
} profile_t;



#endif // __PROFILE_H

