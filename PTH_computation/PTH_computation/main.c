#include <avr/io.h>
#define F_CPU 4000000
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "MS8607_drivers.h"

volatile uint8_t high_byte;
volatile uint8_t low_byte;
volatile uint16_t PROM_PT;

//need an array to store the I2C answer for PT
volatile uint8_t pressure_pre_conversion[3]; //3 bytes (24 bits) pressure conversion
volatile uint8_t temp_pre_conversion[3]; //3 bytes for temp conversion
volatile uint8_t hum_pre_conversion[3]; //byte 2 = high, byte 1 = low and status (2 bits), byte 0 = checksum
volatile uint16_t C1, C2, C3, C4, C5, C6, D3;
volatile uint32_t D1,D2;
//need to convert the array into 24 bit number (unsigned int 32)

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
		MS8607_write(PT_ADDRESS, 0xA2); //sending cmd for PROM read
		C1 = MS8607_PT_read(PT_ADDRESS, high_byte, low_byte); //getting prom data for PT
		
		MS8607_write(PT_ADDRESS, 0xA4); //sending cmd for PROM read
		C2 = MS8607_PT_read(PT_ADDRESS, high_byte, low_byte); //getting prom data for PT
		
		MS8607_write(PT_ADDRESS, 0xA6); //sending cmd for PROM read
		C3 = MS8607_PT_read(PT_ADDRESS, high_byte, low_byte); //getting prom data for PT
		
		MS8607_write(PT_ADDRESS, 0xA8); //sending cmd for PROM read
		C4 = MS8607_PT_read(PT_ADDRESS, high_byte, low_byte); //getting prom data for PT
		
		MS8607_write(PT_ADDRESS, 0xA8); //sending cmd for PROM read
		C5 = MS8607_PT_read(PT_ADDRESS, high_byte, low_byte); //getting prom data for PT
		
		MS8607_write(PT_ADDRESS, 0xAC); //sending cmd for PROM read
		C6 = MS8607_PT_read(PT_ADDRESS, high_byte, low_byte); //getting prom data for PT
		
		//read Pressure conversion sequence
		MS8607_write(PT_ADDRESS, 0x48); //command for pressure conversion
		_delay_ms(20);
		MS8607_write(PT_ADDRESS, 0x00); //start read sequence
		D1 = MS8607_PT_read_convert(PT_ADDRESS, pressure_pre_conversion[2], pressure_pre_conversion[1], pressure_pre_conversion[0]);
		
		//read temp conversion sequence
		MS8607_write(PT_ADDRESS, 0x58); //command for temp conversion
		_delay_ms(20);
		MS8607_write(PT_ADDRESS, 0x00); //start read sequence
		D2 = MS8607_PT_read_convert(PT_ADDRESS, temp_pre_conversion[2], temp_pre_conversion[1], temp_pre_conversion[0]);
		
		//do the conversion
		MS8607_pressure_temp_calc(D1, D2, C1, C2, C3, C4, C5, C6); //put TEMP and P in watch window
		
		//read hum conversion
		MS8607_hum_data(HUM_ADDRESS, 0xE7); //getting hum data
		//_delay_ms(160);
		//get data from hold or no hold master (which to use??)
		D3 = MS8607_hum_hold(HUM_ADDRESS, 0xE5, hum_pre_conversion[2], hum_pre_conversion[1], hum_pre_conversion[0]);
		//_delay_ms(20);
		//try using hold instead of no hold (write function for this)
		MS8607_hum_calc(D3);
	}
}

