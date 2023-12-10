#include <msp430.h>
#include <stdlib.h>
#include <libTimer.h>
#include "libTimer.h"
#include "lcdutils.h"
#include "buzzer.h"
#include "led.h"
#include "lcddraw.h"


// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!!
#define LED BIT6/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15
int stone = 0;
int sheet = 0;
int mystery = 0;
int cutting = 0;
int score = 0;
int titlescreen = 1;
int highscore = 0;
int lost = 0;
int win = 0;
int tie = 0;
unsigned short colors[] = {COLOR_MAGENTA, COLOR_CYAN, COLOR_PINK, COLOR_PURPLE, COLOR_TURQUOISE, COLOR_ROYAL_BLUE, COLOR_VIOLET, COLOR_HOT_PINK};
unsigned char step = 0;
short drawPos[2] = {1, 10};
short controlPos[2] = {0, 120};
short colVelocity = 1;
short colLimits[2] = {0,127};
static char switch_update_interrupt_sense(){
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);/* if switch down, sense up */
  return p2val;
}

void switch_init(){/* setup switch */
  P2REN |= SWITCHES;/* enables resistors for switches */
  P2IE |= SWITCHES;/* enable interrupts from switches */
  P2OUT |= SWITCHES;/* pull-ups for switches */
  P2DIR &= ~SWITCHES;/* set switches' bits for input */

  switch_update_interrupt_sense();
}

int switches = 0;

void switch_interrupt_handler(){
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}

void rock(){
  if(stone == 1){
    drawString5x7(56, 76, "ROCK", COLOR_WHITE, COLOR_BLACK);
  }
}
void paper(){
  if(sheet == 1){
    drawString5x7(54, 76, "PAPER", COLOR_WHITE, COLOR_BLACK);
  }
}
void scissors(){
  if(cutting == 1){
    drawString5x7(52, 76, "SCISSORS", COLOR_WHITE, COLOR_BLACK);
  }
}
void unknown(){
  if(mystery == 1){
    for(int i = 0; i < 100; i+= 10) fillRectangle(40+i, 0, 1, 8, COLOR_BLACK);
    drawPixel(64, 80, COLOR_PURPLE);
    drawString5x7(40, 0, "?", COLOR_PINK, COLOR_BLACK);
    drawString5x7(46, 0, "?", COLOR_PURPLE, COLOR_BLACK);
    drawString5x7(52, 0, "?", COLOR_WHITE, COLOR_BLACK);
  }
}
void points(int score){
  int line = 40;
  if(score == 0){
    for(int i = 0; i < 100; i+=10) fillRectangle(line+i, 0, 1, 8, COLOR_BLACK);
  }
  if(score >= 1){
    for(score; score > 0; score--){
      fillRectangle(line, 0, 1, 8, COLOR_WHITE);
      fillRectangle(line+1,0, 1, 8, COLOR_WHITE);
      line += 5;
      if(score > highscore){
	highscore = score;
    }
  }
  }
}
void highestscore(int highscore){
  int horizontal = 60;
  for(highscore; highscore > 0; highscore--){
    fillRectangle(horizontal, 141, 1, 8, colors[highscore]);
    fillRectangle(horizontal+1, 141, 1, 8, colors[highscore]);
    horizontal += 5;
  }
}
void gametitle(){
  drawString5x7(0, 0, "SCORE:", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(0, 142, "HIGHSCORE:", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(0, 152, "ROCK", COLOR_RED, COLOR_BLACK);
  drawString5x7(25, 152, "PAPER", COLOR_GREEN, COLOR_BLACK);
  drawString5x7(57, 152, "SCISSORS", COLOR_BLUE, COLOR_BLACK);
  drawString5x7(107, 152, "???", COLOR_PINK, COLOR_BLACK);
}
void title(){
  drawString5x7(40, 0, "WElCOME!", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(15, 20, "THIS GAME IS ROCK", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(25, 30, "PAPER SCISSORS", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(6, 50, "THE GAME WILL START", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(12, 60, "WHEN YOU PRESS ANY", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(25, 70, "SWITCH BELOW", COLOR_WHITE, COLOR_BLACK);
  drawString5x7(0, 90, "S1: ROCK", COLOR_RED, COLOR_BLACK);
  drawString5x7(0, 100, "S2: PAPER", COLOR_GREEN, COLOR_BLACK);
  drawString5x7(0, 110, "S3: SCISSORS", COLOR_BLUE, COLOR_BLACK);
  drawString5x7(0, 120, "S4: ???", COLOR_PINK, COLOR_BLACK);
}
short redrawScreen = 0;
void losebuzz(){
 
}
void wdt_c_handler(){
  static int secCount = 0;
  static int curr_color = 0;
  static int limit = 3;
  if(switches & SW1){
    stone = 1;
    redrawScreen = 1;
    titlescreen = 0;
  }
  if(switches & SW2){
    sheet = 1;
    redrawScreen = 1;
    titlescreen = 0;
  }
  if(switches & SW3){
    cutting = 1;
    redrawScreen = 1;
    titlescreen = 0;
  }
  if(switches & SW4){
    mystery = 1;
    redrawScreen = 1;
    titlescreen = 0;
  }
  if(secCount++ >= 125 && titlescreen > 0){
    secCount = 0;
    curr_color = (curr_color + 1) % 8; 
    fillRectangle(0, 0, 15, 15, colors[curr_color]);
    fillRectangle(112,  144, 15, 15, colors[curr_color]);
    fillRectangle(0, 144, 15, 15, colors[curr_color]);
    fillRectangle(112, 0, 15, 15, colors[curr_color]);
  }
}
void main(){
  P1DIR |= LED;/**< Green led on when CPU on */
  P1OUT &= ~LED_GREEN;
  P1OUT |= LED_RED;
  
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8); /**< GIE (enable interrupts) */
  clearScreen(COLOR_BLACK);
  title();
  while(1){
    if(redrawScreen){
      clearScreen(COLOR_BLACK);
      gametitle();
      if(stone == 1){
	mystery = 0;
	rock();
	stone = 0;
	if(rand() % 3 == 2){
	  score += 1;
	  drawString5x7(0, 10, "YOU WIN!", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "SCISSORS", colors[rand() % 8],COLOR_BLACK);
	  points(score);
	  win = 1;
	}
	else if(rand() % 3 == 0){
	  drawString5x7(0, 10, "TIE", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "ROCK", colors[rand() % 8], COLOR_BLACK);
	  tie = 1;
	}
	else{
	  score = 0;
	  drawString5x7(0, 10, "YOU LOSE", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "PAPER", colors[rand() % 8], COLOR_BLACK);
	  points(score);
	  lost = 1;
	}
      }
      if(sheet == 1){
	mystery = 0;
	paper();
        sheet = 0;
	if(rand() % 3 == 0){
	  score+=1;
	  drawString5x7(0, 10, "YOU WIN!", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "ROCK", colors[rand() % 8], COLOR_BLACK);
	  points(score);
	}
	else if(rand() % 3 == 1){
	  drawString5x7(0, 10, "TIE", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "PAPER", colors[rand() % 8], COLOR_BLACK);
	}
	else{
	  score = 0;
	  drawString5x7(0, 10, "YOU LOSE", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "SCISSORS", colors[rand() % 8], COLOR_BLACK);
	  points(score);
	}
      }
      if(cutting == 1){
	mystery = 0;
	scissors();
	cutting = 0;
	if(rand() % 3 == 1){
	  score+=1;
	  drawString5x7(0, 10, "YOU WIN!", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "PAPER", colors[rand() % 8], COLOR_BLACK);
	  points(score);
	}
        else if(rand() % 3 == 2){
	  drawString5x7(0, 10, "TIE", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "SCISSORS", colors[rand() % 8], COLOR_BLACK);
	}
	else{
	  score = 0;
	  drawString5x7(0, 10, "YOU LOSE", COLOR_WHITE, COLOR_BLACK);
	  drawString5x7(0, 20, "ROCK", colors[rand() % 8], COLOR_BLACK);
	  points(score);
	}
      }
      if(mystery == 1){
	unknown();
      }
      highestscore(highscore);
      redrawScreen = 0;
    }
    P1OUT &= ~LED;
    or_sr(0x10);
    P1OUT |= LED;
  }
}
void __interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;      /* clear pending sw interrupts */
    switch_interrupt_handler();/* single handler for all switches */
  }
}
