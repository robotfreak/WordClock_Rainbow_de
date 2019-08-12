#include "Rainbow.h"
#include "Arduino.h"
#include "data.h"
#include <avr/pgmspace.h>


//==============================================================

//shift 1 bit to the rgb data driver MBI5168
void shift_1_bit(unsigned char LS)
{
  if(LS)
  {
    shift_data_1;
  }
  else
  {
    shift_data_0;
  }
  clk_rising;
}

//sweep the specific line,used in the Timer1 ISR
void flash_line(unsigned char line,unsigned char level)
{
  disable_oe;
  close_all_line;
  shift_24_bit(line,level);
  open_line(line);
  enable_oe;
}

//one line with 8 rgb data,so totally 8x3=24 bits. Data format:0x0bgr
void shift_24_bit(unsigned char line,unsigned char level)
{
  unsigned char column=0;
  unsigned char g=0,r=0,b=0;
  le_high;

  //Output G0~G8
  for(column=0;column<8;column++)
  {
    g=(matrixColorData[line][column]&0x00FF)>>4;

    if(g>level) {
      shift_1_bit(1);
    }
    else {
      shift_1_bit(0);
    }  //gray scale,11100000b always light

  }
  //Output R0~R8
  for(column=0;column<8;column++)
  {
    r=matrixColorData[line][column]&0x000f;

    if(r>level) {
      shift_1_bit(1);
    }
    else {
      shift_1_bit(0);
    }  //gray scale,00011000 always light

  }
  //Output B0~B8
  for(column=0;column<8;column++)
  {
    b=(matrixColorData[line][column]&0x0fff)>>8;

    if(b>level) {
      shift_1_bit(1);
    }
    else {
      shift_1_bit(0);
    }  //gray scale,00000111 always light

  }

  le_low;
}

//open the specific line
void open_line(unsigned char line)
{
  switch(line)
  {
  case 0:
    {
      open_line0;
      break;
    }
  case 1:
    {
      open_line1;
      break;
    }
  case 2:
    {
      open_line2;
      break;
    }
  case 3:
    {
      open_line3;
      break;
    }
  case 4:
    {
      open_line4;
      break;
    }
  case 5:
    {
      open_line5;
      break;
    }
  case 6:
    {
      open_line6;
      break;
    }
  case 7:
    {
      open_line7;
      break;
    }
  }
}
//======================================================================
Rainbow::Rainbow(){
}

//invoke initIO and initTimer1
void Rainbow::init(void)
{
  initIO();
  initTimer1();
}

//initialize IO for controlling the leds
void Rainbow::initIO(void)
{
  DDRD=0xff;//set port D as output
  DDRC=0xff;//set port C as output
  DDRB=0xff;//set port B as output
  PORTD=0;//initialize port D to LOW
  PORTB=0;//initialize port B to LOW
}

//initialize Timer1 to 100us overflow           
void Rainbow::initTimer1(void)        
{
  TCCR1A = 0;                 // clear control register A
  TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer
  ICR1=800;                   //(XTAL * microseconds) / 2000000  100us cycles 
  TIMSK1 = _BV(TOIE1);
  TCNT1 = 0;
  TCCR1B |= _BV(CS10);
  sei();                      //enable global interrupt
}

//close all leds
void Rainbow::closeAll()
{
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
      matrixColorData[i][j] = 0;
}

//close one line
void Rainbow::closeOneLine(unsigned char line)
{
  for(int j = 0; j < 8; j++)
    matrixColorData[line][j] = 0;
}

//close one column
void Rainbow::closeOneColumn(unsigned char column)
{
  for(int i = 0; i < 8; i++)
    matrixColorData[i][column] = 0;
}

//close specific dot
void Rainbow::closeOneDot(unsigned char line, unsigned char column)
{
  matrixColorData[line][column] = 0;
}

//close one diagonal line
void Rainbow::closeOneDiagonal(unsigned char line, unsigned char type)
{
  int num = 0;//number of lighting leds     

  if(LEFT_BOTTOM_TO_RIGHT_TOP == type)  
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][k+8-num] = 0;
      }
    }
    else//line = 0...7
    {
      num = line + 1;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-k-1][k] = 0;
      }
    }
  }
  else if(LEFT_TOP_TO_RIGHT_BOTTOM == type)
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-1-k][7-k] = 0;
      }
    }
    else//line = 0...7
    {
      num = line + 1;

      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][num-1-k] = 0;
      }
    }
  }
}


//light all with one color
void Rainbow::lightAll(unsigned short colorData)
{
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
      matrixColorData[i][j] = colorData;
}

//light all with matrix data
void Rainbow::lightAll(unsigned short colorData[8][8])
{
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
      matrixColorData[i][j] = colorData[i][j];
}

//light all with matrix data
void Rainbow::lightAll(unsigned int colorData[64])
{
  
  for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
      matrixColorData[i][j] = Color16BitRGB(colorData[i*8+j]);
}

//only light one line with one color
void Rainbow::lightOneLine(unsigned char line, unsigned short color,unsigned char othersState)
{
  if( OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }
  for(int k = 0; k < 8; k++)
    matrixColorData[line][k] = color;
}
//only light one line with 8 colors
void Rainbow::lightOneLine(unsigned char line, unsigned short color[8],unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  for(int k = 0; k < 8; k++)
    matrixColorData[line][k] = color[k];
}

//only light one column with one color
void Rainbow::lightOneColumn(unsigned char column, unsigned short color,unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  for(int k = 0; k < 8; k++)
    matrixColorData[k][column] = color;  
}

//only light one column with 8 colors
void Rainbow::lightOneColumn(unsigned char column, unsigned short color[8],unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  for(int k = 0; k < 8; k++)
    matrixColorData[k][column] = color[k];  

}

//only light one column with serialColorData 8 colors
void Rainbow::lightOneColumn(unsigned char column, unsigned short color[8][8],unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  for(int k = 0; k < 8; k++)
    matrixColorData[k][column] = color[k][column];  
}

//only light one dot at specific position
void Rainbow::lightOneDot(unsigned char line,unsigned char column, unsigned short color,unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  matrixColorData[line][column] = color;
}

unsigned short Rainbow::Color16BitRGB(unsigned int RGBcol)
{
  unsigned short col, r,g,b;
  r  = (unsigned short)(((RGBcol & 0x00FF0000) >> 20) & 0x000F);
  g += (unsigned short)(((RGBcol & 0x0000FF00) >> 12) & 0x000F);
  b += (unsigned short)(((RGBcol & 0x000000FF) >>  4) & 0x000F);
  col = (unsigned short) (r + (g << 4) + (b << 8)); 
  return col; 
}

//only light one dot at specific position
void Rainbow::lightOneDot(unsigned char line,unsigned char column, unsigned int color)
{
  matrixColorData[line][column] = Color16BitRGB(color);
}

//only light one diagonal line  with one color
void Rainbow::lightOneDiagonal(unsigned char line, unsigned char type, unsigned short color,unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  int num = 0;//number of lighting leds     

  if(LEFT_BOTTOM_TO_RIGHT_TOP == type)  
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][k+8-num] = color;
      }
    }
    else//line = 0...7
    {
      num = line + 1;

      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-k-1][k] = color;
      }
    }
  }
  else if(LEFT_TOP_TO_RIGHT_BOTTOM == type)
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-1-k][7-k] = color;
      }
    }
    else//line = 0...7
    {
      num = line + 1;

      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][num-1-k] = color;
      }
    }
  }
}

//only light one diagonal line with a number of colors
void Rainbow::lightOneDiagonal(unsigned char line, unsigned char type, unsigned short *color,unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  int num = 0;//number of lighting leds     

  if(LEFT_BOTTOM_TO_RIGHT_TOP == type)  
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][k+8-num] = color[k];
      }
    }
    else//line = 0...7
    {
      num = line + 1;

      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-k-1][k] = color[k];
      }
    }
  }
  else if(LEFT_TOP_TO_RIGHT_BOTTOM == type)
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-1-k][7-k] = color[k];
      }
    }
    else//line = 0...7
    {
      num = line + 1;

      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][num-1-k] = color[k];
      }
    }
  }

}

//only light one diagonal line with serialColorData colors
void Rainbow::lightOneDiagonal(unsigned char line, unsigned char type, unsigned short color[8][8],unsigned char othersState)
{
  if(OTHERS_OFF == othersState)
  {
    for(int i = 0; i < 8; i++)
      for(int j = 0; j < 8; j++)    
        matrixColorData[i][j] = 0;
  }

  int num = 0;//number of lighting leds     

  if(LEFT_BOTTOM_TO_RIGHT_TOP == type)  
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][k+8-num] = color[7-k][k+8-num];
      }
    }
    else//line = 0...7
    {
      num = line + 1;

      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-k-1][k] = color[num-k-1][k];
      }
    }
  }
  else if(LEFT_TOP_TO_RIGHT_BOTTOM == type)
  {
    if(line&0x08)//line = 8...15
    {
      num = 15 - line;
      for(int k = 0; k < num; k++)
      {
        matrixColorData[num-1-k][7-k] = color[num-1-k][7-k];
      }
    }
    else//line = 0...7
    {
      num = line + 1;

      for(int k = 0; k < num; k++)
      {
        matrixColorData[7-k][num-1-k] = color[7-k][num-1-k];
      }
    }
  }

}

//shift pic 
void Rainbow::shiftPic(unsigned char shift,unsigned short colorData[8][8])
{
  int i,j;
  unsigned char shiftDirection = shift>>6;//high 2 bits repersent shift direction:0-left,1-right,2-up,3-down
  unsigned char offset = shift&0x0F;//low 4 bits repersent shift offset
  //Serial.println(shiftDirection,DEC);
  if(offset > 8)//offset should be no more than 8
      return;

  switch (shiftDirection){
  case LEFT:
    {//shift left
      for(i = 0; i < 8; i++){
        for(j = 0; j < (8-offset); j++){
          matrixColorData[i][j] = colorData[i][j+offset];
        }
        for(j = (8-offset); j < 8; j++){
          matrixColorData[i][j] = 0;
        }
      }
      break;
    }
  case RIGHT:
    {//shift right
      for(int i = 0; i < 8; i++){
        for(j = 0; j < offset; j++){
          matrixColorData[i][j] = 0;
        }
        for(j = offset; j < 8; j++){
          matrixColorData[i][j] = colorData[i][j-offset];
        }
      }
      break;
    }
  case UP:
    {//shift up
      for(int i = 0; i < (8-offset) ; i++){
        for(j = 0; j < 8; j++){
          matrixColorData[i][j] = colorData[i+offset][j];
        }
      }
      for(int i = (8-offset); i < 8 ; i++){
        for(j = 0; j < 8; j++){
          matrixColorData[i][j] = 0;
        }
      }
      break;
    }
  case DOWN:
    {//shift down
      for(int i = 0; i < offset; i++){
        for(j = 0; j < 8; j++){
          matrixColorData[i][j] = 0;
        }
      }
      for(int i = offset; i < 8 ; i++){
        for(j = 0; j < 8; j++){
          matrixColorData[i][j] = colorData[i-offset][j];
        }
      }
      break;
    }
  default:
    break;
  }
}
  
//disp picture preset in the flash with specific index and shift position
void Rainbow::dispPresetPic(unsigned char shift,unsigned char index)
{
  int i = 0;
  int j = 0;
  
  //enrich the serialColorData with specific preset matrix color data
  for(i = 0; i < 8; i++){
    for(j = 0; j < 8; j++){
      serialColorData[i][j] = pgm_read_word(&presetMatrixColorData[index][i][j]);
    }
  }
  
  //shift the pic
  shiftPic(shift,serialColorData);
}

//disp character with specific shift position
void Rainbow::dispChar(unsigned char ASCII,unsigned short color,unsigned char shift)
{
  unsigned char index = 0;
  unsigned char bitMap[8] ;
  int i = 0;
  int j = 0;
  
  //get the bitmap of the ASCII
  index = ASCII - 32;//32-> ' '
  for(i = 0; i < 8; i++){
      bitMap[i] = pgm_read_byte(&myFont[index][i]);
    }
 /* if(ASCII >= '0' && ASCII <= '9'){
    index = ASCII - '0';
    for(i = 0; i < 8; i++){
      bitMap[i] = pgm_read_byte(&ASCII_Number[index][i]);
    }
  }
  else{
    if(ASCII >= 'A' && ASCII <= 'Z'){
      index = ASCII - 'A';
    }
    else if(ASCII >= 'a' && ASCII <= 'z'){
      index = ASCII - 'a' + 26;
    }
    for(i = 0; i < 8; i++){
      bitMap[i] = pgm_read_byte(&ASCII_Char[index][i]);
    }
  }
  */
  //enrich the serialColorData with the color data coresponding to the specific ASCII bitmap
  /*unsigned char bitPos = 0x01;
  for(i = 0; i < 8; i++){
    for(j = 0; j < 8; j++){
      if(bitMap[7-i]&(bitPos<<j)){
        serialColorData[i][j] = color;
      }
      else{
        serialColorData[i][j] = 0;
      }
    }
  }
  */
  unsigned char bitPos = 0x01;
  for(j = 0; j < 8; j++){//column first
    for(i = 0; i < 8; i++){//then line
      if(bitMap[j]&(bitPos<<i)){
        serialColorData[i][j] = color;
      }
      else{
        serialColorData[i][j] = 0;
      }
    }
  }
  
  //shift the pic
  shiftPic(shift,serialColorData);
}

//disp specific color
void Rainbow::dispColor(unsigned short color)
{
  lightAll(color);
}
