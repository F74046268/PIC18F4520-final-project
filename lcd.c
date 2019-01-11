#include <pic18f4520.h>

#include "lcd.h"

void oled_init(void)
{
    _rs_pin = 0;
    _rw_pin = 0;
    _en_pin = 0;

    oled_write_command(0x38); //FUNCTION SET INSTRUCTION
    oled_write_command(0x08); //Display on/off control  (off)
    oled_write_command(0x01); //Display Clear
    oled_write_command(0x0c); //Display ON/OFF Control  (on)
    oled_write_command(0x06); //Entry Mode Set 00000110
		//	000001XX
		//		  ||--	Shift Entire Display Control Bit
		//		  |      >>0: Decrement DDRAM Address by 1 when a character
		//		  |		       code is written into or read from DDRAM
		//		  |        1: Increment DDRAM Address by 1 when a character
		//		  |		       code is written into or read from DDRAM
		//		  |
		//		  |---	Increment/Decrement bit
		//                0: when writing to DDRAM, each
		//                   entry moves the cursor to the left
		//              >>1: when writing to DDRAM, each 
       //		  		       entry moves the cursor to the right
    oled_write_command(0x02); //Return Home 00000010
}

void oled_clear_display(void)
{
    _en_pin = 0;
    oled_write_command(0x01);
}

void oled_write_upper_line(unsigned char *string)
{
    // using snprintf() with string
    unsigned char i = 0;
    for (; i < 12 && string[i] != NULL; i++) {  //這塊LCD的DDRAM為12x2 每一格能放8bits
        oled_set_DDRAM(i, 0);                 //先放前12格
        oled_write_data(string[i]);
    }
    for (; i < 12; i++) {                   //如果string有null,替換成 ' '
        oled_set_DDRAM(i, 0);
        oled_write_data(' ');
    }
    oled_set_DDRAM(0, 0);
}

void oled_write_lower_line(unsigned char *string)
{
    unsigned char i = 0;
    for (; i < 12 && string[i] != NULL; i++) {
        oled_set_DDRAM(i, 1);
        oled_write_data(string[i]);
    }
    for (; i < 12; i++) {
        oled_set_DDRAM(i, 1);
        oled_write_data(' ');
    }
    oled_set_DDRAM(0, 0);
}


void oled_write_data(unsigned char value)
{
    _en_pin_write = 0;
    _rs_pin_write = 1;
    _rw_pin_write = 0;
    oled_write_8bits(value);
    oled_check_busy();
}

void oled_set_DDRAM(unsigned char x, unsigned char y)
{
    oled_write_command(0x80 | (y << 6) | x);
    // 1000 0000        1000 0000
    //    +               +
    // 0000 0000        0100 0000
    //    +               +
    // 00xx xxxx        00xx xxxx      -> from 0 ~ 12
    // for first line   for second line
}

void oled_write_command(unsigned char value)
{
    _en_pin_write = 0;
    _rs_pin_write = 0;
    _rw_pin_write = 0;
    oled_write_8bits(value);
    oled_check_busy();
}

void oled_write_8bits(unsigned char value)
{
    _data_pins = 0x00; //set output
    _data_pins_write = value;

    _en_pin_write = 0;
    __delay_us(50);
    _en_pin_write = 1;
}

void oled_check_busy(void)
{
    unsigned char busy = 1;
    _busy_pin = 1; // make busy pin input
    _rs_pin_write = 0;
    _rw_pin_write = 1;

    do {
        _en_pin_write = 0;
        _en_pin_write = 1;

        __delay_us(10);
        busy = _busy_pin_read; // HIGH means not ready for next command

        _en_pin_write = 0;
    } while(busy);

    _busy_pin = 0; // make busy pin output
    _rw_pin_write = 0;
}