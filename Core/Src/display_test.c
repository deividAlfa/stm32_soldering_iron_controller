/*
 *  display_test.c
 *
 *  Created on: Jul 1, 2021
 *      Author: David
 */


#include <display_test.h>
#include "main.h"
#include "board.h"
#include "iron.h"
#include "display.h"
#include "gui.h"

#ifdef RUN_DISPLAY_TEST
struct{
  uint32_t tim_fps, tim_move;
  uint16_t fps,last_fps, rate, seconds;
  int8_t rad,x,y,xdir,ydir,run;
}test;


// This is just a test that draws a bouncing ball and updates the screen as fast as possible
// To measure display performance
void display_test(void){
  test.tim_fps = test.tim_move = HAL_GetTick();
  test.rad=12;
  test.x=63+32;
  test.y=test.rad+1;
  test.xdir=1;
  test.ydir=1;
  test.rate=20;

  //#ifndef DEBUG
  test.run=1;
  //#endif

  char str[16];
  setDisplayContrastOrBrightness(255);
  setDisplayVcom(0xff);
  setDisplayPrecharge(0x0f);
  fillBuffer(BLACK, fill_dma);
  u8g2_SetFont(&u8g2,u8g2_font_menu );
  u8g2_SetDrawColor(&u8g2, WHITE);
  if(oled.use_sw){
    u8g2_DrawUTF8(&u8g2,0,0,"SW Mode");
  }
  else{
    u8g2_DrawUTF8(&u8g2,0,0,"HW Mode");
  }
  u8g2_DrawUTF8(&u8g2,0,48,"FPS:");
  u8g2_DrawUTF8(&u8g2,0,32,"TIM:");
  while(1){
    setIronSafeMode(enable);
    if(oled.status==oled_idle){
      if((HAL_GetTick()-test.tim_fps)>999){
          test.seconds++;
          test.tim_fps=HAL_GetTick();
          u8g2_SetDrawColor(&u8g2, BLACK);
          u8g2_DrawBox(&u8g2, 30, 16, 34, 48);
          u8g2_SetDrawColor(&u8g2, WHITE);
          sprintf(str,"%u", test.fps);
          u8g2_DrawUTF8(&u8g2,30,48,str);
          sprintf(str,"%u", test.seconds);
          u8g2_DrawUTF8(&u8g2,30,32,str);
          test.last_fps = test.fps;
          test.fps=0;
      }
      if((HAL_GetTick()-test.tim_move)>=test.rate){
        test.tim_move=HAL_GetTick();
        u8g2_SetDrawColor(&u8g2, BLACK);
        u8g2_DrawBox(&u8g2, test.x-test.rad, test.y-test.rad, (test.rad*2)+1, (test.rad*2)+1);
        u8g2_SetDrawColor(&u8g2, WHITE);
        if(test.run){
          test.x += test.xdir;
          test.y += test.ydir;
          if(test.x>=(127-(test.rad+1)) || test.x<=(64+test.rad+1)){
            test.xdir = -test.xdir;
          }
          if(test.y>=(63-(test.rad+1))|| test.y<=(test.rad+1)){
            test.ydir = -test.ydir;
          }
        }
        u8g2_DrawDisc(&u8g2, test.x, test.y, test.rad, U8G2_DRAW_ALL);
        u8g2_DrawFrame(&u8g2, 64, 0, 64, 64);
      }
      test.fps++;
      update_display();
    }
  }
}

#endif
