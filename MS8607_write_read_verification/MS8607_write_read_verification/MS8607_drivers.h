/*
 * MS8607_drivers.h
 *
 * Created: 4/21/2024 5:00:17 PM
 *  Author: umasi
 */ 


#ifndef MS8607_DRIVERS_H_
#define MS8607_DRIVERS_H_


/***********Function Defs*******************/
void MS8607_write (uint8_t SLAVE_ADDRESS, uint8_t cmd); //can be used for P&T and Humidity+
uint16_t MS8607_PT_read (uint8_t SLAVE_ADDRESS, uint8_t high_byte, uint8_t low_byte); //returns 16 bits [2 bytes]
void MS8607_init();
void MS8607_PT_read_convert(uint8_t SLAVE_ADDRESS, uint8_t high_byte, uint8_t mid_byte, uint8_t low_byte);
void MS8607_hum_data(uint8_t SLAVE_ADDRESS, uint8_t cmd);
void MS8607_hum_no_hold(uint8_t SLAVE_ADDRESS, uint8_t cmd, uint8_t high_data, uint8_t low_data, uint8_t checksum);

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
void MS8607_PT_read_convert(uint8_t SLAVE_ADDRESS, uint8_t high_byte, uint8_t mid_byte, uint8_t low_byte){
		
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
	
	//TWI0.MCTRLB = 0x07; //not acknowledge bit
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
void MS8607_hum_no_hold(uint8_t SLAVE_ADDRESS, uint8_t cmd, uint8_t high_data, uint8_t low_data, uint8_t checksum){
	uint32_t data;
	
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
	
	data = (high_data << 16) | (low_data << 8) | checksum;
	
}
#endif /* MS8607_DRIVERS_H_ */