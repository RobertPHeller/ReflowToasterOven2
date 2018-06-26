// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 13:02:49 2018
//  Last Modified : <180326.1823>
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

#ifndef __TEMPERATUREMEASUREMENT_H
#define __TEMPERATUREMEASUREMENT_H

#define THERMOCOUPLE_CONSTANT 0.32 // this is derived from the AD595AQ datasheet
#define ROOM_TEMP 20.0
#define TEMP_MEASURE_CHAN A0 // AnalogRead() the ADC pin connected to the AD595AQ
#define ADC_SAMPLE_SIZE 128
#define ADC_AVERAGE_SIZE (ADC_SAMPLE_SIZE/4)

class TemperatureMeasurement {
private:
    volatile uint16_t adc_samples[ADC_SAMPLE_SIZE];
    volatile uint8_t adc_sample_idx;
    uint16_t temp_last_read;
public:
    TemperatureMeasurement() {
        temp_last_read = 0;
    }
    void init();
    uint16_t read();
    uint16_t temperature_to_sensor(double temp) const;
    void filter_reset();
    void get_ADC_sample ();
};


#define sensor_to_temperature(x) ((x)*THERMOCOUPLE_CONSTANT)

extern TemperatureMeasurement sensor;

#endif // __TEMPERATUREMEASUREMENT_H

