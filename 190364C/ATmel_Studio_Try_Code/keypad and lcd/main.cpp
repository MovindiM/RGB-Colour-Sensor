#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"
#include "LCD_I2C.h"


	int main()
	{
		i2c_init();
		i2c_start();
		i2c_write(0x70);
		lcd_init();
		
		while (1)
		{
			lcd_cmd(0x80);
			lcd_msg("HELLO");
			_delay_ms(1000);
			lcd_cmd(0xC3);
			lcd_msg("HI");
		}
	}

