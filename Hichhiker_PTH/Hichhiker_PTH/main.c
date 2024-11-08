/*
 * Hichhiker_PTH.c
 *
 * Created: 4/26/2024 6:05:19 AM
 * Author : umasi
 */ 

#include <avr/io.h>
#define F_CPU 4000000
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "DS3231_RTC_drivers.h"
#include "MS8607_drivers.h"
#include "LCD_Drivers.h"

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

#define PT_ADDRESS 0x76 //pressure temp address
#define HUM_ADDRESS 0x40 //humidity address
#define DS3231_ADDR 0x68     // DS3231 address
#define START_ADDRESS 0x00

void BCD_conversion(uint8_t minutes, uint8_t seconds);

volatile uint8_t sec, min;

//display buffer for DOG LCD
char dsp_buff1[17];
char dsp_buff2[17];
char dsp_buff3[17];

int main(void)
{
   init_lcd_dog(); //initialize the lcd
   clear_dsp(); //clear the lcd display
   update_lcd_dog(); //update the lcd with the empty screen
   
   I2C_rtc_DS3231_config();//configure I2C for DS3231
   PORTC_INTFLAGS = 0x02;
   PORTC.PIN2CTRL = PORT_ISC_FALLING_gc; //enable interrupt for falling edge
   
   MS8607_init(); //initialize TWI0 for MS8607
   
   MS8607_write(PT_ADDRESS, 0x1E); //resetting for PT
   MS8607_write(HUM_ADDRESS, 0xFE); //resetting for hum
   
   
   
   RTC_time_date_write[0] = 0x00; //seconds
   RTC_time_date_write[1] = 0x0A; //minutes
   RTC_time_date_write[2] = 0x0F; //hours (15:00:00)
   RTC_time_date_write[3] = 0x00; //day
   RTC_time_date_write[4] = 0x03; //date
   RTC_time_date_write[5] = 0x05; //month
   
   block_write_RTC(DS3231_ADDR, RTC_time_date_write, START_ADDRESS, 4); //get date and time (4 bytes)
   
   /****************Alarm 1 Interrupt Every Second******************/
	//write_RTC(DS3231_ADDR, 0x0E, 0x05);
	//write_RTC(DS3231_ADDR, 0x07, 0x80);
	//write_RTC(DS3231_ADDR, 0x08, 0x80);
	//write_RTC(DS3231_ADDR, 0x09, 0x80);
	//write_RTC(DS3231_ADDR, 0x0A, 0x80);
   //write_RTC(DS3231_ADDR, 0x0F, 0x80);
   
   write_RTC(DS3231_ADDR, 0x0E, 0x05); //enable alarm 1
   write_RTC(DS3231_ADDR, 0x07, 0x80); //enable seconds
   write_RTC(DS3231_ADDR, 0x08, 0x80); //enable minutes
   write_RTC(DS3231_ADDR, 0x09, 0x80); //enable hours
   write_RTC(DS3231_ADDR, 0x0A, 0x80); //enable day/date*/
   write_RTC(DS3231_ADDR, 0x0F, 0x80);
   
   sei(); //enable global interrupts
   
    while (1){
    }
}
ISR(PORTC_PORT_vect){
	cli();
	
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
	
	block_read_RTC(DS3231_ADDR, RTC_time_date_read, START_ADDRESS, 3);
	BCD_conversion(RTC_time_date_read[1], RTC_time_date_read[0]); //BCD conversion
	
	sprintf(dsp_buff1, "%d:%d, %d/%d", RTC_time_date_read[2], min, RTC_time_date_write[5], RTC_time_date_write[4]); //time
	sprintf(dsp_buff2, "%.1fC,  %.1f%%", TEMP_F, RH_F); //temp and humidity
	sprintf(dsp_buff3, "%.1fmbar", P_F); //pressure and day
	
	 //_delay_ms(200); 
	 //should we do alarm every 2 minutes??
	update_lcd_dog();
	write_RTC(DS3231_ADDR, 0x0F, 0x80);
	
	PORTC_INTFLAGS = 0x02; //set interrupt flags
	sei();
}
void BCD_conversion(uint8_t minutes, uint8_t seconds){
	min = minutes % 60;
	sec = seconds % 60;
}
