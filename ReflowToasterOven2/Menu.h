// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 14:45:39 2018
//  Last Modified : <180626.1243>
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

#ifndef __MENU_H
#define __MENU_H

#include <Arduino.h>
#include <stdio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "reflowtoasteroven.h"
#include "heatingelement.h"
#include "temperaturemeasurement.h"
#include "buttons.h"
#include <string.h>
#include <stdlib.h>




#define BUZZER 12    // Buzzer -- passed to Tone()
#define BACKLIGHT 5 // PWM / AnalogWrite() backlight brightness
#define TFT_DC 10 //  
#define TFT_RST 11 // 
#define TFTCS 6 //   |
//#define MOSI 11 //    } TFT Display connections (MISO not used)
//#define MISO 12 //    |
//#define SCK 13 //    /

#define LCD_WIDTH ST7735_TFTHEIGHT_160
#define LCD_HEIGHT ST7735_TFTWIDTH_128
#define FONT_WIDTH 6
#define FONT_HEIGHT 8
#define LCD_ROWS (LCD_HEIGHT / FONT_HEIGHT)

class Menu : public Adafruit_ST7735 {
private:
    // Graphics colors
    uint16_t bg, graphcolor, txtfg, txtbg;
    // this string is allocated for temporary use
    char strbuf[(LCD_WIDTH/FONT_WIDTH) + 2];
    // this counts the number of loop iterations that a button has been held for
    uint8_t button_held;
    // store the temperature history for graphic purposes
    uint8_t temp_history[LCD_WIDTH];
    uint16_t temp_history_idx;
    //uint8_t temp_plan[LCD_WIDTH]; // also store the target temperature for comparison purposes
    
    char graph_text[((LCD_WIDTH/FONT_WIDTH) + 2) * 5];
    
    // changes a value based on the up and down buttons
    double button_change_double(double oldvalue, double increment, double limit1, double limit2);
    // same as above but for integers
    int32_t button_change_int(int32_t oldvalue, int32_t increment, int32_t limit1, int32_t limit2);
    void menu_manual_pwm_ctrl();
    void menu_manual_temp_ctrl();
    void menu_edit_profile(profile_t* profile);
    void menu_auto_mode();
    void menu_edit_settings();
    void menu_flash_filesystem();
#define printat(r,c,t) (setCursor((c)*FONT_WIDTH,(r)*FONT_HEIGHT), print(t))
#define printlnat(r,c,t) (setCursor((c)*FONT_WIDTH,(r)*FONT_HEIGHT), println(t))
    
    inline char* str_from_int(signed long value)
    {
	return ltoa(value, strbuf, 10);
    }
    char* str_from_double(double value, int decimalplaces);
    void auto_go(profile_t* profile);
    void draw_graph(void);
    double approx_pwm(double target);
    uint16_t pid(double target, double current, double * integral, double * last_error);
    int current_row;
    static void file_list_callback(File file,void *userdata) {
        Menu *m = (Menu *)userdata;
        m->file_list(file);
    }
    void file_list(File file);
public:
    Menu(uint16_t _bg = ST7735_BLACK,uint16_t _txtfg = ST7735_WHITE,
         uint16_t _graphcolor = ST7735_WHITE) : 
    Adafruit_ST7735(TFTCS, TFT_DC, TFT_RST) {
        button_held = 0;
        bg = _bg;
        txtfg = _txtfg;
        txtbg = _bg;
        graphcolor = _graphcolor;
    }
    void init() {
        buttons_init();
        initR(INITR_BLACKTAB);
        fillScreen(bg);
        setTextColor(txtfg,txtbg);
        setTextWrap(false);
        setTextSize(1);
        setRotation(1); // Rotate screen 90 degrees (?)
        pinMode(BUZZER,OUTPUT);
        pinMode(BACKLIGHT,OUTPUT);
        analogWrite(BACKLIGHT,128);/* Medium brightness */
    }
    void main();
    
};


#endif // __MENU_H

