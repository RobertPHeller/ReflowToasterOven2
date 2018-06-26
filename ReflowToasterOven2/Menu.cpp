// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sat Mar 24 14:46:53 2018
//  Last Modified : <180624.1930>
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
#include "Menu.h"
#include "pitches.h"
static const char rcsid[]  = "@(#) : $Id$";



// this estimates the PWM duty cycle needed to reach a certain steady temperature
// if the toaster is capable of a maximum of 300 degrees, then 100% duty cycle is used if the target temperature is 300 degrees, and 0% duty cycle is used if the target temperature is room temperature.
double Menu::approx_pwm(double target)
{
    return 65535.0 * ((target * THERMOCOUPLE_CONSTANT) / settings.max_temp);
}

uint16_t Menu::pid(double target, double current, double * integral, double * last_error)
{
    double error = target - current;
    if (target == 0)
    {
        // turn off if target temperature is 0
        
        (*integral) = 0;
        (*last_error) = error;
        return 0;
    }
    else
    {
        if (target < 0)
        {
            target = 0;
        }
        
        // calculate PID terms
        
        double p_term = settings.pid_p * error;		
        double new_integral = (*integral) + error;
        double d_term = ((*last_error) - error) * settings.pid_d;
        (*last_error) = error;
        double i_term = new_integral * settings.pid_i;
        
        double result = approx_pwm(target) + p_term + i_term + d_term;
        
        // limit the integral so it doesn't get out of control
        if ((result >= 65535.0 && new_integral < (*integral)) || (result < 0.0 && new_integral > (*integral)) || (result <= 65535.0 && result >= 0))
        {
            (*integral) = new_integral;
        }
        
        // limit the range and return the rounded result for use as the PWM OCR value
        return (uint16_t)lround(result > 65535.0 ? 65535.0 : (result < 0.0 ? 0.0 : result));
    }
}

// changes a value based on the up and down buttons
double Menu::button_change_double(double oldvalue, double increment, double limit1, double limit2)
{
	double maxlimit = limit1 >= limit2 ? limit1 : limit2;
	double minlimit = limit1 < limit2 ? limit1 : limit2;
	
	if (button_up())
	{
		button_debounce();

		// the amount of change occurs increases as the button is held down for longer
		button_held = (button_held < 200) ? (button_held + 1) : button_held;
		increment += increment*(button_held/3);

		// return the changed value
		return ((oldvalue + increment) > maxlimit) ? maxlimit : (oldvalue + increment);
		// the range of this value is also limited
	}
	else if (button_down())
	{
		button_debounce();

		// the amount of change occurs increases as the button is held down for longer
		button_held = (button_held < 200) ? (button_held + 1) : button_held;
		increment += increment*(button_held/3);

		// return the changed value
		return ((oldvalue - increment) < minlimit) ? minlimit : (oldvalue - increment);
		// the range of this value is also limited
	}
	else
	{
		// no button pressed, meaning button is not held down
		button_held = 0;
		// and the value does not change, but keep it in range
		return (oldvalue > maxlimit) ? maxlimit : ((oldvalue < minlimit) ? minlimit : oldvalue);
	}
}

// same as above but for integers
int32_t Menu::button_change_int(int32_t oldvalue, int32_t increment, int32_t limit1, int32_t limit2)
{
	int32_t maxlimit = limit1 >= limit2 ? limit1 : limit2;
	int32_t minlimit = limit1 < limit2 ? limit1 : limit2;
	
	if (button_up())
	{
		button_debounce();

		// the amount of change occurs increases as the button is held down for longer
		button_held = (button_held < 100) ? (button_held + 1) : button_held;
		increment += increment*(button_held/3);

		// return the changed value
		return ((oldvalue + increment) > maxlimit) ? maxlimit : (oldvalue + increment);
		// the range of this value is also limited
	}
	else if (button_down())
	{
		button_debounce();

		// the amount of change occurs increases as the button is held down for longer
		button_held = (button_held < 100) ? (button_held + 1) : button_held;
		increment += increment*(button_held/3);

		// return the changed value
		return ((oldvalue - increment) < minlimit) ? minlimit : (oldvalue - increment);
		// the range of this value is also limited
	}
	else
	{
		// no button pressed, meaning button is not held down
		button_held = 0;
		// and the value does not change, but keep it in range
		return (oldvalue > maxlimit) ? maxlimit : ((oldvalue < minlimit) ? minlimit : oldvalue);
	}
}

char* Menu::str_from_double(double value, int decimalplaces)
{
    
    snprintf(strbuf,sizeof(strbuf),"%f.*",decimalplaces,value);
    return strbuf;
#if 0
	char* result = dtostrf(value, -(LCD_WIDTH/FONT_WIDTH), decimalplaces, strbuf);
	
	// trim trailing spaces
	int8_t len = strlen(result);
	len--;
	while (result[len] == ' ' && len >= 0)
	{
		result[len] = 0;
		len--;
	}
        return result;
#endif
}

void Menu::menu_manual_pwm_ctrl()
{
    sensor.filter_reset();
    
    Serial.println("manual PWM control mode,");
    uint16_t iteration = 0;

    uint16_t cur_pwm = 0;
    uint16_t cur_sensor = sensor.read();
    element.set(cur_pwm);
    while(1)
    {
        element.set(cur_pwm);
        cur_sensor = sensor.read();
        
        // draw the LCD
        for (int r = 0; r < LCD_ROWS; r++)
        {
            //lcd_set_row_column(r, 0);
            
            switch (r)
            {
            case 0:
                // screen title
                printlnat(r,0,"Manual PWM Control");
                break;
            case 1:
                // this creates a horizontal divider line
                for (int c = 0; c < LCD_WIDTH; c++)
                {
                    printat(r,c,'\176');
                }
                break;
                
                // print info/submenu items
            case 2:
                printat(r,0,"PWM= ");
                print(str_from_int(cur_pwm));
                println(" / 65535");
                break;
            case 3:
                printat(r,0,"Sensor: ");
                print(str_from_int(cur_sensor));
                println(" / 1023");
                break;
            case 4:
                printat(r,0,"Temp: ");
                print(str_from_int(lround(cur_sensor * THERMOCOUPLE_CONSTANT)));
                println(" `C");
                break;
            default:
                /*lcd_clear_restofrow();*/
                break;
            }
        }
        
        // change this value according to which button is pressed
        cur_pwm = button_change_int(cur_pwm, 1024, 0, 65535);
        
        if (button_mid())
        {
            button_held = 0;
            button_debounce();
            while (button_mid());
            button_debounce();
            
            // exit this mode, back to home menu
            return;
        }
        
        if (tmr_writelog_flag)
        {
            tmr_writelog_flag = 0;
            
            iteration++; // used so CSV entries can be sorted by time
            
            // print log in CSV format
            Serial.print(str_from_double(iteration * TMR_OVF_TIMESPAN * 512, 1));
            Serial.print(", ");
            Serial.print(str_from_int(cur_sensor));
            Serial.print(", ");
            Serial.print(str_from_int(cur_pwm));
            Serial.println(",");
        }
    }
}

void Menu::menu_manual_temp_ctrl()
{
    sensor.filter_reset();
    
    settings.Load(); // load from eeprom
	
    // signal start of mode in log
    Serial.println("manual temperature control mode,");

    uint16_t iteration = 0;
    uint16_t tgt_temp = 0;
    uint16_t cur_pwm = 0;
    double integral = 0.0, last_error = 0.0;
    uint16_t tgt_sensor = sensor.temperature_to_sensor((double)tgt_temp);
    uint16_t cur_sensor = sensor.read();
	
    while(1)
    {
        if (tmr_drawlcd_flag || (tmr_checktemp_flag == 0 && tmr_writelog_flag == 0))
        {
            tmr_drawlcd_flag = 0;
            // draw the LCD
            for (int r = 0; r < LCD_ROWS; r++)
            {
                /*lcd_set_row_column(r, 0);*/
                
                switch (r)
                {
                case 0:
                    // screen title
                    printlnat(r,0,"Manual Temp Control");
                    break;
                case 1:
                    // this creates a horizontal divider line
                    for (int c = 0; c < LCD_WIDTH; c++)
                    {
                        printat(r,c,'\176');
                    }
                    break;
                    
                    // print info/submenu items
                case 2:
                    printat(r,0,"Target= ");
                    print(str_from_int(tgt_temp));
                    println(" `C");
                    break;
                case 3:
                    printat(r,0,"Current: ");
                    print(str_from_int(lround(sensor_to_temperature(cur_sensor))));
                    println(" `C");
                    break;
                case 4:
                    printat(r,0,"Sensor: ");
                    print(str_from_int(cur_sensor));
                    println(" / 1023");
                    break;
                case 5:
                    printat(r,0,"PWM: ");
                    print(str_from_int(cur_pwm));
                    println(" / 65535");
                    break;
                default:
                    /*lcd_clear_restofrow();*/
                    break;
                }
            }
            
            // change this value according to which button is pressed
            tgt_temp = button_change_int(tgt_temp, 10, 0, 350);
        }
        
        // reset these so the PID controller starts fresh when settings change
        if (button_up() || button_down())
        {
            integral = 0;
            last_error = 0;
        }
        
        if (button_mid())
        {
            button_held = 0;
            
            button_debounce();
            while (button_mid());
            button_debounce();
            
            // exit this mode, back to home menu
            return;
        }
        
        if (tmr_checktemp_flag)
        {
            tmr_checktemp_flag = 0;
            
            tgt_sensor = sensor.temperature_to_sensor((double)tgt_temp);
            cur_sensor = sensor.read();
            cur_pwm = pid((double)sensor.temperature_to_sensor((double)tgt_temp), (double)cur_sensor, &integral, &last_error);
        }
        
        if (tmr_writelog_flag)
        {
            tmr_writelog_flag = 0;
            
            iteration++; // used so CSV entries can be sorted by time
            
            // print log in CSV format
            Serial.print(str_from_double(iteration * TMR_OVF_TIMESPAN * 512, 1)); Serial.print(", ");
            Serial.print(str_from_int(cur_sensor)); Serial.print(", ");
            Serial.print(str_from_int(tgt_temp)); Serial.print(", ");
            Serial.print(str_from_int(cur_pwm)); Serial.println(",");
        }
    }
}

void Menu::menu_edit_profile(profile_t* profile)
{
    char selection = 0;

    while(1)
    {
        element.set(0); // keep off for safety
        
        // draw on LCD
        for (int r = 0; r < LCD_ROWS; r++)
        {
            /*lcd_set_row_column(r, 0);*/
            
            // draw a indicator beside the selected menu item
            if ((r - 2) == selection)
            {
                printat(r,0,'>');
            }
            else
            {
                if (r != 1)
                {
                    printat(r,0,' ');
                }
            }
            
            switch (r)
            {
            case 0:
                // menu title
                printlnat(r,1,"Edit Profile");
                break;
            case 1:
                // this draws a horizontal divider line across the screen
                for (int c = 0; c < LCD_WIDTH; c++)
                {
                    printat(r,c,'\176');
                }
                break;
                
                // display info/submenu items
            case 2:
                printat(r,1,"Start Rate= ");
                print(str_from_double(profile->start_rate, 1));
                println(" `C/s");
                break;
            case 3:
                printat(r,1,"Soak Temp 1= ");
                print((uint16_t)lround(profile->soak_temp1));
                println(" `C");
                break;
            case 4:
                printat(r,1,"Soak Temp 2= ");
                print((uint16_t)lround(profile->soak_temp2));
                println(" `C");
                break;
            case 5:
                printat(r,1,"Soak Length= ");
                print(profile->soak_length);
                println(" s");
                break;
            case 6:
                printat(r,1,"Peak Temp= ");
                print((uint16_t)lround(profile->peak_temp));
                println(" `C");
                break;
            case 7:			
                printat(r,1,"Time to Peak= ");
                print(profile->time_to_peak);
                println(" s");
                break;
            case 8:
                printat(r,1,"Cool Rate= ");
                print(str_from_double(profile->cool_rate, 1));
                println(" `C/s");
                break;
            case 9:
                printlnat(r,1,"Return to Auto Mode");
                break;
            default:
                /*lcd_clear_restofrow();*/
                break;
            }
        }
	
        // change values according to the selected item and buttons being pressed
        switch (selection)
        {
        case 0:
            profile->start_rate = button_change_double(profile->start_rate, 0.1, 0.1, 5.0);
            break;
        case 1:
            profile->soak_temp1 = button_change_double(profile->soak_temp1, 1, 50, 300);
            break;
        case 2:
            profile->soak_temp2 = button_change_double(profile->soak_temp2, 1, 50, 300);
            break;
        case 3:
            profile->soak_length = button_change_int(profile->soak_length, 0.1, 60, 60*5);
            break;
        case 4:
            profile->peak_temp = button_change_double(profile->peak_temp, 1, 150, 350);
            break;
        case 5:
            profile->time_to_peak = button_change_int(profile->time_to_peak, 1, 0, 60*5);
            break;
        case 6:
            profile->cool_rate = button_change_double(profile->cool_rate, 0.1, 0.1, 5.0);
            break;
        default:
            // there's no need for button holding if it's in a non-value-changing menu item
            button_held = 0;
            break;
        }
        
        if (selection == 7) // the "return" option
        {
            if (button_up())
            {
                button_debounce();
                while (button_up());
                button_debounce();
                
                if (profile->Valid())
                {
                    return;
                }
                else
                {
                    fillScreen(bg);
                    /*lcd_clear_screen();*/
                    /*lcd_set_row_column(0, 0);*/
                    printlnat(0,0,"Error in Profile");
                    tone(BUZZER,NOTE_GS3,3000);
                    delay(1000);
                }
            }
        }
        
        if (button_mid())
        {
            button_held = 0;
            
            button_debounce();
            while (button_mid());
            button_debounce();
            
            // change selected menu item to the next one
            selection = (selection + 1) % 8;
        }
    }
}

void Menu::menu_auto_mode()
{
    static profile_t profile;
    profile.Load(); // load from eeprom

    char selection = 0;

    while(1)
    {
        element.set(0); // keep off for safety

        // draw on LCD
        for (int r = 0; r < LCD_ROWS; r++)
        {
            /*lcd_set_row_column(r, 0);*/
            
            // draw a indicator beside the selected menu item
            if ((r - 2) == selection)
            {
                printat(r,0,'>');
            }
            else
            {
                if (r != 1)
                {
                    printat(r,0,' ');
                }
            }
            
            switch (r)
            {
            case 0:
                // menu title
                printlnat(r,1,"Auto Mode");
                break;
            case 1:
                // this draws a horizontal divider line across the screen
                for (int c = 0; c < LCD_WIDTH; c++)
                {
                    printat(r,c,'\176');
                }
                break;
                
                // display info/submenu items
            case 2:
                printlnat(r,1,"Edit Profile");
                break;
            case 3:
                printlnat(r,1,"Reset to Defaults");
                break;
            case 4:
                printlnat(r,1,"Start");
                break;
            case 5:
                printlnat(r,1,"Back to Home Menu");
                break;
            default:
                break;
            }
        }
        
        if (selection == 0) // the "edit profile" menu item
        {
            if (button_up())
            {
                button_debounce();
                while (button_up());
                button_debounce();
                
                menu_edit_profile(&profile);
                profile.Save(); // save to eeprom
            }
        }
        else if (selection == 1) // the "reset to default" menu item
        {
            if (button_up())
            {
                button_debounce();
                while (button_up());
                button_debounce();
                
                profile.Default();
                profile.Save(); // save to eeprom
                
                fillScreen(bg);
                printlnat(0,0,"Reset to Defaults... Done");
                tone(BUZZER,NOTE_C5,3000);
                delay(1000);
            }
        }
        else if (selection == 2) // the "start" menu item
        {
            if (button_up())
            {
                button_debounce();
                while (button_up());
                button_debounce();
                
                fillScreen(bg);
                
                auto_go(&profile);
                tone(BUZZER,NOTE_A4,3000);
                // go back to home menu when finished
                return;
            }
        }
        else if (selection == 3) // the "back to home menu" option
        {
            if (button_up())
            {
                button_debounce();
                while (button_up());
                button_debounce();
                
                // go back to home menu
                return;
            }
        }
        
        if (button_mid())
        {
            button_debounce();
            while (button_mid());
            button_debounce();
            
            // change selected menu item to the next one
            selection = (selection + 1) % 4;
        }
    }
}

void Menu::menu_edit_settings()
{
    settings.Load(); // load from eeprom
    
    char selection = 0;
    while(1)
    {
        element.set(0); // keep off for safety
        
        // draw LCD
        for (int r = 0; r < LCD_ROWS; r++)
        {
            /*lcd_set_row_column(r, 0);*/
            
            // draw indicator beside the selected menu item
            if (r - 2 == selection)
            {
                printat(r,0,'>');
            }
            else
            {
                if (r != 1)
                {
                    printat(r,0,' ');
                }
            }
            
            switch (r)
            {
            case 0:
                // menu title
                printlnat(r,1,"Edit Settings");
                break;
            case 1:
                // draw a horizontal divider line across the screen
                for (int c = 0; c < LCD_WIDTH; c++)
                {
                    printat(r,c,'\176');
                }
                break;
                
                // display info/submenu items
            case 2:
                printat(r,1,"PID P= ");
                println(str_from_double(settings.pid_p, 2));
                break;
            case 3:
                printat(r,1,"PID I= ");
                println(str_from_double(settings.pid_i, 2));
                break;
            case 4:
                printat(r,1,"PID D= ");
                println(str_from_double(settings.pid_d, 2));
                break;
            case 5:
                printat(r,1,"Max Temp= ");
                print((uint16_t)lround(settings.max_temp));
                println(" `C");
                break;
            case 6:
                printat(r,1,"Time to Max= ");
                print((uint16_t)lround(settings.time_to_max));
                println(" s");
                break;
            case 7:
                printlnat(r,1,"Reset to Defaults");
                break;
            case 8:
                printlnat(r,1,"Back to Home Menu");
            default:
                break;
            }
        }
        
        // change value according to which value is selected and which button is pressed
        switch(selection)
        {
        case 0:
            settings.pid_p = button_change_double(settings.pid_p, 0.1, 0.0, 10000.0);
            break;
        case 1:
            settings.pid_i = button_change_double(settings.pid_i, 0.01, 0.0, 10000.0);
            break;
        case 2:
            settings.pid_d = button_change_double(settings.pid_d, 0.01, -10000.0, 10000.0);
            break;
        case 3:
            settings.max_temp = button_change_double(settings.max_temp, 1.0, 200.0, 350.0);
            break;
        case 4:
            settings.time_to_max = button_change_double(settings.time_to_max, 1.0, 0.0, 60*20);
            break;
        default:
            // there's no need for button holding if it's in a non-value-changing menu item
            button_held = 0;
            break;
        }
        
        if (selection == 5) // the "reset to default" menu item
        {
            if (button_up())
            {
                button_debounce();
                while (button_up());
                button_debounce();
                
                settings.Default();
            }
        }
        else if (selection == 6) // the "back to home menu" option
        {
            if (button_up())
            {
                button_debounce();
                while (button_up());
                button_debounce();
                
                if (settings.Valid())
                {
                    settings.Save(); // save to eeprom
                    
                    // back to main menu
                    return;
                }
                else
                {
                    fillScreen(bg);
                    printlnat(0,0,"Error in Settings");
                    tone(BUZZER,NOTE_GS3,3000);
                    delay(1000);
                }
            }
        }
        
        if (button_mid())
        {
            button_held = 0;
            
            button_debounce();
            while (button_mid());
            button_debounce();
            
            // change selected menu item to the next one
            selection = (selection + 1) % 7;
        }
    }
}

void Menu::main()
{
    char selection = 0;
    char screen_dirty = 1;
    
    while(1)
    {
        element.set(0); // turn off for safety
        
        if (screen_dirty != 0) // only draw if required
        {
            for (int r = 0; r < LCD_ROWS; r++)
            {
                /*lcd_set_row_column(r, 0);*/
                
                // draw a indicator beside the selected menu item
                if (r - 2 == selection)
                {
                    printat(r,0,'>');
                }
                else
                {
                    if (r != 1)
                    {
                        printat(r,0,' ');
                    }
                }
                
                switch (r)
                {
                case 0:
                    // menu title
                    printlnat(r,1,"Home Menu");
                    break;
                case 1:
                    // this draws a horizontal divider line across the screen
                    for (int c = 0; c < LCD_WIDTH; c++)
                    {
                        printat(r,c,'\176');
                    }
                    break;
                    
                    // display submenu items
                case 2:
                    printlnat(r,1,"Auto Mode");
                    break;
                case 3:
                    printlnat(r,1,"Manual Temp Control");
                    break;
                case 4:
                    printlnat(r,1,"Manual PWM Control");
                    break;
                case 5:
                    printlnat(r,1,"Edit Settings");
                    break;
                default:
                    /*lcd_clear_restofrow();*/
                    break;
                }
                
                screen_dirty = 0; // the screen is fresh
            }
        }
        
        if (button_up())
        {
            button_debounce();
            while (button_up());
            button_debounce();
            
            // enter the submenu that is selected
            
            if (selection == 0)
            {
                menu_auto_mode();
            }
            else if (selection == 1)
            {
                menu_manual_temp_ctrl();
            }
            else if (selection == 2)
            {
                menu_manual_pwm_ctrl();
            }
            else if (selection == 3)
            {
                menu_edit_settings();
            }
            
            screen_dirty = 1;
        }
        else if (button_mid())
        {
            button_debounce();
            while (button_mid());
            button_debounce();
            
            selection = (selection + 1) % 4; // change selected menu item
            
            screen_dirty = 1;
        }
    }
}


void Menu::auto_go(profile_t* profile)
{
    sensor.filter_reset();
    
    settings.Load(); // load from eeprom
    
    // validate the profile before continuing
    if (!profile->Valid())
    {
        /*lcd_set_row_column(0, 0);*/
        printlnat(0,0,"Error in Profile");
        tone(BUZZER,NOTE_GS3,3000);
        delay(1000);
        return;
    }
    
    Serial.println("auto mode session start,");
    
    // this will be used for many things later
    double max_heat_rate = settings.max_temp / settings.time_to_max;
    
    // reset the graph
    for (int i = 0; i < LCD_WIDTH; i++)
    {
        temp_history[i] = 0;
        //temp_plan[i] = 0;
    }
    temp_history_idx = 0;
    
    // total duration is calculated so we know how big the graph needs to span
    // note, this calculation is only an worst case estimate
    // it is also aware of whether or not the heating rate can be achieved
    double total_duration = (double)(profile->soak_length + profile->time_to_peak) +
          ((profile->soak_temp1 - ROOM_TEMP) / min(profile->start_rate, max_heat_rate)) + 
          ((profile->peak_temp - ROOM_TEMP) / profile->cool_rate) +
          10.0; // some extra just in case
    double graph_tick = total_duration / LCD_WIDTH;
    double graph_timer = 0.0;
    
    // some more variable initialization
    double integral = 0.0, last_error = 0.0;
    char stage = 0; // the state machine state
    uint32_t total_cnt = 0; // counter for the entire process
    uint16_t length_cnt = 0; // counter for a particular stage
    char update_graph = 0; // if there is new stuff to draw
    uint16_t pwm_ocr = 0; // temporary holder for PWM duty cycle
    double tgt_temp = sensor_to_temperature(sensor.read());
    double start_temp = tgt_temp;
    uint16_t cur_sensor = sensor.read();
    while (1)
    {	
        if (tmr_checktemp_flag)
        {
            tmr_checktemp_flag = 0;
            
            total_cnt++;
            
            cur_sensor = sensor.read();
            
            if (DEMO_MODE)
            {
                // in demo mode, we fake the reading
                cur_sensor = sensor.temperature_to_sensor(tgt_temp);
            }
            
            if (stage == 0) // preheat to thermal soak temperature
            {
                length_cnt++;
                if (sensor_to_temperature(cur_sensor) >= profile->soak_temp1)
                {
                    // reached soak temperature
                    stage++;
                    integral = 0.0;
                    last_error = 0.0;
                    length_cnt = 0;
                }
                else
                {
                    // calculate next temperature by increasing current temperature
                    tgt_temp = max(ROOM_TEMP, start_temp) + (profile->start_rate * TMR_OVF_TIMESPAN * 256 * length_cnt);
                    
                    if (length_cnt % 8 == 0)
                    {
                        start_temp = sensor_to_temperature(cur_sensor);
                        length_cnt = 0;
                    }
                    
                    tgt_temp = min(tgt_temp, profile->soak_temp1);
                    
                    // calculate the maximum allowable PWM duty cycle because we already know the maximum heating rate
                    //uint32_t upperlimit = lround((1.125 * 65535.0 * profile->start_rate) / max_heat_rate);
                    //upperlimit = max(upperlimit, approx_pwm(sensor.temperature_to_sensor(tgt_temp)));
                    
                    // calculate and set duty cycle
                    uint16_t pwm = pid((double)sensor.temperature_to_sensor(tgt_temp), (double)cur_sensor, &integral, &last_error);
                    pwm_ocr = pwm;
                    //pwm_ocr = pwm > upperlimit ? upperlimit : pwm;
                }
            }
            
            if (stage == 1) // thermal soak stage, ensures entire PCB is evenly heated
            {
                length_cnt++;
                if (((uint16_t)lround(length_cnt * TMR_OVF_TIMESPAN * 256) > profile->soak_length))
                {
                    // has passed time duration, next stage
                    length_cnt = 0;
                    stage++;
                    integral = 0.0;
                    last_error = 0.0;
                }
                else
                {
                    // keep the temperature steady
                    tgt_temp = (((profile->soak_temp2 - profile->soak_temp1) / profile->soak_length) * (length_cnt * TMR_OVF_TIMESPAN * 256)) + profile->soak_temp1;
                    tgt_temp = min(tgt_temp, profile->soak_temp2);
                    pwm_ocr = pid((double)sensor.temperature_to_sensor(tgt_temp), (double)cur_sensor, &integral, &last_error);
                }
            }
            
            if (stage == 2) // reflow stage, try to reach peak temp
            {
                length_cnt++;
                if (((uint16_t)lround(length_cnt * TMR_OVF_TIMESPAN * 256) > profile->time_to_peak))
                {
                    // has passed time duration, next stage
                    length_cnt = 0;
                    stage++;
                    integral = 0.0;
                    last_error = 0.0;
                }
                else
                {
                    // raise the temperature
                    tgt_temp = (((profile->peak_temp - profile->soak_temp2) / profile->time_to_peak) * (length_cnt * TMR_OVF_TIMESPAN * 256)) + profile->soak_temp2;
                    tgt_temp = min(tgt_temp, profile->peak_temp);
                    pwm_ocr = pid((double)sensor.temperature_to_sensor(tgt_temp), (double)cur_sensor, &integral, &last_error);
                }
            }
            
            if (stage == 3) // make sure we've reached peak temperature
            {
                if (sensor_to_temperature(cur_sensor) >= profile->peak_temp)
                {
                    stage++;
                    integral = 0.0;
                    last_error = 0.0;
                    length_cnt = 0;
                }
                else
                {
                    tgt_temp = profile->peak_temp + 5.0;
                    pwm_ocr = pid((double)sensor.temperature_to_sensor(tgt_temp), (double)cur_sensor, &integral, &last_error);
                }
            }
            
            if (stage == 4) // cool down
            {
                length_cnt++;
                if (cur_sensor < sensor.temperature_to_sensor(ROOM_TEMP * 1.25))
                {
                    pwm_ocr = 0; // turn off
                    tgt_temp = ROOM_TEMP;
                    stage++;
                }
                else
                {
                    // change the target temperature
                    tgt_temp = profile->peak_temp - (profile->cool_rate * TMR_OVF_TIMESPAN * 256 * length_cnt);
                    uint16_t pwm = pid((double)sensor.temperature_to_sensor(tgt_temp), (double)cur_sensor, &integral, &last_error);
                    
                    // apply a upper limit to the duty cycle to avoid accidentally heating instead of cooling
                    //uint16_t ap = approx_pwm(sensor.temperature_to_sensor(tgt_temp));
                    //pwm_ocr = pwm > ap ? ap : pwm;
                    pwm_ocr = pwm;
                }
            }
            
            element.set(pwm_ocr); // set the heating element power
            
            graph_timer += TMR_OVF_TIMESPAN * 256;
            
            if (stage != 5 && graph_timer >= graph_tick)
            {
                graph_timer -= graph_tick;
                // it's time for a new entry on the graph
                
                if (temp_history_idx == (LCD_WIDTH - 1))
                {
                    // the graph is longer than expected
                    // so shift the graph
                    for (int i = 0; i < LCD_WIDTH - 1; i++)
                    {
                        //temp_plan[i] = temp_plan[i+1];
                        temp_history[i] = temp_history[i+1];
                    }
                }
                
                // shift the graph down a bit to get more room
                int32_t shiftdown = lround((ROOM_TEMP * 1.25 / settings.max_temp) * LCD_HEIGHT);
                
                // calculate the graph plot entries
                
                int32_t plan = lround((tgt_temp / settings.max_temp) * LCD_HEIGHT) - shiftdown;
                //temp_plan[temp_history_idx] = plan >= LCD_HEIGHT ? LCD_HEIGHT : (plan <= 0 ? 0 : plan);
                
                int32_t history = lround((sensor_to_temperature(cur_sensor) / settings.max_temp) * LCD_HEIGHT) - shiftdown;
                temp_history[temp_history_idx] = history >= LCD_HEIGHT ? LCD_HEIGHT : (history <= 0 ? 0 : history);
                
                if (temp_history_idx < (LCD_WIDTH - 1) && (temp_history[temp_history_idx] != 0 /*|| temp_plan[temp_history_idx] != 0*/))
                {
                    temp_history_idx++;
                }
                
                update_graph = 1;
            }
        }
        
        if (tmr_drawlcd_flag)
        {
            tmr_drawlcd_flag = 0;
            
            // print some data to top left corner of LCD
            sprintf_P(&(graph_text[0]), "cur:%d`C ", (uint16_t)lround(sensor_to_temperature(cur_sensor)));
            sprintf_P(&(graph_text[1*((LCD_WIDTH/FONT_WIDTH) + 2)]), "tgt:%d`C ", (uint16_t)lround(tgt_temp));
            
            // tell the user about the current stage
            switch (stage)
            {
            case 0:
                sprintf(&(graph_text[2*((LCD_WIDTH/FONT_WIDTH) + 2)]), "Preheat");
                break;
            case 1:
                sprintf(&(graph_text[2*((LCD_WIDTH/FONT_WIDTH) + 2)]), "Soak   ");
                break;
            case 2:
            case 3:
                sprintf(&(graph_text[2*((LCD_WIDTH/FONT_WIDTH) + 2)]), "Reflow");
                break;
            case 4:
                sprintf(&(graph_text[2*((LCD_WIDTH/FONT_WIDTH) + 2)]), "Cool  ");
                break;
            default:
                sprintf(&(graph_text[2*((LCD_WIDTH/FONT_WIDTH) + 2)]), "Done  ");
                break;
            }
            
            // indicate whether or not this is running in demo mode
            if (DEMO_MODE)
            {
                sprintf(&(graph_text[3*((LCD_WIDTH/FONT_WIDTH) + 2)]), "Demo  ");
            }
            else
            {
                graph_text[3*((LCD_WIDTH/FONT_WIDTH) + 2)] = 0;
            }
            
            if (update_graph)
            {
                update_graph = 0;
                draw_graph();
            }
            else
            {
                for (int r = 0; r < 4; r++)
                {
                    /*lcd_set_row_column(r, 0);*/
                    printlnat(r,0, &(graph_text[r*((LCD_WIDTH/FONT_WIDTH) + 2)]));
                }
                /*lcd_draw_end();*/
            }
        }
        
        if (tmr_writelog_flag)
        {
            tmr_writelog_flag = 0;
            
            // print to CSV log format
            Serial.print(stage); Serial.print(", ");
            Serial.print(str_from_double(total_cnt * TMR_OVF_TIMESPAN * 256, 1)); Serial.print(", ");
            Serial.print(cur_sensor); Serial.print(", ");
            Serial.print(sensor.temperature_to_sensor(tgt_temp)); Serial.print(", ");
            Serial.print(str_from_int(pwm_ocr)); Serial.print(", ");
            //fprintf(&log_stream, "%s, ", str_from_int(pwm_ocr));
            //fprintf(&log_stream, "%s,\n", str_from_double(integral, 1));
        }
        
        // hold down mid button to stop
        if (button_mid())
        {
            if (stage != 5)
            {
                /*lcd_set_row_column(2, 0);*/
                printlnat(2,0,"Done    ");
                /*lcd_draw_end();*/
            }
            
            button_debounce();
            while (button_mid());
            button_debounce();
            
            if (stage != 5)
            {
                stage = 5;
            }
            else
            {
                // release and hold down again to exit
                return;
            }
        }
    }
}

void Menu::draw_graph(void)
{
    int text_end = 0;
    int r;
    for (r = 0; r < LCD_ROWS; r++)
    {
        if (r < 4)
        {
            /*lcd_set_row_column(r, 0);*/
            printlnat(r,0,&(graph_text[r*((LCD_WIDTH/FONT_WIDTH) + 2)]));
            text_end = max(text_end, FONT_WIDTH * strlen(&(graph_text[r*((LCD_WIDTH/FONT_WIDTH) + 2)])));
        }
    }
        
    for (int c = text_end; c < LCD_WIDTH; c++)
    {
        // flip the numbers because of y axis
        uint8_t history = LCD_HEIGHT - temp_history[c];
        //uint8_t plan = LCD_HEIGHT - temp_plan[c];
        
        int rowtop = r * FONT_HEIGHT; // calculate actual y location based on row and 8 pixels per row
        drawLine(c,0,c,history,graphcolor);
    }
}
    
