//****main.c****//
#include "lcd.h"
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h>

//Defined values 
#define FanPin (1<<0) //PB0
#define HeaterPin (1<<1) //PB1

//Global variables 
static unsigned int set_temp = 20; //The temp the system is trying to achieve
bool display_temp = true; //enables the display function
bool enable_dry = false; //Whether the heating process is enabled or not, "safety" feature

//ADC conversion variables
int val;
long R;
double Thermister;

int air_temp_adc()
{
	ADMUX = 0b10000011;		
	ADCSRA |= (1<<ADSC);	
	while ((ADCSRA &(1<<ADIF))==0);
	ADCSRA |=(1<<ADIF);	
	return(ADC);		
}

double getTemp()
{
	val = air_temp_adc();		
	R=((10230000/val) - 10000); //calculate the resistance 
	Thermister = log(R);	//calculate natural log of resistance
	//Steinhart-Hart Thermistor Equation:
	Thermister = 1 / (0.001129148 + (0.000234125 * Thermister) + (0.0000000876741 * Thermister * Thermister * Thermister));
	Thermister = Thermister - 273.15;// convert kelvin to °C 
	
	return Thermister;
}

void startup(void);
void update(void); 
void print_LCD(unsigned int current_temp);



int main(void){	
	startup(); 
	while(1){
		update(); 
	}
	return 0; 
}

void startup()
{		
	
		// data Directions  
		DDRD &= ~((1<<2)|(1<<3)|(1<<4)|(1<<5));
		DDRC &= ~(1<<3);
		DDRB |= (FanPin | HeaterPin);
		
		//interrupt settings 
		TIMSK1 = (1 << TOIE1);
		EIMSK = 0x03;
		EICRA = 0x0A; 
		sei();
		
		//timer settings 
		TCCR1A = 0x00;
		TCCR1B = 0x03;
		TCNT1 = 0;
		
		//ADC Settings
		//ADMUX = 0b11000011;
		ADCSRA = 0b10010111;
		print_LCD(0);
		
		//LCD Settings 
		lcd_init(LCD_DISP_ON);    // init lcd and turn on
}


void update()
{		
	//monitors the start and stop pins
	if((PIND & (1<<4)) != 0)
	{
		enable_dry = true;
	}
	if((PIND & (1<<5)) != 0)
	{
		enable_dry = false;
	}
}


void print_LCD(unsigned int c){
	//current temp
	lcd_gotoxy(2,2); 
	lcd_puts("CT: "); 
	char *current_temp_char[3]; 
	itoa(c, *current_temp_char, 10);
	lcd_puts(*current_temp_char); 
	lcd_puts(" °C"); 
	
	//set temp
	lcd_gotoxy(2,4);
	lcd_puts("ST: ");
	char *set_temp_char[3];
	itoa(set_temp, *set_temp_char, 10);
	lcd_puts(*set_temp_char);
	lcd_puts(" °C");
}

ISR(TIMER1_OVF_vect)
{
	int temp = getTemp();
	if(display_temp)
	{
	print_LCD(temp); 	
	}
//set heater status based on current temp
	if (enable_dry)
	{
		PORTB |= FanPin;
		if(temp<set_temp)
		{
			PORTB |= HeaterPin;
		}
		else
		{
			PORTB &= ~HeaterPin;
		}
		TCNT1 = 0;
	}
	else
	{
		PORTB &= ~HeaterPin;
		PORTB &= ~FanPin;
	}
}

ISR(INT0_vect)
{
	++set_temp;
}

ISR(INT1_vect)
{
	--set_temp;
}