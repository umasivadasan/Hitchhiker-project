/*
 * MS8607_write_read_verification.c
 *
 * Created: 4/21/2024 4:59:44 PM
 * Author : umasi
 */ 

#include <avr/io.h>
#define F_CPU 4000000
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "MS8607_drivers.h"

volatile uint8_t high_byte;
volatile uint8_t low_byte;
volatile uint16_t PROM_PT; 

//need an array to store the I2C answer for PT
volatile uint8_t pressure_pre_conversion[3]; //3 bytes (24 bits) pressure conversion
volatile uint8_t temp_pre_conversion[3]; //3 bytes for temp conversion
volatile uint8_t hum_pre_conversion[3]; //byte 2 = high, byte 1 = low and status (2 bits), byte 0 = checksum

#define PT_ADDRESS 0x76
#define HUM_ADDRESS 0x40

int main(void)
{
	
	MS8607_init(); //initialize TWI0 for MS8607
	
	MS8607_write(PT_ADDRESS, 0x1E); //resetting for PT
	MS8607_write(HUM_ADDRESS, 0xFE); //resetting for hum
	
    while (1) 
    {
		//read PROM for PT
		MS8607_write(PT_ADDRESS, 0xA6); //sending cmd for PROM read
		PROM_PT = MS8607_PT_read(PT_ADDRESS, high_byte, low_byte); //getting prom data for PT
		
		//read Pressure conversion sequence
		MS8607_write(PT_ADDRESS, 0x48); //command for pressure conversion
		_delay_ms(20);
		MS8607_write(PT_ADDRESS, 0x00); //start read sequence
		MS8607_PT_read_convert(PT_ADDRESS, pressure_pre_conversion[2], pressure_pre_conversion[1], pressure_pre_conversion[0]);
		
		//read temp conversion sequence
		MS8607_write(PT_ADDRESS, 0x58); //command for temp conversion
		_delay_ms(20);
		MS8607_write(PT_ADDRESS, 0x00); //start read sequence
		MS8607_PT_read_convert(PT_ADDRESS, temp_pre_conversion[2], temp_pre_conversion[1], temp_pre_conversion[0]);
		
		//read hum conversion
		MS8607_hum_data(HUM_ADDRESS, 0xE7); //getting hum data
		
		//get data from hold or no hold master (which to use??)
		MS8607_hum_no_hold(HUM_ADDRESS, 0xF5, hum_pre_conversion[2], hum_pre_conversion[1], hum_pre_conversion[0]); 
		
    }
}

