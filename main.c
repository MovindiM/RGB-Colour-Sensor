/*
 * FINAL_CODE.c
 *
 * Created: 8/8/2021 3:44:09 PM
 * Author : Movindi
 */ 

#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define Vref 5 //reference voltage for ADC
#define Res 1 //Resistor connected to LDR and GND (1k)

char* getKey(); //Function to get input from keypad

void i2cInit(); //i2c initializing Function
void i2cStart();
void i2cWrite(char x);
char i2cRead();

void lcdInit();			//----LCD Initializing Function
void toggle();				//----Latching function of LCD
void lcdCmd_hf(char v1);   //----Function to send half byte command to LCD
void lcdCmd(char v2);		//----Function to send Command to LCD
void lcdDwr(char v3);		//----Function to send data to LCD
void lcdString(char *c);		//----Function to Send String to LCD
void lcdString_xy(char row_1, char position, char *string_00); //Function to type string at x row y column

void inputFunc(int count, char* key, int Arr[9]); //Function to convert char inputs to int and store
void getReading(); //Function to light up r, g, and b led and take values
void sensor(int rgbArr[3]); //Function of sensing Mode
void calibrate(); //Function of calibrating Mode
void rgbCalc(int inputs[9], int rgb[3]); //Function to get RGB values from given input keys
int ldrValue(); //Function o read value of LDR
int getMax(int num_1,int num_2,int num_3); //Function to get maximum value
int mapValue(int value, int maximum, int minimum); //Function to map LDR value to integer number
void rgbLED(char x, int num); //Function to control RGB LED
int checkError(int Arr[3]); //Function to check input rgb values are less than 255
void sensorOutput(int whiteArr[3], int blackArr[3], int rgbArr[3]); //Function of output (sensor Mode)

int inputCount = 0; //global variable to count inputs in input Mode

int main(void)
{
	DDRB = 0XCF; //PORTB4 and PORTB5 are inputs
	DDRD = 0XE8; //PORTD0, PORTD1, PORTD2 and PORTD4 are inputs
	DDRC = 0XFE; //PORTC0 is input others are output
	
	ADMUX = (1 << REFS0); // setting the reference of ADC  (connected to ADC0 --> 0000)
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); //ADC enable  ADC interrupt enable ADC pre-scaler selection to 128
	DIDR0 = (ADC0D); //Digital input disable
	
	int inputMode = 0;
	int sensingMode = 0;
	
	int inputArr[9] = {0,0,0,0,0,0,0,0,0}; //Array to store input values
	int whiteArr[3] = {0,0,0}; //Array to store highest values(calibrating)
	int blackArr[3] = {0,0,0}; //Array to store lowest values(calibrating)
	int rgbArr_1[3] = {0,0,0}; //Array to store RGB values for sensing mode
	int rgbArr_2[3] = {0,0,0}; //RGB values for input giving mode
	
	i2cInit();
	i2cStart();
	i2cWrite(0X70);
	lcdInit();
	
	lcdString("****WELCOME*****");//welcome message
	_delay_ms(2500);
	lcdCmd(0x01);//clear LCD
	lcdString_xy(0,0,"A:Calib");
	lcdString_xy(0,9,"B:Sense");
	lcdString_xy(1,0,"C:Input");
	lcdString_xy(1,9,"D:Reset");
	
	while (1)
	{
		char* key = getKey(); //get key from key pad
		if (strcmp(key,"NO_KEY")!=0) //checking key has been given or not
		{
			if (strcmp(key,"A")==0) //Calibrating Mode
			{
				OCR2B = 0; //pwm pins are 0
				OCR0B = 0;
				OCR0A = 0;
				int inputMode = 0;
				int sensingMode = 0;
				lcdCmd(0x01);//clear LCD
				lcdCmd(0x0C);//cursor off
				lcdString_xy(0,3,"Calibrating");
				lcdString_xy(1,5,"Mode...");
				_delay_ms(1000);
				calibrate(whiteArr,blackArr);//calibrating
			}
			else if (strcmp(key,"B")==0) //Sensing Mode
			{
				OCR2B = 0; //pwm pins are 0
				OCR0B = 0;
				OCR0A = 0;
				inputMode = 0;
				sensingMode = 1;
				lcdCmd(0x01);
				lcdCmd(0x0C);//cursor off
				lcdString_xy(0,5,"Sensing");
				lcdString_xy(1,5,"Mode...");
				_delay_ms(1000);
				sensor(rgbArr_1);//sensing
				sensorOutput(whiteArr,blackArr,rgbArr_1);//output
			}
			else if (strcmp(key,"C")==0) //Input Mode (select the input Mode)
			{
				OCR2B = 0; //pwm pins are 0
				OCR0B = 0;
				OCR0A = 0;
				inputMode = 1;
				sensingMode = 0;
				inputCount = 0;
				lcdCmd(0x01);
				lcdCmd(0x0C);
				lcdString_xy(0,6,"Input");
				lcdString_xy(1,5,"Mode...");
				_delay_ms(1000);
				lcdCmd(0x01);
				lcdString("R:");
				lcdCmd(0X0E);//cursor on
			}
			
			else if (strcmp(key,"D")==0) //Used to reset values in sensing and input modes
			{
				OCR2B = 0; //pwm pins are 0
				OCR0B = 0;
				OCR0A = 0;
				if (sensingMode == 1)//reset sensor values
				{
					sensor(rgbArr_1);//sensing
					sensorOutput(whiteArr,blackArr,rgbArr_1);//output
				}
				else if (inputMode==1)//reset inputs
				{
					inputCount = 0;
					lcdCmd(0x01);
					lcdString("R:");
					lcdCmd(0X0E);//Cursor on
				}
			}
			
			else if ((strcmp(key,"#")!=0) && (strcmp(key,"*")!=0) && (inputMode==1)) //if input mode selected and key is a number
			{
				//get inputs
				inputFunc(inputCount,key,inputArr);
				inputCount++;
				if (inputCount==9)
				{
					inputCount = 0;
					lcdCmd(0x0C); //cursor off
					_delay_ms(1000);
					rgbCalc(inputArr,rgbArr_2);//calculate rgb values
					int Error = checkError(rgbArr_2);//check error
					if (Error == 0)
					{
						rgbLED('r',rgbArr_2[0]); //Red
						rgbLED('g',rgbArr_2[1]); //Green
						rgbLED('b',rgbArr_2[2]); //Blue
					}
					else
					{
						lcdCmd(0x01);
						lcdString("Value Error!"); //if value>255
					}
				}
			}
		}
	}
}

//get inputs from keypad
char* getKey()
{
	char* key = "NO_KEY";
	PORTB = 0X01; //1st row
	if (PINB == 0X11)
	{
		key = "1";
	}
	if (PINB == 0X21)
	{
		key = "2";
	}
	if ((PINB == 0X01) && ((PIND & 0X04) == 0X04))
	{
		key = "3";
	}
	if ((PINB == 0X01) && ((PIND & 0X10) == 0X10))
	{
		key = "A";
	}
	_delay_ms(40);
	
	PORTB = 0X02; //2nd row
	if (PINB == 0X12)
	{
		key = "4";
	}
	if (PINB == 0X22)
	{
		key = "5";
	}
	if ((PINB == 0X02) && ((PIND & 0X04) == 0X04))
	{
		key = "6";
	}
	if ((PINB == 0X02) && ((PIND & 0X10) == 0X10))
	{
		key = "B";
	}
	_delay_ms(40);
	
	PORTB = 0X04; //3rd row
	if (PINB == 0X14)
	{
		key = "7";
	}
	if (PINB == 0X24)
	{
		key = "8";
	}
	if ((PINB == 0X04) && ((PIND & 0X04) == 0X04))
	{
		key = "9";
	}
	if ((PINB == 0X04) && ((PIND & 0X10) == 0X10))
	{
		key = "C";
	}
	_delay_ms(40);
	
	PORTB = 0X08; //4th row
	if (PINB == 0X18)
	{
		key = "*";
	}
	if (PINB == 0X28)
	{
		key = "0";
	}
	if ((PINB == 0X08) && ((PIND & 0X04) == 0X04))
	{
		key = "#";
	}
	if ((PINB == 0X08) && ((PIND & 0X10) == 0X10))
	{
		key = "D";
	}
	_delay_ms(40);
	return key;
}

void i2cInit(){
	TWBR = 0x62;		//	Baud rate is set by calculating
	TWCR = (1<<TWEN);	//Enable I2C
	TWSR = 0x00;		//Pre-scaler set to 1
}

//Start condition
void i2cStart(){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);	//start condition
	while (!(TWCR & (1<<TWINT)));				//check for start condition

}

//I2C stop condition
void i2cWrite(char x){
	TWDR = x;						//Move value to I2C
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
}

char i2cRead(){
	TWCR  = (1<<TWEN) | (1<<TWINT);	//Enable I2C and clear interrupt
	while (!(TWCR & (1<<TWINT)));	//Read successful with all data received in TWDR
	return TWDR;
}

void toggle()
{
	TWDR |= 0x02;					//---PIN En de la LCD en = 1;  -----Latching data in to LCD data register using High to Low signal
	TWCR = (1<<TWINT) | (1<<TWEN);	//---Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	_delay_ms(1);
	TWDR &= ~0x02;					//---PIN del Enable de la LCD en = 0;
	TWCR = (1<<TWINT) | (1<<TWEN);	//---Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
}

void lcdCmd_hf(char v1)
{
	TWDR &=~0x01;					//PIN RS de la LCD rs = 0; ----Selecting register as Command register
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR &= 0x0F;					//----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= (v1 & 0xF0);			//----Masking higher 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
}

//Send command to LCD
void lcdCmd(char v2)
{
	TWDR&=~0x01;					//rs = 0; ----Selecting register as command register
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR &= 0x0F;                   //----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= (v2 & 0xF0);			//----Masking higher 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
	
	TWDR &= 0x0F;                    //----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= ((v2 & 0x0F)<<4);		//----Masking lower 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
}

//send data to LCD
void lcdDwr(char v3)
{
	TWDR|=0x01;						//rs = 1; ----Selecting register as command register
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR &= 0x0F;				    //----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= (v3 & 0xF0);			//----Masking higher 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
	
	TWDR &= 0x0F;					//----clearing the Higher 4 bits
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	TWDR |= ((v3 & 0x0F)<<4);		//----Masking lower 4 bits and sending to LCD
	TWCR = (1<<TWINT) | (1<<TWEN);	//Enable I2C and clear interrupt
	while  (!(TWCR &(1<<TWINT)));
	toggle();
}

//initiating LCD
void lcdInit()
{
	lcdCmd_hf(0x30);       //-----Sequence for initializing LCD
	lcdCmd_hf(0x30);       //-----     "            "              "               "
	lcdCmd_hf(0x20);       //-----     "            "              "               "
	lcdCmd(0x28);          //-----Selecting 16 x 2 LCD in 4Bit mode
	lcdCmd(0x0C);          //-----Display ON Cursor OFF
	lcdCmd(0x01);          //-----Clear display
	lcdCmd(0x06);          //-----Cursor Auto Increment
	lcdCmd(0x80);          //-----1st line 1st location of LCD
}

//send string to LCD
void lcdString(char *c)
{
	while(*c != 0)      //----Wait till all String are passed to LCD
	lcdDwr(*c++);		//----Send the String to LCD
}

//sent string to x row y column of LCD
void lcdString_xy(char row_1, char position, char *string_00)
{
	if ((row_1 == 0) && (position<16))
	{
		lcdCmd((position & 0x0F) | 0x80);
		lcdString(string_00);
	}
	else if ((row_1 == 01) && (position<16))
	{
		lcdCmd((position & 0x0F) | 0xC0);
		lcdString(string_00);
	}
}

//convert char input to int and store(used in input mode)
void inputFunc(int count, char* key, int Arr[9])
{
	lcdString(key);
	_delay_ms(300);
	Arr[count] = atoi(key);
	if (count==2)
	{
		lcdString_xy(0,10,"G:");
	}
	else if (count==5)
	{
		lcdString_xy(1,0,"B:");
	}
}

//getting LDR resistant
int ldrValue()
{
	//float Vin = voltage at ADC pin
	float LDR; //resistant of LDR
	ADCSRA |= (1 << ADSC); // ADC start conversion
	_delay_ms(100);
	/*ADC = (Vin*1024)/Vref
	  Vin = (ADC*Vref)/1024
	  LDR = (1/Vin)*(Vref-Vin)
	  LDR = (1024/ADC)-1*/
	LDR = (1024/ADC)-1;
	int x = round(LDR);
	return x;
}

//light up r, g, b led and take values(used in both calibration and sensor mode)
void getReading(int rgbArr[3])
{
	PORTC |= 0X02; //light red led
	_delay_ms(300);
	//get readings
	rgbArr[0] = ldrValue();
	PORTC &= 0XFD; //off red led
	_delay_ms(1000);
	PORTC |= 0X04; //light green led
	_delay_ms(300);
	//get readings
	rgbArr[1] = ldrValue();
	PORTC &= 0XFB; //off green led
	_delay_ms(1000);
	PORTC |= 0X08; //light blue led
	_delay_ms(300); 
	//get readings
	rgbArr[2] = ldrValue();
	PORTC &= 0XF7; //off blue led
}

//Sensing Mode
void sensor(int rgbArr[3])
{
	lcdCmd(0x01); //clear LCD display
	lcdString("Put the Color...");
	_delay_ms(2000);
	lcdCmd(0x01);
	lcdString("Sensing...");
	getReading(rgbArr);
	lcdCmd(0X01);
	lcdString("Done");
}

//get maximum
int getMax(int num_1,int num_2,int num_3)
{
	int Maximum = 0;
	int Arr[3] = {num_1,num_2,num_3};
	for (int i=0;i<3;i++)
	{
		if (Arr[i]>Maximum)
		{
			Maximum = Arr[i];
		}
	}
	return Maximum;
}

//Function of calibrating mode
void calibrate(int whiteArr[3],int blackArr[3])
{
	int Arr_1[3] = {0,0,0};
	int Arr_2[3] = {0,0,0};
	int Arr_3[3] = {0,0,0};
	lcdCmd(0x01);
	lcdString("Put White"); //white
	_delay_ms(2000);
	lcdCmd(0x01);
	lcdString("Calibrating...");
	_delay_ms(10);
	getReading(whiteArr);//lighting up LEDs and sensing
	lcdCmd(0x01);
	lcdString("Put Color1"); //Color1
	_delay_ms(2000);
	lcdCmd(0x01);
	lcdString("Calibrating...");
	_delay_ms(10);
	getReading(Arr_1);//lighting up LEDs and sensing
	lcdCmd(0x01);
	lcdString("Put Color2");
	_delay_ms(2000);
	lcdCmd(0x01);
	lcdString("Calibrating...");
	_delay_ms(10);
	getReading(Arr_2);//lighting up LEDs and sensing
	lcdCmd(0x01);
	lcdString("Put Color3");
	_delay_ms(2000);
	lcdCmd(0x01);
	lcdString("Calibrating...");
	_delay_ms(10);
	getReading(Arr_3);//lighting up LEDs and sensing
	lcdCmd(0x01);
	lcdString("Calibration done");
	//light intensity and resistant of LDR are inversely proportional. to get darkest values maximum resistant should be taken
	blackArr[0] = getMax(Arr_1[0],Arr_2[0],Arr_3[0]);
	blackArr[1] = getMax(Arr_1[1],Arr_2[1],Arr_3[1]);
	blackArr[2] = getMax(Arr_1[2],Arr_2[2],Arr_3[2]);
}

//Function to calculate RGB values from given inputs
void rgbCalc(int inputs[9], int rgb[3])
{
	rgb[0] = (inputs[0]*100)+(inputs[1]*10)+inputs[2];
	rgb[1] = (inputs[3]*100)+(inputs[4]*10)+inputs[5];
	rgb[2] = (inputs[6]*100)+(inputs[7]*10)+inputs[8];
}

//Function to map LDR value to integer number
int mapValue(int value, int maximum, int minimum)
{
	float x=255.00;
	int a = (maximum-minimum);
	int b = (maximum-value);
	float c = x*b/a;
	int y = round(c);
	if (y<0)
	{
		y = 0;
	}
	return y;
}

//Function to control RGB LED
void rgbLED(char x, int num)
{
	TCCR0B = (1 << CS00) | (1 << CS01); //pre scaler 64 ( for OC0A and OC0B)
	TCCR2B = (1 << CS20) | (1 << CS21); //pre scaler 64 ( for OC2B)
	
	if (x == 'r')
	{
		TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2B1); // fast PWM non inverting
		OCR2B = num;
	}
	else if (x == 'g')
	{
		TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0B1); // fast PWM non inverting
		OCR0B = num;
	}
	else if (x == 'b')
	{
		TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1); //fast PWM non inverting
		OCR0A = num;
	}
}

//Function to check input rgb values are less than 255
int checkError(int Arr[3])
{
	int Error = 0;
	for (int i = 0; i<3; i++)
	{
		if (Arr[i] > 255)
		{
			Error = 1;
			break;
		}
	}
	return Error;
}

//Function of output (sensor Mode)
void sensorOutput(int whiteArr[3], int blackArr[3], int rgbArr[3] )
{
	int value;
	char str[20];
	lcdCmd(0x01); //clear LCD display
	//Red
	value = mapValue(rgbArr[0],blackArr[0],whiteArr[0]);
	lcdString("R:");
	sprintf(str, "%d", value);
	lcdString(str);
	rgbLED('r',value);
	//Green
	value = mapValue(rgbArr[1],blackArr[1],whiteArr[1]);
	lcdString_xy(0,10,"G:");
	sprintf(str, "%d", value);
	lcdString(str);
	rgbLED('g',value);
	//Blue
	value = mapValue(rgbArr[2],blackArr[2],whiteArr[2]);
	lcdString_xy(1,0,"B:");
	sprintf(str, "%d", value);
	lcdString(str);
	rgbLED('b',value);
}

