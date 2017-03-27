#include "main.h"

//const float POSITIONS[] = {POS_LIN1, POS_LIN2, POS_LIN3, POS_LIN4, POS_LIN5};
const uint8_t MUXES[] = {MUX_LIN1, MUX_LIN2, MUX_LIN3, MUX_LIN4, MUX_LIN5};
volatile uint16_t readings[] = {0, 0, 0, 0, 0}; //10 bit output from the ADC
volatile uint32_t adjusted[] = {0, 0, 0, 0, 0}; //10 bit output from the ADC
volatile uint16_t maxes[] = {0, 0, 0, 0, 0}; //10 bit output from the ADC
volatile uint16_t mins[] = {1023, 1023, 1023, 1023, 1023}; //10 bit output from the ADC
const uint16_t centerValue = (ADC_NUMBER-1) * 500;
uint8_t curMux = 0;
uint16_t lastPos = (ADC_NUMBER-1)*500;
bool lostLine = false;


int main() {
	setupPins();
	setupADC(); //start reading line sensor values
	
	while (1) {
		//PORTB |= _BV(5);D
		//PORT_MA1 |= _BV(B_MA1);
		//PORT_MB1 |= _BV(B_MB1);
		//PORT_MB1 |= _BV(B_MB1);
		//_delay_ms(1000);
		PORT_MB1 &= ~_BV(B_MB1);
		PORT_MB2 &= ~_BV(B_MB2);
		PORT_MA1 &= ~_BV(B_MA1);
		PORT_MA2 &= ~_BV(B_MA2);
		if (PINC & _BV(6)) {
			PORT_MB1 |= _BV(B_MB1); // heats up
			PORT_MA1 |= _BV(B_MA1); //heats up
		}
		if (PINC & _BV(7)) {
			PORT_MB2 |= _BV(B_MB2); // M2 B2
			PORT_MA2 |= _BV(B_MA2); //M1 A2
		}
		
		uint16_t pos = getCoL();
		if (pos < 1000) {
			if (lostLine) PORTD |= _BV(5);
			PORT_LED0 |= _BV(B_LED0);
			PORT_LED1 &= ~_BV(B_LED1);
			PORT_LED2 &= ~_BV(B_LED2);
			PORT_LED3 &= ~_BV(B_LED3);
		} else if (pos < 1500) {
			PORTD &= ~_BV(5);
			PORT_LED0 |= _BV(B_LED0);
			PORT_LED1 |= _BV(B_LED1);
			PORT_LED2 &= ~_BV(B_LED2);
			PORT_LED3 &= ~_BV(B_LED3);
		} else if (pos < 2500) {
			PORTD &= ~_BV(5);
			PORT_LED0 &= ~_BV(B_LED0);
			PORT_LED1 |= _BV(B_LED1);
			PORT_LED2 |= _BV(B_LED2);
			PORT_LED3 &= ~_BV(B_LED3);
		} else if (pos < 3000) {
			PORTD &= ~_BV(5);
			PORT_LED0 &= ~_BV(B_LED0);
			PORT_LED1 &= ~_BV(B_LED1);
			PORT_LED2 |= _BV(B_LED2);
			PORT_LED3 |= _BV(B_LED3);
		} else {
			if (lostLine) PORTD |= _BV(5);
			PORT_LED0 &= ~_BV(B_LED0);
			PORT_LED1 &= ~_BV(B_LED1);
			PORT_LED2 &= ~_BV(B_LED2);
			PORT_LED3 |= _BV(B_LED3);
		}
	}
	
	
	// lcdInit();
	// lcdDefaults();
	// lcdPutString(0,0,"Hello World");
	// lcdDraw();
	// while(1) {
		// char* str = "XXXX";
		// lcdClearBuffer();
		// for (int i = 0; i < 5; i++) {
			// uint16_t reading = adjusted[i];
			// itoa(reading, str, 10); //base 10 decimal
			// uint8_t row = i > 2 ? 1 : 0;
			// uint8_t col = i > 2 ? (i - 3) * 5 : i * 5;
			// lcdPutString(col, row, str);
		// }
		// uint16_t pos = getCoL();
		// itoa(pos, str, 10);
		// lcdPutString(10, 1, str);
		// if (pos < 1500)	lcdPutString(14, 1, "<-");
		// else if (pos > 2500)	lcdPutString(14, 1, "->");
		// else	lcdPutString(14, 1, "||");

		// lcdDraw();
		// _delay_ms(200);
	// }
	return 1;
}

void setupADC() {
	ADMUX = _BV(REFS0);
	setMux(curMux);
	ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
	sei();
	ADCSRA |= _BV(ADSC);
}

void setMux(int mux) {
	ADMUX = (ADMUX & 0b11100000) | (MUXES[mux] & 0b00011111);
	ADCSRB = (ADCSRB & ~(_BV(MUX5))) | (MUXES[mux] & _BV(MUX5));
}

ISR(ADC_vect) {
	uint16_t reading = ADC;
	readings[curMux] = reading;
	if (reading < mins[curMux]) mins[curMux] = reading;
	if (reading > maxes[curMux]) maxes[curMux] = reading;
	
	curMux = (curMux + 1) % ADC_NUMBER;
	setMux(curMux);
	ADCSRA |= _BV(ADSC); //start new conversion
}

// float getCoL() {
	// float massDist = 0;
	// float mass = 0;
	// float spreadMin = 1;
	// float spreadMax = 0;
	// uint16_t range = maxLine - minLine;
	
	// for (uint8_t i = 0; i < ADC_NUMBER; i++) {
		// float fltReading = (readings[i] - minLine) / ((float)range);
		// fltReading = fltReading == 1 ? 0.01 : 1 - fltReading;
		// uint16_t reading = readings[i] < 1023 ? (1023 - readings[i]) : 1;
		// float fltReading =  reading / 1023.0;
		// mass += fltReading;
		// massDist += fltReading * POSITIONS[i];
	// }
	// return (massDist / mass);
// }

uint16_t getCoL() {
	uint32_t massDist = 0;
	uint32_t mass = 0;
	uint16_t spreadMin = 1000;
	uint16_t spreadMax = 0;
		
	const uint16_t lastDir = lastPos < centerValue ? 0 : (ADC_NUMBER-1)*1000;
	
	//floats have been replaced with 1000 int precision
	for (uint8_t i = 0; i < ADC_NUMBER; i++) {
		
		uint32_t range = maxes[i] - mins[i];
		if (range == 0) return centerValue;
		
		uint32_t adjReading = (((uint32_t)readings[i] - mins[i]) * 1000);
		adjReading = adjReading / (range);
		adjReading = adjReading == 0 ? 1 : adjReading;
		
		if (adjReading < spreadMin) spreadMin = adjReading;
		if (adjReading > spreadMax) spreadMax = adjReading;

		adjusted[i] = adjReading;
		mass += adjReading;
		massDist += adjReading * (i * 1000);
	}
	
	if (spreadMax - spreadMin < 250) {
		lostLine = true;
		return lastDir;
	}
	lostLine = false;
	lastPos = (uint16_t)(massDist / mass);
	return lastPos;
}

void setupPins() {
	//outputs for motor driver
	DDRB |= _BV(B_MA1) | _BV(B_MA2) | _BV(B_MB2); 
	DDRD |= _BV(B_MB1);
	//outputs for board LEDs
	DDRE |= _BV(B_LED0);
	DDRB |= _BV(B_LED1) | _BV(B_LED2) | _BV(B_LED3);
	//output for onboard LED
	DDRD |= _BV(5);
	
}