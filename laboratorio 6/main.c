/*
 * Lab6.c
 *
 * Created: 19/04/2024 00:34:21
 * Author : esteb
 */ 	


#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>


int activa = 0, activa2 = 0;
char dato;
void initADC(void);
void initUART9600(void);
void writeUART(char Caracter);
void writeTextUART(char * Texto);
volatile uint8_t  datoTX;
volatile char receivedChar = 0;


int main(void)
{
	DDRB = 0b00111111;  //Salida hacia LEDs  PB0 a PB5
		
	initADC();
	initUART9600();
	
	sei(); 
	
	
	while (1)
	{
	//************MENU*********************************
	if(activa == 0){
	writeTextUART("\n\r**************Menu****************"); 
	writeTextUART("\n\r*******Ingrese un valor****************");    //Mostrar inicio 
	writeUART(10);
	writeUART(13);
	writeUART(10);
	writeTextUART("          Coloque 1 para leer potenciometro");   //Mostrar inicio
	writeUART(10);
	writeUART(13);
	writeTextUART("          Coloque 2 para enviar ASCII\n\r");   //Mostrar inicio
	writeUART(10);
	writeUART(13);
	activa = 1;
	}
	
	if(receivedChar != 0){
		switch(receivedChar)
		{
			case '1':
			writeTextUART("\n\rLectura actual de potenciometro es: ");
			ADCSRA |= (1 << ADSC);   //Leer puerto de ADC
			uint16_t valor_decimal = (ADC * 5 / 1023.0) * 100; // Convertir a decimal
			char buffer[10];
			itoa(valor_decimal, buffer, 10); // Convertir a cadena de caracteres
			writeTextUART(buffer);
			writeTextUART("\n\r");
			activa = 0;
			receivedChar = 0;
			break;
			
			case '2':
			receivedChar = 0;
			writeTextUART("\n\rIngrese un valor\n\r");
			activa2 = 1;
			break;
			
		}
	}
		
	
}

}

void initUART9600(void)
{
	//Paso 1: RX y TX como salida
	DDRD &= ~(1<<DDD0);
	DDRD |= (1<<DDD1);
	
	//Paso 2:Configurar UCSR0A
	UCSR0A = 0;
	
	//Paso 3: configurar UCSR0B donde se habilita ISR de recepcion y se habilita RX y TX
	UCSR0B = 0;
	UCSR0B |= (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
	
	//Paso 4: Configurar UCSR0C, asincrono, paridad: none, 1 bit stop, data bits de 8 bits
	UCSR0C = 0;
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
	
	//Paso 5: COnfigurar velocidad de baudrate a 9600
	UBRR0 = 103;
	
}


void initADC(){
	ADMUX = 6;
	ADMUX |= (1<<REFS0);  //Referencia AVCC = 5V
	ADMUX &= ~(1<<REFS1); //Justificación a la izquierda
	ADMUX |= (1<<ADLAR);
	ADCSRA = 0;
	ADCSRA |= (1<<ADIE);  //Habilitando interrupcion de ADC
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);  //Habilitando prescaler de 16M/128 frecuencia = 125Khz
	ADCSRA |= (1<<ADEN);   //Habilitando el ADC
}




ISR(ADC_vect)
{
	dato = ADCH;   //Contador general
	ADCSRA |= (1<<ADIF); //Se borra la bandera de interrupción
}


//Funcion de escritura de un caracter
void writeUART(char Caracter)
{
	while(!(UCSR0A & (1<<UDRE0)));  //hasta que la bandera este en 1
		UDR0 = Caracter;
		
}

 //Función de escritura de una cadena de caracteres
void writeTextUART(char * Texto){
	uint8_t o;
	for(o = 0; Texto[o]!= '\0'; o++){
		while(!(UCSR0A & (1<<UDRE0)));
			UDR0 = Texto[o];
	}
}

ISR(USART_RX_vect)
{
	receivedChar = UDR0; // Almacena el carácter recibido
	
	if (activa2 == 1){
		// Dividir el carácter recibido en dos partes
		uint8_t lower_bits = receivedChar & 0b00111111; // Los 6 bits menos significativos
		uint8_t upper_bits = (receivedChar >> 6) & 0b11; // Los 2 bits más significativos

		// Mostrar los 6 bits menos significativos en PORTB
		PORTB = lower_bits;

		// Mostrar los 2 bits más significativos en los pines PD6 y PD7 de PORTD
		PORTD = (PORTD & ~0b11000000) | (upper_bits << 6);
		activa2 = 0;
		activa = 0;
	}
	
	while(!(UCSR0A & (1<<UDRE0)));    //Mientras haya caracteres
		UDR0 = receivedChar;
		
	

}

