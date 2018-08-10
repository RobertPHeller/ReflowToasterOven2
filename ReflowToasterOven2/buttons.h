// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 13:23:58 2018
//  Last Modified : <180722.1952>
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

#ifndef __BUTTONS_H
#define __BUTTONS_H
#define button_debounce_us(x) delayMicroseconds(x)
#define button_debounce_ms(x) delay(x)
#define button_debounce() delayMicroseconds(100)

#define SWITCH1 A3 /* |     */ 
#define SWITCH2 A2 /*  } Push buttons (DigitalRead()) */
#define SWITCH3 A1 /* |     */

#define buttons_init() do{ \
pinMode(SWITCH1, INPUT_PULLUP); \
pinMode(SWITCH2, INPUT_PULLUP); \
pinMode(SWITCH3, INPUT_PULLUP); \
}while(0);

#define button_up() (digitalRead(SWITCH3) == 0)
#define button_mid() (digitalRead(SWITCH2) == 0)
#define button_down() (digitalRead(SWITCH1) == 0)

#endif // __BUTTONS_H

