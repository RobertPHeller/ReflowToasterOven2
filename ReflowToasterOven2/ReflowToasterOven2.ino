// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sun Jun 24 09:38:54 2018
//  Last Modified : <180625.1526>
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
#include <Arduino.h>
#include <stdio.h>
//#include <EEPROM.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>
#include <Adafruit_ZeroTimer.h>
#include "reflowtoasteroven.h"
#include "temperaturemeasurement.h"
#include "heatingelement.h"
#include "Menu.h"
#include "FileIO.h"

static const char rcsid[] = "@(#) : $Id$";

settings_t settings; // store this globally so it's easy to access

FileIO filesystem;

void tmr_init();

Adafruit_ZeroTimer zt4 = Adafruit_ZeroTimer(4);


Menu menu = Menu();
TemperatureMeasurement sensor = TemperatureMeasurement();
Heat element = Heat();

void setup()
{
    Serial.begin(115200);
    Serial.println(PSTR("Initializing..."));
    
    sensor.init();
    element.init();
    tmr_init();
    menu.init();
    Serial.println(PSTR("Reflow Toaster Oven 1.0 Setup finished"));
    menu.main();
}

void loop() {} // Not used

volatile uint16_t tmr_ovf_cnt = 0;
volatile char tmr_checktemp_flag = 0;
volatile char tmr_drawlcd_flag = 0;
volatile char tmr_writelog_flag = 0;

void TIMER0_OVF_vect()
{
    element.isr();
    sensor.get_ADC_sample();
    
    tmr_ovf_cnt++;
    if (tmr_ovf_cnt % 1024 == 0) //  2s
    {
        tmr_ovf_cnt = 0;
        tmr_checktemp_flag = 1;
        tmr_drawlcd_flag = 1;
        tmr_writelog_flag = 1;
    }
    else if (tmr_ovf_cnt % 512 == 0) //  1s
    {
        tmr_checktemp_flag = 1;
        tmr_writelog_flag = 1;
    }
    else if (tmr_ovf_cnt % 256 == 0) //  0.5s
    {
        tmr_checktemp_flag = 1;
    }
}

void tmr_init()
{
    /********************* Timer #4, 8 bit, one callback with adjustable period */
    zt4.configure(TC_CLOCK_PRESCALER_DIV64, // prescaler: 48Mhz/64 = 750Khz
                  TC_COUNTER_SIZE_16BIT,   // bit width of timer/counter
                  TC_WAVE_GENERATION_MATCH_FREQ  // match style
                  );
    
    zt4.setPeriodMatch(1464, 1, 0); // 1 match, channel 0 -- 1.953125ms (512Hz == 750Khz/1464)
    zt4.setCallback(true, TC_CALLBACK_CC_CHANNEL0, TIMER0_OVF_vect); 
    zt4.enable(true);
}



