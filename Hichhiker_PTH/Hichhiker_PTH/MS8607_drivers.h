/*
 * MS8607_drivers.h
 *
 * Created: 4/21/2024 5:00:17 PM
 *  Author: umasi
 */ 


#ifndef MS8607_DRIVERS_H_
#define MS8607_DRIVERS_H_

/********************Global Variables****************/
volatile float TEMP_F;
volatile float P_F;
volatile float RH_F;

/***********Function Defs*******************/
void MS8607_write (uint8_t SLAVE_ADDRESS, uint8_t cmd); //can be used for P&T and Humidity+
uint16_t MS8607_PT_read (uint8_t SLAVE_ADDRESS, uint8_t high_byte, uint8_t low_byte); //returns 16 bits [2 bytes]
void MS8607_init();
uint32_t MS8607_PT_read_convert(uint8_t SLAVE_ADDRESS, uint8_t high_byte, uint8_t mid_byte, uint8_t low_byte);
void MS8607_hum_data(uint8_t SLAVE_ADDRESS, uint8_t cmd);
uint16_t MS8607_hum_no_hold(uint8_t SLAVE_ADDRESS, uint8_t cmd, uint8_t high_data, uint8_t low_data, uint8_t checksum);
void MS8607_pressure_temp_calc(uint32_t D1, uint32_t D2, uint16_t C1, uint16_t C2, uint16_t C3, uint16_t C4, uint16_t C5, uint16_t C6);
void MS8607_hum_calc(uint16_t D3);

/********************Function Code********************/
void MS8607_write(uint8_t SLAVE_ADDRESS, uint8_t cmd){
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = (SLAVE_ADDRESS << 1);
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MDATA = cmd; //writing command
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MCTRLB |= 0x03;//stop bit
}
uint16_t MS8607_PT_read(uint8_t SLAVE_ADDRESS, uint8_t high_byte, uint8_t low_byte ){
	
	uint16_t data; 
	
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = (SLAVE_ADDRESS << 1) | 0x01;
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	
	high_byte = TWI0.MDATA; //reading high byte
	TWI0.MCTRLB = 0x02; //acknowledge bit
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	low_byte = TWI0.MDATA; //reading low byte
	
	TWI0.MCTRLB = 0x07; //not acknowledge bit
	TWI0.MCTRLB |= 0x03; //stop bit
	
	data = ((high_byte << 8) | (low_byte & 0x80));
	
	return data;
}
uint32_t MS8607_PT_read_convert(uint8_t SLAVE_ADDRESS, uint8_t high_byte, uint8_t mid_byte, uint8_t low_byte){
		
	uint32_t data;
	
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = (SLAVE_ADDRESS << 1) | 0x01; //read mode
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	high_byte = TWI0.MDATA; //reading high byte
	TWI0.MCTRLB = 0x02; //acknowledge bit
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	mid_byte = TWI0.MDATA; //reading mid byte
	TWI0.MCTRLB = 0x02; //acknowledge bit
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	low_byte = TWI0.MDATA; //reading low byte
	
	//while((TWI_RIF_bm & TWI0.MSTATUS)); //waiting for nack [might not need this]
	TWI0.MCTRLB = 0x07; //not acknowledge bit
	TWI0.MCTRLB |= 0x03; //stop bit
	
	data = ((uint32_t)high_byte << 16) | ((uint32_t)mid_byte << 8) | low_byte;
	
	return data;
}
void MS8607_init(){
	PORTA_DIR |= PIN3_bm | PIN2_bm;
	TWI0.MBAUD = 0x00;//setting up the baud rate
	TWI0.MCTRLA = TWI_ENABLE_bm; //enable TWI
	TWI0.DBGCTRL = 0x01; 
	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc; //make TWI bus idle
}
void MS8607_hum_data(uint8_t SLAVE_ADDRESS, uint8_t cmd){
	//write sequence
	uint8_t user_data; 
	
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = (SLAVE_ADDRESS << 1);
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MDATA = cmd; //writing command
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	
	TWI0.MCTRLB |= TWI_MCMD_REPSTART_gc; //issue repeated start
	
	//read sequence
	//while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle (do I need this?)
	TWI0.MADDR = (SLAVE_ADDRESS << 1) | 0x01; //read mode
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	
	user_data = TWI0.MDATA; //reading user data
	//wait for nack??
	TWI0.MCTRLB |= TWI_MCMD_REPSTART_gc; //issue repeated start
	
	//write sequence
	//while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle (do I need this again)
	TWI0.MADDR = (SLAVE_ADDRESS << 1); //write mode
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MDATA = cmd; //writing command
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MDATA = user_data;
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MCTRLB |= 0x03;//stop bit
}
uint16_t MS8607_hum_no_hold(uint8_t SLAVE_ADDRESS, uint8_t cmd, uint8_t high_data, uint8_t low_data, uint8_t checksum){
	
	uint16_t hum, comp_hum;
	
	//write sequence
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = (SLAVE_ADDRESS << 1);
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MDATA = cmd; //writing command
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MCTRLB |= TWI_MCMD_REPSTART_gc; //issue repeated start
	//read sequence
	//while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle (do I need this again?)
	TWI0.MADDR = (SLAVE_ADDRESS << 1) | 0x01; //read mode
	
	while(!(TWI_WIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	
	high_data = TWI0.MDATA; //reading high byte
	//TWI0.MCTRLB = 0x02; //acknowledge bit
	
	while(!(TWI_WIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	low_data = TWI0.MDATA; //reading low byte
	
	while(!(TWI_WIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	checksum = TWI0.MDATA; //getting the checksum
	//need to wait for nack??
	//TWI0.MCTRLB = 0x07; //not acknowledge bit
	TWI0.MCTRLB |= 0x03; //stop bit
	
	hum = (high_data << 8) | (low_data); //bits 15 to 2
	
	//calc compensated humidity
	comp_hum = (hum + (25 - TEMP_F) * (float)(-0.15));
	
	return comp_hum;
}
uint16_t MS8607_hum_hold(uint8_t SLAVE_ADDRESS, uint8_t cmd, uint8_t high_data, uint8_t low_data, uint8_t checksum){
	
	uint16_t hum, comp_hum;
	
	//write sequence
	while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle
	TWI0.MADDR = (SLAVE_ADDRESS << 1);
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MDATA = cmd; //writing command
	
	while(!(TWI0.MSTATUS & TWI_WIF_bm)); //waiting to be able to write byte
	TWI0.MCTRLB |= TWI_MCMD_REPSTART_gc; //issue repeated start
	//read sequence
	//while(!(TWI_BUSSTATE_IDLE_gc & TWI0.MSTATUS)); //waiting for idle (do I need this again?)
	TWI0.MADDR = (SLAVE_ADDRESS << 1) | 0x01; //read mode
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	_delay_ms(160);
	high_data = TWI0.MDATA; //reading high byte
	TWI0.MCTRLB = 0x02; //acknowledge bit
	//_delay_ms(160);
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	low_data = TWI0.MDATA; //reading low byte
	TWI0.MCTRLB = 0x02; //acknowledge bit
	
	while(!(TWI_RIF_bm & TWI0.MSTATUS)); //waiting for data to be received
	checksum = TWI0.MDATA; //getting the checksum
	//need to wait for nack??
	TWI0.MCTRLB = 0x07; //not acknowledge bit
	TWI0.MCTRLB |= 0x03; //stop bit
	
	hum = (high_data << 8) | (low_data); //bits 15 to 2
	
	//calc compensated humidity
	comp_hum = (hum + (25 - TEMP_F) * (float)(-0.15));
	
	return comp_hum;
}
void MS8607_pressure_temp_calc(uint32_t D1, uint32_t D2, uint16_t C1, uint16_t C2, uint16_t C3, uint16_t C4, uint16_t C5, uint16_t C6)
{
	int32_t dT, TEMP; 
	int64_t OFF, T2, OFF2, SENS, SENS2, P;
	
	dT = (int32_t)D2 - ((int32_t)C5 << 8); 
	TEMP = 2000 + ((int64_t)dT * (int64_t)C6 >> 23); //temp value
	
	//second order temperature comp
	
	if (TEMP < 2000) {
		T2 = (3 * ((int64_t)dT * (int64_t)dT) >> 33);
		OFF2 = 61 * ((int64_t)TEMP - 2000) * ((int64_t)TEMP - 2000) / 16;
		SENS2 = 29 * ((int64_t)TEMP - 2000) * ((int64_t)TEMP - 2000) / 16;
		
		if(TEMP < -1500){
			OFF2 += 17 * ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500);
			SENS2 += 9 * ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500);
		}
	}
	else {
		T2 = (5 * ((int64_t)dT * (int64_t)dT)) >> 38;
		OFF2 = 0;
		SENS2 = 0;
	}
	
	//calculate actual temp + pressure
	TEMP_F = ((float)TEMP - T2) / 100;
	TEMP_F = (TEMP_F-32)*5;
	TEMP_F = TEMP_F/9;
	
	//pressure calculations
	
	OFF = ((int64_t)(C2) << 17) + ((int64_t)(C4 * dT) >> 6);
	SENS = ((int64_t)(C1) << 16) + ((int64_t)(C3 * dT) >> 7);
	
	OFF -= OFF2;
	SENS -= SENS2;
	
	P = (((D1 * SENS) >> 21) - OFF) >> 15;
	P_F = (float) P / 100;
}
void MS8607_hum_calc(uint16_t D3)
{
	RH_F = ((float)D3 * 125 / (1UL<<16) + -6);
}

#endif /* MS8607_DRIVERS_H_ */