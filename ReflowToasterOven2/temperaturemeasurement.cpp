// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 13:03:08 2018
//  Last Modified : <180624.1857>
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
#include "reflowtoasteroven.h"
#include "temperaturemeasurement.h"

static const char rcsid[] PROGMEM = "@(#) : $Id$";


/*
 * This file contains code that is specific for measuring the temperature using the AD595AQ
 * I discovered that the AD595AQ sometimes outputs a square wave instead of a steady voltage
 * 
 * I'm not sure if this is intentional and a part of the AD595AQ's design, or if it has something to do with my own circuit
 *
 * Whatever the reason is, this code is written to take the peak readings of the square wave, while ignoring the troughs
 *
 * 
*/

uint16_t TemperatureMeasurement::read()
{
	int i, j;
	uint16_t working_sample;
	uint16_t minimum = 0x7FFF;
	uint16_t maximum = 0x0000;
	uint16_t middle;
	uint32_t sum = 0;
	uint8_t sum_cnt = 0;
	
	// determine the crest and troughs of the wave
	for (i = 0; i < ADC_SAMPLE_SIZE; i++)
	{
		working_sample = adc_samples[i];
		
		if (working_sample > maximum)
		{
			maximum = working_sample;
		}
		
		if (working_sample < minimum)
		{
			minimum = working_sample;
		}
	}
	
	// calculate the value in the middle of the wave
	middle = ((maximum - minimum) / 2) + minimum;
	// now we can take the peak values since we know the middle value
	
	// average recent peak values while ignoring troughs
	for (i = 0, j = adc_sample_idx; sum_cnt < ADC_AVERAGE_SIZE; i++)
	{
		working_sample = adc_samples[j];
		if (working_sample >= middle)
		{
			sum += working_sample;
			sum_cnt++;
		}
		
		j = (j + ADC_SAMPLE_SIZE - 1) % ADC_SAMPLE_SIZE;
	}
	
	uint16_t result = (uint16_t)lround((double)sum / (double)sum_cnt);
	if (result < temp_last_read - 5 && temp_last_read > 5)
	{
		result = temp_last_read - 5;
	}
	//else
	{
		temp_last_read = result;
	}
	
	return result;
}

void TemperatureMeasurement::filter_reset()
{
    temp_last_read = 0;
}

// conversion functions
uint16_t TemperatureMeasurement::temperature_to_sensor(double temp) const
{
	return (uint16_t)lround(temp / THERMOCOUPLE_CONSTANT);
}

// new sample has arrived
void TemperatureMeasurement::get_ADC_sample ()
{
    // read in sample
    adc_samples[adc_sample_idx] = analogRead(TEMP_MEASURE_CHAN);
    adc_sample_idx = (adc_sample_idx + 1) % ADC_SAMPLE_SIZE;
	
}

void TemperatureMeasurement::init()
{
    adc_sample_idx = 0;
}

