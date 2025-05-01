/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"

/* Static variables */
static uint16_t player_select = 0;
static uint8_t player_mode_1_or_2 = 1;
static uint8_t active_screen = 1; // For the 3 screens
static uint8_t just_moved_coin_down = 0;

extern void initialise_monitor_handles(void); 

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData StaticTouchData;
#endif // COMPILE_TOUCH_FUNCTIONS

void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);

    #if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be direclty up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

	#endif // COMPILE_TOUCH_FUNCTIONS
	Button_Init();
	// !!!!!! Disable button interrupt - keeps getting called repeatedly, use polling loop instead
	// __enable_irq();
	// HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	// set up screen 1
	LCD_Screen1();
	active_screen = 1;
}
/*
STMPE811_TouchData getTouchData()
{
	return StaticTouchData;
}*/

void LCD_Visual_Demo(void)
{
	// Test function - not called
	//visualDemo();
	//LCD_Screen1();
	LCD_PlayBoard_SetUp();
}

#if COMPILE_TOUCH_FUNCTIONS == 1
void LCD_Touch_Polling_Demo(void)
{
	//LCD_Clear(0,LCD_COLOR_GREEN);

	/* If button pressed */
	while (1)
	{
		bool button_on = Button_State();
		if(button_on)
		{
			if((active_screen == 2) && (just_moved_coin_down == 0))
			{
				uint8_t connected4 = LCD_MoveCoin_Down(&player_select);
				//create new coin - remove following, done in LCD_MoveCoin_Down
				// if(player == 1)
					// LCD_Draw_Circle_Fill(newx, Coin_ypos, 13, LCD_COLOR_RED);
				// if(player == 2)
					// LCD_Draw_Circle_Fill(newx, Coin_ypos, 13, LCD_COLOR_BLUE);
				//switch turns
				just_moved_coin_down = 1;
				if (connected4 > 0)
				{
					active_screen = 3;
				}
				else if(player_mode_1_or_2 == 1)
				{
					// If using AI, then make the next move automatically after
					// figuring out where the coin needs to be dropped
					LCD_Move_Coin_to_AI_Column();
					HAL_Delay(500);
					connected4 = LCD_MoveCoin_Down(&player_select);
					if (connected4 > 0)
					{
						active_screen = 3;
					}
				}
			}
		}
		else
		{
			just_moved_coin_down = 0;
		}

		/* If touch pressed */
		if ((button_on == false) && (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed)) {
			/* Touch valid */
			// printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);

			if(active_screen==1)
			{
				LCD_Clear(0, LCD_COLOR_WHITE);
				if(StaticTouchData.y >= LCD_PIXEL_HEIGHT/2)
				{
					player_mode_1_or_2 = 1;
					LCD_Clear(0, LCD_COLOR_RED);
					LCD_Write_1_or_2_Player(player_mode_1_or_2);
				}
				if(StaticTouchData.y < LCD_PIXEL_HEIGHT/2)
				{
					player_mode_1_or_2 = 2;
					LCD_Clear(0, LCD_COLOR_BLUE);
					LCD_Write_1_or_2_Player(player_mode_1_or_2);
				}
				HAL_Delay(500);
				LCD_Clear(0, LCD_COLOR_WHITE);

				active_screen = 2;
				LCD_PlayBoard_SetUp();
				player_select = 1;
			}

			if(active_screen==2)
			{
				//move coin left and right
				if(StaticTouchData.x < 120)
				{
					LCD_MoveCoin_Left(player_select);
				}
				if(StaticTouchData.x > 120)
				{
					LCD_MoveCoin_Right(player_select);
				}
			}

			if(active_screen==3)
			{
				// Note y axis is complement of value return because axis is flipped here
				// Can be made consistent by changing StaticTouchData.orientation
				if (1 == LCD_Is_Within_Screen3_Button(StaticTouchData.x, LCD_PIXEL_HEIGHT - StaticTouchData.y))
				{
					LCD_Clear(0, LCD_COLOR_WHITE);
					LCD_Screen1();
					active_screen = 1;
				}
			}

		} else {
			/* Touch not pressed */
			printf("Not Pressed\n\n");
			/*if(sceen1flag)
			if(screen2flag == 1 && screen2_startflag == 1)
			{
				if(player_mode == 1)
					LCD_1Player_Mode();
				if(player_mode == 2)
					LCD_2Player_Mode();
				screen2_startflag = 0;
			}
			if(screen2 == 1 && screen2_startflag == 0)
			{

			}*/
			//LCD_Clear(0, LCD_COLOR_GREEN);
		}
	}
}
#endif // COMPILE_TOUCH_FUNCTIONS


void EXTI0_IRQHandler(){
//	if (button_push_completed == 0)
//	{
		bool button_on = Button_State();
		if(button_on)
		{
			IRQ_Disable_Interrupt(EXTI0_IRQn);
			LCD_MoveCoin_Down(&player_select);
			HAL_Delay(100);
			IRQ_Clear_Interrupt_Pending(EXTI0_IRQn);
			HAL_Delay(100);
			IRQ_Enable_Interrupt(EXTI0_IRQn);
		}
		else
		{
			// button_push_completed = true;
			IRQ_Disable_Interrupt(EXTI0_IRQn);
			IRQ_Clear_Interrupt_Pending(EXTI0_IRQn);
			IRQ_Enable_Interrupt(EXTI0_IRQn);
		}
//	}
}


