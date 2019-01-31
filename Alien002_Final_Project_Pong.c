/*	Alien002_Final_Project_Pong.c - $8/28/18$
 *	Name & E-mail: Alic Lien - Alien002@ucr.edu
 *	CS Login: Alien002
 *	Partner(s) Name & E-mail:  - 
 *	Lab Section: 
 *	Assignment: Final Lab Project Pong 
 *	Exercise Description: 8 - bit Pong game using LCD Matrix, LCD Display for information/scores, 1/2 player game,
 *	and utilizes shift register.
 *	
 * 
 *	I acknowledge all content contained herein, excluding template or example 
 *	code, is my own original work.
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#include "io.c"
#include "timer.c"
#include <util/delay.h>

#define TIMERPERIOD 1
//******			GLOBAL VARIABLES			******//

//Paddle init
char paddle1 = 0x38;
char paddle2 = 0x38;
//Buttons
char p1UP = 0x00;
char p1DWN = 0x00;
char p2UP = 0x00;
char p2DWN = 0x00;
//Menu buttons
char button1;
char button2; 
//Ball
int ballX = 0b00001000;
int ballY = 0b00010000;

//flag modified by MENU_SM for single or multiplayer
char flag = 0;

//reset flag
char reset2 = 0;

//Scorekeeping
int p1SCORE = 0;
int p2SCORE = 0;


#define D4 293.66	//violin D string
#define E4 329.63

//****************************************************//


//******			PWM FUNCTIONS				******//

// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		// prevents OCR3A from underflowing, using prescaler 64
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}
void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}
void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}
//****************************************************//


unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}


//******		DISPLAY IN/OUT FUNCTIONS		******//
int MATRIXrow[] = {9, 14, 8, 12, 1, 7, 2, 5};
int MATRIXcolumn[] = {13, 3, 4, 10, 6, 11, 15, 16};

void matrixDisplay(unsigned char x, unsigned char y) {
	unsigned char input[16];
	x = ~x;					//Reverses inputs for 
	for(int i = 0; i < 8; ++i) {
		if((x >> i) & 0x01) {
			input[MATRIXrow[i]-1] = 0x01;
			} else {
			input[MATRIXrow[i]-1] = 0x00;
		}
	}
	for(int j = 0; j < 8; ++j) {
		if((y >> j) & 0x01) {
			input[MATRIXcolumn[j]-1] = 0x01;
			} else {
			input[MATRIXcolumn[j]-1] = 0x00;
		}
	}

	for(int k = 16; k > 0; --k) {
		PORTD = input[k-1];
		PORTD = input[k-1] | 0x04;
	}
	PORTD = 0x02;
}
//****************************************************//



//******				MENU STATES				******//
enum MENU_States { MENU_START, MENU_ON, MENU_SET_FLAG } MENU_State;

Menu() {
	switch(MENU_State) { // Transitions
		case -1:
			
			MENU_State = MENU_START;
			break;
		case MENU_START:
			LCD_ClearScreen();
			LCD_DisplayString(1, "1. Single Player2. Multiplayer");
			MENU_State = MENU_ON;
			break;
		case MENU_ON:
			if ((button1 && !button2) || (!button1 && button2)) {
				MENU_State = MENU_SET_FLAG;
				if(button1 && !button2){
					flag = 1;
					LCD_ClearScreen();
					LCD_DisplayString(1, "Single Player   Selected");					delay_ms(1922);		//1922 the year Soviet Union began
				}
				else if(!button1 && button2){
					flag = 2;
					LCD_ClearScreen();
					LCD_DisplayString(1, "Multi Player    Selected");					delay_ms(1922);		//truly khorosho 
				}
			}
			break;
		case MENU_SET_FLAG:
			break;
		default:
			MENU_State = MENU_ON;
	} // Transitions

	switch(MENU_State) { // State actions
		case MENU_START:
			break;
		case MENU_ON:
			button1 = (~PINA) & 0x01;
			button2 = (~PINA) & 0x02;
			break;
		case MENU_SET_FLAG:
			break;
		default: // ADD default behaviour below
			break;
	} // State actions

}

//****************************************************//



//******	SCORES + LCD DISPLAY STATE MACHINE	******//
enum SCORE_States { SCORE_START, SCORE_ON, SCORE_UPDATE, SCORE_WINNER } SCORE_State;

Score_SM() {
	switch(SCORE_State) { // Transitions
		case -1:
			SCORE_State = SCORE_START;
			break;
		case SCORE_START:
		LCD_ClearScreen();
			if (1) {
				SCORE_State = SCORE_ON;
			}
			break;
		case SCORE_ON:
			
			SCORE_State = SCORE_UPDATE;
			
			LCD_Cursor(16);
			LCD_WriteData(p2SCORE + '0');
			
			if(p1SCORE >= 3 || p2SCORE >= 3){
				SCORE_State = SCORE_WINNER;
				if(p1SCORE >= 3){
					matrixDisplay(0,0);
					LCD_ClearScreen();
					LCD_Cursor(1);
					LCD_DisplayString(1, "Player 1 Wins!!!");
					//LCD_DisplayString(17, "Ayaya!! Ayaya!!");
					delay_ms(1991);		//1991 because thats the year the Soviet Union collapsed
					reset2 = 1;
				}
				else if(p2SCORE >= 3){
					matrixDisplay(0,0);
					LCD_ClearScreen();
					LCD_Cursor(1);
					LCD_DisplayString(1, "Player 2 Wins!!!");
					//LCD_DisplayString(17, "Ayaya!! Ayaya!!");
					delay_ms(1991);		//They were their own worst enemy
					reset2 = 1;
				}
				else{
					LCD_ClearScreen();
					LCD_Cursor(1);
					LCD_DisplayString(17, "Ayaya!! Ayaya!!");
				}
			}
			
			
			break;
		case SCORE_UPDATE:
			if (1) {
				SCORE_State = SCORE_ON;
				LCD_Cursor(1);
				LCD_DisplayString(1, "P1:        P2: ");
				LCD_Cursor(5);
				LCD_WriteData(p1SCORE + '0');				LCD_Cursor(16);
				LCD_WriteData(p2SCORE + '0');
				
			}
			
			
			break;
		case SCORE_WINNER:
			break;
		default:
			SCORE_State = SCORE_START;
	} // Transitions

	switch(SCORE_State) { // State actions
		case SCORE_START:
			LCD_ClearScreen();
			break;
		case SCORE_ON:
			break;
		case SCORE_UPDATE:
			break;
		case SCORE_WINNER:
			delay_ms(1000); 
			reset2 = 1;
			break;
		default: // ADD default behaviour below
			break;
	} // State actions

}

//****************************************************//



//******	LED MATRIX DISPLAY STATE MACHINE	******//
enum DISPLAY_States { DISPLAY_s1, DISPLAY_s2, DISPLAY_s3, DISPLAY_s4 } DISPLAY_State;

Display_SM() {
	switch(DISPLAY_State) { // Transitions
		case -1:
		DISPLAY_State = DISPLAY_s1;
		break;
		case DISPLAY_s1:
		if (1) {
			DISPLAY_State = DISPLAY_s2;
			matrixDisplay(paddle1, 0b00000001);
		}
		break;
		case DISPLAY_s2:
		if (1) {
			DISPLAY_State = DISPLAY_s3;
			matrixDisplay(paddle2, 0b10000000);
		}
		break;
		case DISPLAY_s3:
		if (1) {
			DISPLAY_State = DISPLAY_s4;
			matrixDisplay(ballX,ballY);
		}
		break;
		case DISPLAY_s4:
		if (1) {
			DISPLAY_State = DISPLAY_s2;
			matrixDisplay(paddle1, 0b00000001);
		}
		break;
		default:
		DISPLAY_State = DISPLAY_s1;
	} // Transitions
	
	switch(DISPLAY_State) { // State actions
		case DISPLAY_s1:
			break;
		case DISPLAY_s2:
			break;
		case DISPLAY_s3:
			break;
		case DISPLAY_s4:
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	
}

//****************************************************//



//******	PLAYER & PADDLE 1 STATE MACHINE		******//
enum P1_States { P1_NO_PRESS, P1_UP_PRESS, P1_DOWN_PRESS} P1_State;

P1_SM() {
	p1UP = (~PINA) & 0x01;		//P2 is left, P1 is right
	p1DWN = (~PINA) & 0x02;
	switch(P1_State) { // Transitions
		case -1:
			P1_State = P1_NO_PRESS;
			break;
		case P1_NO_PRESS:
			if (p1UP && !p1DWN) {
				P1_State = P1_UP_PRESS;
				if(paddle1 < 0xE0){					//Overflow guard
					paddle1 = paddle1 << 1;			//Moves paddle 1 up
				}
			}
			else if (!p1UP && p1DWN) {
				P1_State = P1_DOWN_PRESS;
				if(paddle1 > 0x07){					//Overflow guard
					paddle1 = paddle1 >> 1;			//Moves paddle 1 down
				}
			}
			break;
		case P1_UP_PRESS:
			if (!p1UP) {
				P1_State = P1_NO_PRESS;
			}
			break;
		case P1_DOWN_PRESS:
			if (!p1DWN) {
				P1_State = P1_NO_PRESS;
			}
			break;
		default:
			P1_State = P1_NO_PRESS;
	} // Transitions

	switch(P1_State) { // State actions
		case P1_NO_PRESS:
			break;
		case P1_UP_PRESS:
			break;
		case P1_DOWN_PRESS:
			break;
		default: // ADD default behaviour below
			break;
	} // State actions

}

//****************************************************//



//******	PLAYER & PADDLE 2 STATE MACHINE		******//
enum P2_States { P2_NO_PRESS, P2_UP_PRESS, P2_DOWN_PRESS} P2_State;
	
P2_SM() {
	p2UP = (~PINA) & 0x04;						//P2 is left, P1 is right
	p2DWN = (~PINA) & 0x08;
	switch(P2_State) { // Transitions
		case -1:
			P2_State = P2_NO_PRESS;
			break;
		case P2_NO_PRESS:
			if (p2UP && !p2DWN) {
				P2_State = P2_UP_PRESS;
				if(paddle2 < 0xE0){					//Overflow guard
					paddle2 = paddle2 << 1;			//Moves paddle 2 up
				}
			}
			else if (!p2UP && p2DWN) {
				P2_State = P2_DOWN_PRESS;
				if(paddle2 > 0x07){					//Overflow guard
					paddle2 = paddle2 >> 1;			//Moves paddle 2 down
				}
			}
			break;
		case P2_UP_PRESS:
			if (!p2UP) {
				P2_State = P2_NO_PRESS;
			}
			break;
		case P2_DOWN_PRESS:
			if (!p2DWN) {
				P2_State = P2_NO_PRESS;
			}
			break;
		default:
		P2_State = P2_NO_PRESS;
	} // Transitions

	switch(P2_State) { // State actions
		case P2_NO_PRESS:
			break;
		case P2_UP_PRESS:
			break;
		case P2_DOWN_PRESS:
			break;
		default: // ADD default behaviour below
			break;
	} // State actions

}
//****************************************************//



//******	AI & PADDLE 2 STATE MACHINE			******//
enum AI_States { AI_NO_PRESS, AI_UP_PRESS, AI_DOWN_PRESS} AI_State;

AI_SM() {
	char move = 0x00;
	//p2UP = (~PINA) & 0x04;						//AI is left, P1 is right
	//p2DWN = (~PINA) & 0x08;
	move = rand() % 6;
	
	if(move >= 3){
		p2UP = 1;
		p2DWN = 0;
	}
	else if(move < 3){
		p2UP = 0;
		p2DWN = 1;
	}
	
	
	switch(AI_State) { // Transitions
		case -1:
		AI_State = AI_NO_PRESS;
		break;
		case AI_NO_PRESS:
		if (p2UP && !p2DWN) {
			AI_State = AI_UP_PRESS;
			if(paddle2 < 0xE0){					//Overflow guard
				paddle2 = paddle2 << 1;			//Moves paddle 2 up
			}
		}
		else if (!p2UP && p2DWN) {
			AI_State = AI_DOWN_PRESS;
			if(paddle2 > 0x07){					//Overflow guard
				paddle2 = paddle2 >> 1;			//Moves paddle 2 down
			}
		}
		break;
		case AI_UP_PRESS:
		if (!p2UP) {
			AI_State = AI_NO_PRESS;
		}
		break;
		case AI_DOWN_PRESS:
		if (!p2DWN) {
			AI_State = AI_NO_PRESS;
		}
		break;
		default:
		AI_State = AI_NO_PRESS;
	} // Transitions

	switch(AI_State) { // State actions
		case AI_NO_PRESS:
		break;
		case AI_UP_PRESS:
		break;
		case AI_DOWN_PRESS:
		break;
		default: // ADD default behaviour below
		break;
	} // State actions

}

//****************************************************//



//******		BALL PHYSICS STATE MACHINE		******//
enum Ball_States { BALL_START, Ball_s2, Ball_s3, Ball_s4, Ball_s5 } Ball_State;

TickFct_Ball_SM() {
	switch(Ball_State) { // Transitions
		case -1:
			Ball_State = BALL_START;
			break;
		case BALL_START:
			if (1) {
				Ball_State = Ball_s2;
			}
			break;
		case Ball_s2:   //logic complete
			if(ballX << 1 > 0x80 && ballY >> 1 == 0x01 && (ballX & paddle1)){  //corner edge
				Ball_State = Ball_s4;
			}
			else if (ballX << 1 > 0x80) {
				Ball_State = Ball_s3;
			}
			else if (ballY >> 1 == 0x01 &&(ballX & paddle1)){
				Ball_State = Ball_s5;
			}
			else if(ballX << 1 & paddle1 && ballY >> 1 == 0x01){				//paddle corner
				Ball_State = Ball_s4;
			}
			else if(ballY <= 0x00){
				Ball_State = BALL_START;
				++p2SCORE;
			}
		
			break;
		case Ball_s3:		//logic complete
			if(ballX >> 1 < 0x01 && ballY >> 1 == 0x01 && (ballX & paddle1)){  //corner edge
				Ball_State = Ball_s5;
			}
			else if (ballX >> 1 < 0x01) {
				Ball_State = Ball_s2;
			}
			else if (ballY >> 1 == 0x01 &&(ballX & paddle1)){
				Ball_State = Ball_s4;
			}
			else if(ballX >> 1 & paddle1 && ballY >> 1 == 0x01){				//paddle corner
				Ball_State = Ball_s5;
			}
			else if(ballY <= 0x00){
				Ball_State = BALL_START;
				++p2SCORE;
			}
			else{
				//Ball_State = BALL_START;
			}
			break;
		case Ball_s4:   //logic complete
			if(ballY << 1 == 0x80 && ballX >>1 < 0x01 && (ballX & paddle2)){					//corner edge
				Ball_State = Ball_s2;
			}
			else if (ballY << 1 == 0x80 && (ballX & paddle2)) {
				Ball_State = Ball_s3;
			}
			else if (ballX >>1 < 0x01) {
				Ball_State = Ball_s5;
			}
			else if(ballX >> 1 & paddle2 && ballY << 1 == 0x80){		//paddle corner
				Ball_State = Ball_s2;
			}
			else if(ballY >= 0x80){
				Ball_State = BALL_START;
				++p1SCORE;
			}
			else{
				//Ball_State = BALL_START;
			}
			break;
		case Ball_s5:		//logic complete
			if(ballY << 1 == 0x80 && ballX <<1 > 0x80 && (ballX & paddle2)){					//corner edge
				Ball_State = Ball_s3;
			}
			else if (ballY << 1 == 0x80 && (ballX & paddle2)) {
				Ball_State = Ball_s2;
			}
			else if (ballX << 1 > 0x80) {
				Ball_State = Ball_s4;
			}
			else if(ballX << 1 & paddle2 && ballY << 1 == 0x80){		//paddle corner
				Ball_State = Ball_s3;
			}
			else if(ballY >= 0x80){
				Ball_State = BALL_START;
				++p1SCORE;
			}
			else{
				//Ball_State = BALL_START;
			}
			break;
		default:
			Ball_State = BALL_START;
	} // Transitions

	switch(Ball_State) { // State actions
		case BALL_START:
			ballX = 0b00001000;
			ballY = 0b00010000;
			break;
		case Ball_s2:
			ballX = ballX << 1;
		
			ballY = ballY >>1;
			break;
		case Ball_s3:
			ballX = ballX >> 1;
			ballY = ballY >> 1;
			break;
		case Ball_s4:
			ballX = ballX >> 1;
		
			ballY = ballY << 1;
			break;
		case Ball_s5:
			ballX = ballX << 1;
			ballY = ballY << 1;
			break;
		default: // ADD default behaviour below
			break;
	} // State actions

}

//****************************************************//



//Void function, resets all global values for new game
void resetAll() {
	reset2 = 0;
	flag = 0;
	matrixDisplay(0,0);
	p1SCORE = 0;
	p2SCORE = 0;
	paddle1 = 0x38;
	paddle2 = 0x38;
	ballX = 0b00001000;
	ballY = 0b00010000;
}




int main(void){
	// Timer period = 1000 ms (1 sec)
	TimerSet(TIMERPERIOD);				

	// Turns timer on
	TimerOn();					
	
	//Local SyncSM timer variables
	int ballCounter = 0;
	int aiCounter = 0;
	int displayLEDCounter = 0;
	int displayMATCounter = 0;
	
	char reset1 = 0; //local reset
	
	//INPUTS
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
	
	//
	//OUTPUTS
	DDRB = 0xFF; PORTB = 0x00; 
	DDRC = 0xFF; PORTC = 0x00; 
	DDRD = 0xFF; PORTD = 0x00;
	
	LCD_init();
	//LCD_DisplayString(1, "Ayaya!! Ayaya!!");
	
	//goto reaches here and restarts the game when button is pressed.
	RESTART:
	resetAll();
	
	//State Machine initializations
	P1_State = P1_NO_PRESS;
	P2_State = P2_NO_PRESS;
	AI_State = AI_NO_PRESS;
	Ball_State = BALL_START;
	MENU_State = MENU_START;
	SCORE_State = SCORE_START;
	DISPLAY_State = -1;
	
	//Locks game in menu state until user chooses an option.
	while(flag == 0){
		Menu();
	}
	 
	
	while(1){
		////Reset Button
		reset1 = (~PINA & 0x10);
		if(reset1 || reset2){
			goto RESTART;
		}
		//////////
		
		
		//Player 1 always live
		P1_SM();
		
		//Affected by Menu_SM, selects single or multiplayer
		if(flag == 1){
			if(aiCounter >= 200){
				AI_SM();
				aiCounter = 0;
			}
		}
		else if(flag == 2){
			P2_SM();
		}
		
		
		//delayed Ball movement
		if(ballCounter >= 300){
			TickFct_Ball_SM();
			ballCounter = 0;
		}
		
		//Display LCD call
		if(displayLEDCounter >= 250){
			Score_SM();
			displayLEDCounter = 0;
			counter = 0;
		}
		
		//Matrix display call
		if(displayMATCounter >= 1){
			Display_SM();
			displayMATCounter = 0;
			counter = 0;
		}
		
		
		

		
		ballCounter += TIMERPERIOD;
		aiCounter += TIMERPERIOD;
		displayLEDCounter += TIMERPERIOD;
		displayMATCounter += TIMERPERIOD;
		
		while(!TimerFlag) {}// Wait 1 sec
		TimerFlag = 0;
	}
	
	return 0;
}