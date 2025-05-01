/*
 * LCD_Driver.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Xavion
 */

#include "LCD_Driver.h"
#include <stdio.h>


/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor   = 0xFFFF;
static uint16_t Coin_xpos = 0;
static uint16_t Coin_ypos = 0;
static uint16_t Coin_position[6][7] = {0};
uint8_t this_was_winning_coin = 0;
uint16_t player1_wins = 0;
uint16_t player2_wins = 0;
static uint32_t start_time_value = 0;
static uint32_t end_time_value = 0;

static uint16_t screen3_btn_x1 = 15;
static uint16_t screen3_btn_x2 = 225;
static uint16_t screen3_btn_y1 = 210;
static uint16_t screen3_btn_y2 = 290;


/*
 * fb[y*W+x] OR fb[y][x]
 * Alternatively, we can modify the linker script to have an end address of 20013DFB instead of 2002FFFF, so it does not place variables in the same region as the frame buffer. In this case it is safe to just specify the raw address as frame buffer.
 */
//uint32_t frameBuffer[(LCD_PIXEL_WIDTH*LCD_PIXEL_WIDTH)/2] = {0};		//16bpp pixel format. We can size to uint32. this ensures 32 bit alignment


//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] = {0};			//16bpp pixel format.


void LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIO Config
   *
    LCD pins
   LCD_TFT R2 <-> PC.10
   LCD_TFT G2 <-> PA.06
   LCD_TFT B2 <-> PD.06
   LCD_TFT R3 <-> PB.00
   LCD_TFT G3 <-> PG.10
   LCD_TFT B3 <-> PG.11
   LCD_TFT R4 <-> PA.11
   LCD_TFT G4 <-> PB.10
   LCD_TFT B4 <-> PG.12
   LCD_TFT R5 <-> PA.12
   LCD_TFT G5 <-> PB.11
   LCD_TFT B5 <-> PA.03
   LCD_TFT R6 <-> PB.01
   LCD_TFT G6 <-> PC.07
   LCD_TFT B6 <-> PB.08
   LCD_TFT R7 <-> PG.06
   LCD_TFT G7 <-> PD.03
   LCD_TFT B7 <-> PB.09
   LCD_TFT HSYNC <-> PC.06
   LCDTFT VSYNC <->  PA.04
   LCD_TFT CLK   <-> PG.07
   LCD_TFT DE   <->  PF.10
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

 /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

void LTCD_Layer_Init(uint8_t LayerIndex)
{
	LTDC_LayerCfgTypeDef  pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;	//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;	//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1;  //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0){
		pLayerCfg.FBStartAdress = (uintptr_t)frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK)
	{
		LCD_Error_Handler();
	}
}

void clearScreen(void)
{
  LCD_Clear(0,LCD_COLOR_WHITE);
}

void LTCD__Init(void)
{
	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	 {
	   LCD_Error_Handler();
	 }

	ili9341_Init();
}

/* START Draw functions */


/*
 * This is really the only function needed.
 * All drawing consists of is manipulating the array.
 * Adding input sanitation should probably be done.
 */
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	frameBuffer[y*LCD_PIXEL_WIDTH+x] = color;  //You cannot do x*y to set the pixel.
}

/*
 * These functions are simple examples. Most computer graphics like OpenGl and stm's graphics library use a state machine. Where you first call some function like SetColor(color), SetPosition(x,y), then DrawSqure(size)
 * Instead all of these are explicit where color, size, and position are passed in.
 * There is tons of ways to handle drawing. I dont think it matters too much.
 */
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint16_t color)
{
    for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
    Coin_xpos = Xpos;
    Coin_ypos = Ypos;
}

void LCD_Clear_Circle()
{
	uint16_t Xpos = Coin_xpos;
	uint16_t Ypos = Coin_ypos;
	uint16_t radius = 13;
	uint16_t color = LCD_COLOR_WHITE;
    for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
}

void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(x, i+y, color);
  }
}

void LCD_Draw_Horizontal_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(i+x, y, color);
  }
}

void LCD_Clear(uint8_t LayerIndex, uint16_t Color)
{
	if (LayerIndex == 0){
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++){
			frameBuffer[i] = Color;
		}
	}
  // TODO: Add more Layers if needed
}

//This was taken and adapted from stm32's mcu code
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

//This was taken and adapted from stm32's mcu code
void LCD_SetFont(FONT_t *fonts)
{
  LCD_Currentfonts = fonts;
}

//This was taken and adapted from stm32's mcu code
void LCD_Draw_Char(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) && (LCD_Currentfonts->Width <= 12)) || (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
         //Background If want to overwrite text under then add a set color here
      }
      else
      {
    	  LCD_Draw_Pixel(counter + Xpos,index + Ypos,CurrentTextColor);
      }
    }
  }
}

//This was taken and adapted from stm32's mcu code
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_Draw_Char(Xpos, Ypos, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void visualDemo(void)
{
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
		for(x=0; x < LCD_PIXEL_WIDTH; x++){
			if (x & 32)
				frameBuffer[x*y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x*y] = LCD_COLOR_BLACK;
		}
	}

	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_GREEN);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_RED);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Draw_Vertical_Line(10,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Vertical_Line(230,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);

	//LCD_Draw_Circle_Fill(125,150,20,LCD_COLOR_BLACK);
	LCD_Draw_Circle_Fill(10,200,10,LCD_COLOR_BLACK);
	HAL_Delay(2000);

	LCD_Clear(0,LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(100,140,'H');
	LCD_DisplayChar(115,140,'e');
	LCD_DisplayChar(125,140,'l');
	LCD_DisplayChar(130,140,'l');
	LCD_DisplayChar(140,140,'o');

	LCD_DisplayChar(100,160,'W');
	LCD_DisplayChar(115,160,'o');
	LCD_DisplayChar(125,160,'r');
	LCD_DisplayChar(130,160,'l');
	LCD_DisplayChar(140,160,'d');
}

void LCD_Screen1(void)
{
	uint16_t char_width = 15;
	uint16_t player1_xpos = 60;
	uint16_t player1_ypos = LCD_PIXEL_HEIGHT/4;
	uint16_t player2_xpos = 60;
	uint16_t player2_ypos = LCD_PIXEL_HEIGHT*3/4;

	LCD_Draw_Horizontal_Line(0,LCD_PIXEL_HEIGHT/2,LCD_PIXEL_WIDTH,LCD_COLOR_MAGENTA);

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(player1_xpos,player1_ypos,'1');
	LCD_DisplayChar(player1_xpos+char_width,player1_ypos,'-');
	LCD_DisplayChar(player1_xpos+2*char_width,player1_ypos,'P');
	LCD_DisplayChar(player1_xpos+3*char_width,player1_ypos,'l');
	LCD_DisplayChar(player1_xpos+4*char_width,player1_ypos,'a');
	LCD_DisplayChar(player1_xpos+5*char_width,player1_ypos,'y');
	LCD_DisplayChar(player1_xpos+6*char_width,player1_ypos,'e');
	LCD_DisplayChar(player1_xpos+7*char_width,player1_ypos,'r');

	LCD_DisplayChar(player2_xpos,player2_ypos,'2');
	LCD_DisplayChar(player2_xpos+char_width,player2_ypos,'-');
	LCD_DisplayChar(player2_xpos+2*char_width,player2_ypos,'P');
	LCD_DisplayChar(player2_xpos+3*char_width,player2_ypos,'l');
	LCD_DisplayChar(player2_xpos+4*char_width,player2_ypos,'a');
	LCD_DisplayChar(player2_xpos+5*char_width,player2_ypos,'y');
	LCD_DisplayChar(player2_xpos+6*char_width,player2_ypos,'e');
	LCD_DisplayChar(player2_xpos+7*char_width,player2_ypos,'r');

	//screen1flag = 1;
	HAL_Delay(500);
}

void LCD_Write_1_or_2_Player(uint8_t player_mode_1_or_2)
{
	uint16_t char_width = 15;
	uint16_t write_xpos = 60;
	uint16_t write_ypos = LCD_PIXEL_HEIGHT/2;

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	if(player_mode_1_or_2 == 1)
	{
		LCD_DisplayChar(write_xpos,write_ypos,'1');
		LCD_DisplayChar(write_xpos+char_width,write_ypos,'-');
		LCD_DisplayChar(write_xpos+2*char_width,write_ypos,'P');
		LCD_DisplayChar(write_xpos+3*char_width,write_ypos,'l');
		LCD_DisplayChar(write_xpos+4*char_width,write_ypos,'a');
		LCD_DisplayChar(write_xpos+5*char_width,write_ypos,'y');
		LCD_DisplayChar(write_xpos+6*char_width,write_ypos,'e');
		LCD_DisplayChar(write_xpos+7*char_width,write_ypos,'r');
	}

	if(player_mode_1_or_2 == 2)
	{
		LCD_DisplayChar(write_xpos,write_ypos,'2');
		LCD_DisplayChar(write_xpos+char_width,write_ypos,'-');
		LCD_DisplayChar(write_xpos+2*char_width,write_ypos,'P');
		LCD_DisplayChar(write_xpos+3*char_width,write_ypos,'l');
		LCD_DisplayChar(write_xpos+4*char_width,write_ypos,'a');
		LCD_DisplayChar(write_xpos+5*char_width,write_ypos,'y');
		LCD_DisplayChar(write_xpos+6*char_width,write_ypos,'e');
		LCD_DisplayChar(write_xpos+7*char_width,write_ypos,'r');
	}

	//screen1flag = 1;
	HAL_Delay(1000);
}

void LCD_Write_Scores()
{
	uint16_t char_width = 15;
	uint16_t write_xpos = 90 + 40;
	uint16_t write_ypos_player1 = 40;
	uint16_t write_ypos_player2 = 80;
	char number_buffer[20];
	uint16_t num_int_chars = 0, i = 0;

	LCD_Draw_Circle_Fill(write_xpos-40, write_ypos_player1+10, 13, LCD_COLOR_RED);
	LCD_Draw_Circle_Fill(write_xpos-40, write_ypos_player2+10, 13, LCD_COLOR_BLUE);

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	num_int_chars = sprintf(number_buffer, "%d", player1_wins);
	for (i=0; i < num_int_chars; i++)
		LCD_DisplayChar(write_xpos+i*char_width,write_ypos_player1, number_buffer[i]);

	num_int_chars = sprintf(number_buffer, "%d", player2_wins);
	for (i=0; i < num_int_chars; i++)
		LCD_DisplayChar(write_xpos+i*char_width,write_ypos_player2, number_buffer[i]);
}


void LCD_PlayBoard_SetUp(void)
{
	//border
	LCD_Draw_Vertical_Line(15,20,280,LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(225,20,280,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(15,20,210,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(15,300,210,LCD_COLOR_BLACK);

	//vertical lines, 7 columns, 320 pixels, 6 lines draw
	LCD_Draw_Vertical_Line(45,60,240,LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(75,60,240,LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(105,60,240,LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(135,60,240,LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(165,60,240,LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(195,60,240,LCD_COLOR_BLACK);

	//horizontal lines
	LCD_Draw_Horizontal_Line(15,60,210,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(15,100,210,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(15,140,210,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(15,180,210,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(15,220,210,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(15,260,210,LCD_COLOR_BLACK);

	//first coin
	LCD_Draw_Circle_Fill(120,40,13,LCD_COLOR_RED);
	Coin_xpos = 120;
	Coin_ypos = 40;
	start_time_value = HAL_GetTick();
}

void LCD_Write_Elapsed_Time(double elapsed_time_seconds)
{
	uint16_t char_width = 8;
	uint16_t write_xpos = 40;
	uint16_t write_ypos = 140;
	char write_time_buf[128];
	uint16_t num_int_chars = 0, i = 0;

	/*
	if (elapsed_time_seconds > 60)
	{
		long minutes_elapsed = (elapsed_time_seconds/60.0);
		long seconds_elapsed = elapsed_time_seconds - minutes_elapsed*60.0;
		num_int_chars = sprintf(write_time_buf, "Last game time: %d m %d s", (int) minutes_elapsed, (int) seconds_elapsed);
	}
	else
		num_int_chars = sprintf(write_time_buf, "Last game time: %d s", (int) elapsed_time_seconds); */

	num_int_chars = sprintf(write_time_buf, "Last game time: %d s", (int) elapsed_time_seconds);


	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font12x12);

	for (i=0; i < num_int_chars; i++)
		LCD_DisplayChar(write_xpos+i*char_width,write_ypos, write_time_buf[i]);
}

void LCD_Write_Game_Over(uint16_t player)
{
	uint16_t char_width = 12;
	uint16_t write_xpos = 15;
	uint16_t write_ypos = 20;
	char write_buf[128];
	uint16_t num_int_chars = 0, i = 0;

	num_int_chars = sprintf(write_buf, "!---Game-Over---!");


	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	for (i=0; i < num_int_chars; i++)
		LCD_DisplayChar(write_xpos+i*char_width,write_ypos, write_buf[i]);
}

void LCD_Draw_Button(void)
{
	uint16_t char_width = 15;
	uint16_t write_xpos = 45;
	uint16_t write_ypos = 240;
	char write_buf[128];
	uint16_t num_int_chars = 0, i = 0;

	uint16_t lenx = screen3_btn_x2 - screen3_btn_x1;
	uint16_t leny = screen3_btn_y2 - screen3_btn_y1;

	LCD_Draw_Vertical_Line(screen3_btn_x1,screen3_btn_y1,leny,LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(screen3_btn_x2,screen3_btn_y1,leny,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(screen3_btn_x1,screen3_btn_y1,lenx,LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(screen3_btn_x1,screen3_btn_y2,lenx,LCD_COLOR_BLACK);

	uint16_t color = LCD_COLOR_YELLOW;
    for(uint16_t y=screen3_btn_y1+1; y<screen3_btn_y2; y++)
    {
        for(uint16_t x=screen3_btn_x1+1; x<screen3_btn_x2; x++)
        {
			LCD_Draw_Pixel(x, y, color);
        }
    }

	num_int_chars = sprintf(write_buf, "Play Again");


	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	for (i=0; i < num_int_chars; i++)
		LCD_DisplayChar(write_xpos+i*char_width,write_ypos, write_buf[i]);

}

uint8_t LCD_Is_Within_Screen3_Button(uint16_t x, uint16_t y)
{
	if((x >= screen3_btn_x1) && (y >= screen3_btn_y1) && (x <= screen3_btn_x2) && (y <= screen3_btn_y2))
		return 1;
	else
		return 0;
}

void LCD_ResultsBoard_SetUp(void)
{
	uint32_t elapsed_time_milliseconds = 0;
	double elapsed_time_seconds = 0;
	LCD_Clear(0, LCD_COLOR_WHITE);
	// Display score
	LCD_Write_Scores();
	// Time to play last round
	if (end_time_value >= start_time_value)
		elapsed_time_milliseconds = (end_time_value - start_time_value);
	elapsed_time_seconds = ((double)elapsed_time_milliseconds)/1000.0;
	LCD_Write_Elapsed_Time(elapsed_time_seconds);

	// Button to restart another round
	LCD_Draw_Button();
}

void LCD_MoveCoin_Right(uint16_t player)
{
	if(Coin_xpos >= 30 && Coin_xpos <= 180)
	{
		LCD_Clear_Circle();
		uint16_t newx = Coin_xpos + 30;
		//change color based on player_turn
		//make macro for radius size
		if(player == 1)
			LCD_Draw_Circle_Fill(newx, Coin_ypos, 13, LCD_COLOR_RED);
		if(player == 2)
			LCD_Draw_Circle_Fill(newx, Coin_ypos, 13, LCD_COLOR_BLUE);
	}
}

void LCD_MoveCoin_Left(uint16_t player)
{
	if(Coin_xpos >= 60 && Coin_xpos <= 210)
	{
		LCD_Clear_Circle();
		uint16_t newx = Coin_xpos - 30;
		//change color based on player_turn
		//make macro for radius size
		if(player == 1)
			LCD_Draw_Circle_Fill(newx, Coin_ypos, 13, LCD_COLOR_RED);
		if(player == 2)
			LCD_Draw_Circle_Fill(newx, Coin_ypos, 13, LCD_COLOR_BLUE);
	}
}

void LCD_MoveCoin_Up(void)
{
	//not needed
}

uint8_t LCD_MoveCoin_Down(uint16_t *player_input)
{
	uint16_t player = *player_input;
	//pixels go by 30, dividing by 30 and subtracting 1 will give correct index for array for column chosen
	uint16_t column = Coin_xpos / 30 - 1;
	uint16_t newy = 0;

	for(uint16_t row = 0; row < NUM_C4_ROWS; row++)
	{
		if(Coin_position[row][column] == 0)
		{
			//CHECK IF GOING TO BOTTOM
				//IF NOT CHECK PIXEL SIZE
			//pixels on board go from 320 to 0 at top

			//Coin_position[row][column] = player_turn;
			Coin_position[row][column] = player;
			newy = 320 - (row+1)*40;
			LCD_Clear_Circle();
			if(player == 1)
			{
				// grow the circle and delay, so the players can see what is happening
				LCD_Draw_Circle_Fill(Coin_xpos, newy, 5, LCD_COLOR_RED);
				HAL_Delay(500);
				LCD_Draw_Circle_Fill(Coin_xpos, newy, 8, LCD_COLOR_RED);
				HAL_Delay(500);
				LCD_Draw_Circle_Fill(Coin_xpos, newy, 13, LCD_COLOR_RED);
			}
			if(player == 2)
			{
				// grow the circle and delay, so the players can see what is happening
				LCD_Draw_Circle_Fill(Coin_xpos, newy, 5, LCD_COLOR_BLUE);
				HAL_Delay(500);
				LCD_Draw_Circle_Fill(Coin_xpos, newy, 8, LCD_COLOR_BLUE);
				HAL_Delay(500);
				LCD_Draw_Circle_Fill(Coin_xpos, newy, 13, LCD_COLOR_BLUE);
			}

			// Check winning condition
			uint16_t highest_connect = 0;
			this_was_winning_coin = CheckWinCondition(player, row, column, &highest_connect);

			// Draw the next coin
			if (this_was_winning_coin == 0)
			{
				//switch player_turn flag?
				//reset coin position markers
				Coin_xpos = 120;
				Coin_ypos = 40;
				// switch players
				if(player == 1)
				{
					*player_input = 2;
					// draw new coin for next play
					LCD_Draw_Circle_Fill(Coin_xpos, Coin_ypos, 13, LCD_COLOR_BLUE);
				}
				else
				{
					*player_input = 1;
					// draw new coin for next play
					LCD_Draw_Circle_Fill(Coin_xpos, Coin_ypos, 13, LCD_COLOR_RED);
				}
			}
			else if (1 == Game_is_draw())
			{
				end_time_value = HAL_GetTick();
				LCD_Write_Game_Over(player);
				HAL_Delay(2000);
				Reset_Coin_Matrix();
				LCD_ResultsBoard_SetUp();
				return 1;
			}
			else
			{
				if (player == 1)
					player1_wins++;
				if (player == 2)
					player2_wins++;
				end_time_value = HAL_GetTick();
				LCD_Write_Game_Over(player);
				HAL_Delay(2000);
				Reset_Coin_Matrix();
				LCD_ResultsBoard_SetUp();
				return 1;
			}

			break;
		}
	}
	return 0;
}

uint8_t Game_is_draw(void)
{
	uint8_t blank_spot_exists = 0;

	for(uint16_t row = 0; row < NUM_C4_ROWS; row++)
	{
		for(uint16_t column = 0; column < NUM_C4_COLS; column++)
		{
			if (Coin_position[row][column] == 0)
				{
					blank_spot_exists = 1;
					break;
				}
		}
	}

	if(blank_spot_exists == 1)
		return 0;
	else
		return 1;
}

void LCD_Move_Coin_to_AI_Column(void)
{
	// AI is player 2
	// First check if player 2 can win with Connect 4
	uint16_t row_pos = 0, col_pos = 0;
	// int first_empty_slot [NUM_C4_COLS] = {0};
	uint16_t highest_connect_offense_num = 0;
	uint16_t highest_connect_defense_num = 0;
	uint16_t highest_connect_offense_col = 0;
	uint16_t highest_connect_defense_col = 0;
	uint16_t highest_connect = 0;

	for(col_pos = 0; col_pos < NUM_C4_COLS; col_pos++)
	{
		// first_empty_slot[col_pos] = -1;
		for(row_pos = 0; row_pos < NUM_C4_ROWS; row_pos++)
		{
			if(Coin_position[row_pos][col_pos] == 0)
			{
				// first_empty_slot[col_pos] = row_pos;
				// offense for AI is player 2 adding a coin
				CheckWinCondition(2, row_pos, col_pos, &highest_connect);
				if (highest_connect > highest_connect_offense_num)
				{
					highest_connect_offense_num = highest_connect;
					highest_connect_offense_col = col_pos;
				}
				// defense is player 1 adding the next coin in an empty slot
				CheckWinCondition(1, row_pos, col_pos, &highest_connect);
				if (highest_connect > highest_connect_defense_num)
				{
					highest_connect_defense_num = highest_connect;
					highest_connect_defense_col = col_pos;
				}
				break;
			}
		}
	}
	// coin was placed prior to AI call, get current column
	uint16_t current_column = Coin_xpos / 30 - 1;
	uint16_t move_to_col = 0;

	if(highest_connect_offense_num >= highest_connect_defense_num)
		move_to_col = highest_connect_offense_col;
	else
		move_to_col = highest_connect_defense_col;

	// Move the coin, as would be if there was a player 2
	if(move_to_col > current_column)
	{
		uint16_t move_right_by = move_to_col - current_column;
		for (uint16_t i = 0; i < move_right_by; i++)
			LCD_MoveCoin_Right(2);
	}
	if(move_to_col < current_column)
	{
		uint16_t move_left_by =  current_column - move_to_col;
		for (uint16_t i = 0; i < move_left_by; i++)
			LCD_MoveCoin_Left(2);
	}
}

uint8_t CheckWinCondition(uint16_t player, uint16_t row_pos, uint16_t col_pos, uint16_t *highest_connect)
{
	// Returns:
	//          0 = no connect 4
	//          1 = vertical connect 4
	//          2 = horizontal connect 4
	//          3 = right diagonal connect 4
	//          4 = left diagonal connect 4

	int i = 0; // y direction or row counter
	int j = 0; // x direction or column counter

	*highest_connect = 0;

	// Check up-down directions
	uint16_t upcount = 0;
	uint16_t downcount = 0;

	// Up direction
	for (i = ((int)(row_pos)+1); i < NUM_C4_ROWS; i++)
	{
		if(Coin_position[i][col_pos] == player)
			upcount++;
		else
			break;
	}

	// Down direction
	for (i = ((int)(row_pos)-1); i >= 0; i--)
	{
		if(Coin_position[i][col_pos] == player)
			downcount++;
		else
			break;
	}

	*highest_connect = (upcount+downcount+1);

	// up, down, and count current coin itself
	if ((upcount+downcount+1) >= 4)
		return 1;

	// Check left-right directions
	uint16_t leftcount = 0;
	uint16_t rightcount = 0;
	// Right direction
	for (j = ((int)(col_pos)+1); j < NUM_C4_COLS; j++)
	{
		if(Coin_position[row_pos][j] == player)
			rightcount++;
		else
			break;
	}

	// Left direction
	for (j = ((int)(col_pos)-1); j >= 0; j--)
	{
		if(Coin_position[row_pos][j] == player)
			leftcount++;
		else
			break;
	}

	if((*highest_connect) < (rightcount+leftcount+1))
		*highest_connect = (rightcount+leftcount+1);

	// up, down, and count current coin itself
	if ((rightcount+leftcount+1) >= 4)
		return 2;

	// Check right diagonal up and left diag down directions
	// Check left-right directions
	uint16_t rdiag_upcount = 0;
	uint16_t ldiag_downcount = 0;
	// Right direction
	for (i = ((int)(row_pos)+1), j = ((int)(col_pos)+1); (i < NUM_C4_ROWS)&&(j < NUM_C4_COLS); i++, j++)
	{
		if(Coin_position[i][j] == player)
			rdiag_upcount++;
		else
			break;
	}

	// Left direction
	for (i = ((int)(row_pos)-1), j = ((int)(col_pos)-1); (i >= 0)&&(j >= 0); i--, j--)
	{
		if(Coin_position[i][j] == player)
			ldiag_downcount++;
		else
			break;
	}

	if((*highest_connect) < (rdiag_upcount+ldiag_downcount+1))
		*highest_connect = (rdiag_upcount+ldiag_downcount+1);

	// up, down, and count current coin itself
	if ((rdiag_upcount+ldiag_downcount+1) >= 4)
		return 3;

	// Check right diag down and left diagonal up directions
	// Check left-right directions
	uint16_t ldiag_upcount = 0;
	uint16_t rdiag_downcount = 0;
	// Right direction
	for (i = ((int)(row_pos)-1), j = ((int)(col_pos)+1); (i >= 0)&&(j < NUM_C4_COLS); i--, j++)
	{
		if(Coin_position[i][j] == player)
			rdiag_downcount++;
		else
			break;
	}

	// Left direction
	for (i = ((int)(row_pos)+1), j = ((int)(col_pos)-1); (i < NUM_C4_ROWS)&&(j >= 0); i++, j--)
	{
		if(Coin_position[i][j] == player)
			ldiag_upcount++;
		else
			break;
	}

	if((*highest_connect) < (ldiag_upcount+rdiag_downcount+1))
		*highest_connect = (ldiag_upcount+rdiag_downcount+1);

	// up, down, and count current coin itself
	if ((ldiag_upcount+rdiag_downcount+1) >= 4)
		return 4;

	// Check left diagonal up-down directions

	return 0;
}

void LCD_1Player_Mode(void)
{
	//for player 2 use random generator to make choice for where coin goes
	//rng for which column and then drop
	//eventually will check win condition and based on that place coin (check for 3 and then 2 and then use rng)
	//different function for win condition
	//use this to determine if rng is used or not and get rid of other function?




	LCD_PlayBoard_SetUp();
	// LCD_Clear(0,LCD_COLOR_BLUE);
	HAL_Delay(500);
}

void LCD_2Player_Mode(void)
{
	//what needs to be done here?
	//here make sure that both players play with button interrupt (don't have rng or anything else)
		//is this function needed?
	LCD_PlayBoard_SetUp();
	// LCD_Clear(0,LCD_COLOR_RED);
	HAL_Delay(500);
}

void Reset_Coin_Matrix()
{
	for(uint16_t row = 0; row < NUM_C4_ROWS; row++)
	{
		for(uint16_t column = 0; column < NUM_C4_COLS; column++)
		{
			Coin_position[row][column] = 0;
		}
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void LCD_Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// Touch Functionality   //

#if COMPILE_TOUCH_FUNCTIONS == 1

void InitializeLCDTouch(void)
{
  if(STMPE811_Init() != STMPE811_State_Ok)
  {
	 for(;;); // Hang code due to error in initialzation
  }
}

STMPE811_State_t returnTouchStateAndLocation(STMPE811_TouchData * touchStruct)
{
	return STMPE811_ReadTouch(touchStruct);
}

void DetermineTouchPosition(STMPE811_TouchData * touchStruct)
{
	STMPE811_DetermineTouchPosition(touchStruct);
}

uint8_t ReadRegisterFromTouchModule(uint8_t RegToRead)
{
	return STMPE811_Read(RegToRead);
}

void WriteDataToTouchModule(uint8_t RegToWrite, uint8_t writeData)
{
	STMPE811_Write(RegToWrite, writeData);
}

#endif // COMPILE_TOUCH_FUNCTIONS
