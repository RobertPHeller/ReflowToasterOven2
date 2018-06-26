// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 13:22:44 2018
//  Last Modified : <180624.1900>
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

#ifndef __HEATINGELEMENT_H
#define __HEATINGELEMENT_H

#define RELAYCTRL 13 // Relay for heating elements (DigitalWrite())

class Heat {
private:
    volatile uint16_t pwm_ocr;
    volatile uint16_t pwm_ocr_temp;
    volatile uint16_t isr_cnt;
public:
    Heat() {
        pwm_ocr = 0;
        pwm_ocr_temp = 0;
        isr_cnt = 0;
    }
    void init();
    void isr();
    void set(uint16_t ocr);
};

extern Heat element;

#endif // __HEATINGELEMENT_H

