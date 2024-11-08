/*
 * LCD_Drivers.h
 *
 * Created: 4/26/2024 6:11:36 AM
 *  Author: umasi
 */ 


#ifndef LCD_DRIVERS_H_

char dsp_buff1[17];
char dsp_buff2[17];
char dsp_buff3[17];


/****************LCD Defs********************/
void lcd_spi_transmit (uint8_t data);
void lcd_spi_transmit_DATA (uint8_t data);
void init_spi_lcd (void);
void init_lcd_dog (void);
void update_lcd_dog(void);
void clear_dsp(void);

/***********************Transmit Command*********************/
void lcd_spi_transmit_CMD (uint8_t data)
{
	
	//RS = 0 for command
	PORTC_OUT = ~PIN0_bm;
	//select - SS low
	PORTA_OUT &= ~PIN7_bm;
	//send data
	SPI0_DATA = data;
	//wait until data is received
	while(!(SPI0_INTFLAGS & SPI_IF_bm))
	{
		;
	}
	//deselect -- SS high
	PORTA_OUT |= PIN7_bm;
}
/*************************Transmit Data**********************/
void lcd_spi_transmit_DATA (uint8_t data)
{
	//RS = 1 for data
	PORTC_OUT |= PIN0_bm;
	//select - SS low
	PORTA_OUT &= ~PIN7_bm;
	//send data
	SPI0_DATA = data;
	//wait until data is received
	while(!(SPI0_INTFLAGS & SPI_IF_bm))
	{
		;
	}
	//deselect -- SS high
	PORTA_OUT |= PIN7_bm;
}
/***************************Initialize SPI for LCD**********************/
void init_spi_lcd (void)
{
	SPI0_CTRLB |= SPI_MODE_3_gc | SPI_SSD_bm;		//mode 3 - CPHA = 1, CPOL = 1
	SPI0_CTRLA |= SPI_MASTER_bm | SPI_ENABLE_bm;	//master mode, enable SPI0
	PORTA_DIR |= PIN4_bm | PIN6_bm | PIN7_bm;	//MOSI, SCK, SS as output
	PORTA_DIR &= ~PIN5_bm;						//MISO as input
	PORTC_DIR |= PIN0_bm;						//RS for LCD as output
}
/***************Initialize LCD*********************/
void init_lcd_dog (void)
{
	init_spi_lcd();
	_delay_ms(40);					//startup delay
	
	//func_set1:
	lcd_spi_transmit_CMD(0x39);
	_delay_us(30);					//func set delay

	//func_set2:
	lcd_spi_transmit_CMD(0x39);
	_delay_us(30);					//func set delay

	//bias_set:
	lcd_spi_transmit_CMD(0x1E);
	_delay_us(30);					//func set delay

	//power_ctrl:
	lcd_spi_transmit_CMD(0x55);		//~ 0x50 nominal for 5V //~ 0x55 for 3.3V (delicate adjustment)
	_delay_us(30);					//func set delay

	//follower_ctrl:
	lcd_spi_transmit_CMD(0x6C);		//follower mode on...
	_delay_us(30);					//func set delay

	//contrast_set:
	lcd_spi_transmit_CMD(0x7F);		//~ 77 for 5V, ~ 7F for 3.3V
	_delay_us(30);					//func set delay

	//display_on:
	lcd_spi_transmit_CMD(0x0c);		//display on, cursor off, blink off
	_delay_us(30);					//func set delay

	//clr_display:
	lcd_spi_transmit_CMD(0x01);		//clear display, cursor home
	_delay_us(30);					//func set delay

	//entry_mode:
	lcd_spi_transmit_CMD(0x06);		//clear display, cursor home
	_delay_us(30);					//func set delay
}

/*********************Update LCD********************/
void update_lcd_dog(void)
{
	//send line 1
	lcd_spi_transmit_CMD(0x80);
	_delay_us(30);
	for(uint8_t i = 0; i < 16; i++)
	{
		lcd_spi_transmit_DATA(dsp_buff1[i]);
		_delay_us(30);
	}
	//send line 2
	lcd_spi_transmit_CMD(0x90);
	_delay_us(30);
	for(uint8_t i = 0; i < 16; i++)
	{
		lcd_spi_transmit_DATA(dsp_buff2[i]);
		_delay_us(30);
	}
	//send line 3
	lcd_spi_transmit_CMD(0xA0);
	_delay_us(30);
	for(uint8_t i = 0; i < 16; i++)
	{
		lcd_spi_transmit_DATA(dsp_buff3[i]);
		_delay_us(30);
	}
	
}
void clear_dsp(void)
{
	//clear buffer 1
	for(uint8_t i = 0; i < 16; i++)
	{
		dsp_buff1[i] = ' ';
	}
	//clear buffer 2
	for(uint8_t i = 0; i < 16; i++)
	{
		dsp_buff2[i] = ' ';
	}
	//clear buffer 3
	for(uint8_t i = 0; i < 16; i++)
	{
		dsp_buff3[i] = ' ';
	}
}





#define LCD_DRIVERS_H_





#endif /* LCD_DRIVERS_H_ */