/*
 * DS3231_RTC_drivers.h
 *
 * Created: 4/13/2024 10:33:45 AM
 *  Author: umasi
 */ 

#ifndef DS3231_RTC_DRIVERS_H_
#define DS3231_RTC_DRIVERS_H_

/************************Function Definitions******************/
void block_write_RTC (uint8_t slave, volatile uint8_t *array_ptr, uint8_t strt_addr, uint8_t count);
void block_read_RTC (uint8_t slave, volatile uint8_t *array_ptr, uint8_t strt_addr, uint8_t count);
void I2C_rtc_DS3231_config(void);
void write_RTC (uint8_t slave, uint8_t reg_RTC, uint8_t data_RTC);
uint8_t read_RTC (uint8_t slave, uint8_t reg_RTC);

/******************Global Variables******************/
volatile uint8_t RTC_time_date_write[7];
volatile uint8_t RTC_time_date_read[7];

//***************************************************************************
// Function Name : "block_write_RTC"
// void block_write_RTC (uint8_t slave, volatile uint8_t *array_ptr,
// uint8_t strt_addr, uint8_t count)
// Target MCU : AVR128DB48 @ 4MHz
// Author : Ken Short
// DESCRIPTION
// This function writes a block of data from an array to the DS3231. strt_addr
// is the starting address in the DS3231. count is the number of data bytes to
// be transferred and array_ptr is the address of the source array in the AVR128.
//**************************************************************************

void block_write_RTC (uint8_t slave, volatile uint8_t *array_ptr, uint8_t strt_addr, uint8_t count){
	
	while((TWI0.MSTATUS & 0x03) != 0x01) ; //waiting for idle
	
	TWI0.MADDR = (slave << 1); //read mode so MC can read data
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MDATA = strt_addr; //writing register pointer
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	
	for(uint8_t i = 0; i < count; i++){
		
		TWI0.MDATA = *array_ptr; //writing the data
		array_ptr++;
		
		while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	}	
	TWI0.MCTRLB |= 0x03; //stop bit
}

//***************************************************************************
// Function Name : "block_read_RTC"
// void block_read_RTC (uint8_t slave, volatile uint8_t *array_ptr,
// uint8_t strt_addr, uint8_t count)
// Target MCU : AVR128DB48 @ 4MHz
// Author : Ken Short
// DESCRIPTION
// This function reads a block of data from the DS3231 and transfers it to an
// array. strt_addr is the starting address in the DS3231. count is the number
// of data bytes to be transferred and array_ptr is the address of the
// destination array in the AVR128.
//**************************************************************************
void block_read_RTC (uint8_t slave, volatile uint8_t *array_ptr, uint8_t strt_addr, uint8_t count){
	
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = ((slave << 1) | 0x00); //read mode so MC can read data
	
	while(!(TWI_WIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	
	TWI0.MDATA = strt_addr; //getting the address
	while(!(TWI_WIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	
	TWI0.MADDR = ((slave << 1) | 0x01); //read mode so MC can read data
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	
	for(uint8_t j = 0; j < count; j++){
		
		*array_ptr = TWI0.MDATA; //getting data
		array_ptr++;

		if (j == count - 1)
		{
			break;			
		}
		TWI0.MCTRLB = 0x02; //acknowledge bit
		while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be read
	}
	TWI0.MCTRLB = 0x07; //not acknowledge bit
}
//******************************************************************************
// Function : void I2C_rtc_DS3231_config(void)
// Date and version : 041024, version 1.0
// Target MCU : AVR128 @ 4MHz
// Author : Uma Sivadasan and Shazman Shahid

// DESCRIPTION
// This function configures an AVR128DB48 operated at
// 4 MHz to communicate with the DS3231
// SCL must be operated at the maximum possible frequency for
// the DS3231.
//******************************************************************************
void I2C_rtc_DS3231_config(void){
	
	PORTC_DIR &= ~PIN2_bm; 
	TWI0.MBAUD = 0x00;//setting up the baud rate (00 for a baud of 400kHz)
	TWI0.MCTRLA = TWI_ENABLE_bm; //enable TWI
	TWI0.DBGCTRL = 0x01; 
	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc; //make TWI bus idle
}

//***************************************************************************
// Function: void write_RTC (uint8_t slave, unit8_t reg_RTC, uint8_t data_RTC)
//
// Target MCU : AVR128DB48 @ 4MHz
// Target Hardware ;
// Author : Uma Sivadasan and Shazman Shahid

// DESCRIPTION
// This function writes data to a register in the RTC. To accomplish this, it
// must first write the DS3231’s slave address, then the register’s pointer
// address, and finally the data.
//**************************************************************************
void write_RTC (uint8_t slave, uint8_t reg_RTC, uint8_t data_RTC){
	
	/*************Byte 1***************/
	while((TWI0.MSTATUS & 0x03) != 0x01) ; //waiting for idle
	
	TWI0.MADDR = (slave << 1); //read mode so MC can read data
	
	/***********Byte 2****************/
	while((TWI0.MSTATUS & 0x40) == 0); //waiting to be able to write byte
	TWI0.MDATA = reg_RTC; //writing register pointer
	
	/*********Byte 3***************/
	while((TWI0.MSTATUS & 0x40) == 0); //waiting to be able to write byte
	TWI0.MDATA = data_RTC; //writing the data
	
	while((TWI0.MSTATUS & 0x40) == 0);
	TWI0.MCTRLB |= 0x03; //stop bit
}

//***************************************************************************
// Function: uint8_t read_RTC (uint8_t slave, uint8_t reg_RTC)
// Target MCU : AVR128DB48 @ 4MHz
// Author : Uma Sivadasan and Shazman Shahid
// DESCRIPTION
// This function reads data from a register in the RTC. To accomplish this, it
// must first write the DS3231’s slave address, then its pointer address, and
// finally read the data.
//**************************************************************************
uint8_t read_RTC (uint8_t slave, uint8_t reg_RTC){
	
	uint8_t data;
	
	/*************Byte 1***************/
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = ((slave << 1) | 0x01); //read mode so MC can read data
	
	/*************Byte 2************/
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	
	reg_RTC = TWI0.MDATA; //getting the address
	TWI0.MCTRLB = 0x02; //acknowledge bit
	
	/************Byte 3*************/
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	data = TWI0.MDATA; //getting data
	TWI0.MCTRLB = 0x07; //not acknowledge bit
	
	return data;
}

#endif 