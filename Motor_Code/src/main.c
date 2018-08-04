/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f30x.h"
#include "stm32f3_discovery.h"
#include "math.h"

int uartreceive()
{
	while(!USART_GetFlagStatus(UART4, USART_FLAG_RXNE));
	return USART_ReceiveData(UART4);
}
long double map(long double x, long double in_min, long double in_max, long double out_min, long double out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void ellipticalSquareToDisc(long double x, long double y, long double *u, long double *v)
{
	*u = x * sqrt(1.0 - y*y/2.0);
	*v = y * sqrt(1.0 - x*x/2.0);
}

void ellipticalDiscToSquare(long double u, long double v, long double *x, long double *y)
{
	long double u2 = u * u;
	long double v2 = v * v;
	long double twosqrt2 = 2.0 * sqrt(2.0);
	long double subtermx = 2.0 + u2 - v2;
	long double subtermy = 2.0 - u2 + v2;
	long double termx1 = subtermx + u * twosqrt2;
	long double termx2 = subtermx - u * twosqrt2;
	long double termy1 = subtermy + v * twosqrt2;
	long double termy2 = subtermy - v * twosqrt2;
	*x = 0.5 * sqrt(termx1) - 0.5 * sqrt(termx2);
	*y = 0.5 * sqrt(termy1) - 0.5 * sqrt(termy2);
}

void gpioinit()
{
	//Enable clock for GPIOE
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE,ENABLE);

	//SET GPIO PIN 13 as output
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13| GPIO_Pin_12| GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;

	// Initialization of GPIO PORT E Pin 13 and Pin 12
	GPIO_Init(GPIOE,&GPIO_InitStruct);
}

void pwminit()
{
	GPIO_InitTypeDef            GPIO_InitStructure;				//structure for GPIO setup
			TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;			//structure for TIM Time Base
			TIM_OCInitTypeDef			TIM_OCInitStructure;			//structure for TIM Output Compare

			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);			//enable the AHB Peripheral Clock to use GPIOB
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);		//enable the TIM1 clock

			/* Pin configuration */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_Init(GPIOE, &GPIO_InitStructure);
			GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_2);
			GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_2);

			TIM_TimeBaseStructure.TIM_Period = 600;
			TIM_TimeBaseStructure.TIM_Prescaler = 6000-1;
			TIM_TimeBaseStructure.TIM_ClockDivision = 0;
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
			TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
			TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

			TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OCInitStructure.TIM_Pulse = 600/1.2;
			TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

			TIM_OC1Init(TIM1, &TIM_OCInitStructure);
			TIM_OC2Init(TIM1, &TIM_OCInitStructure);

			TIM_CtrlPWMOutputs(TIM1, ENABLE);							//enable the PWM output
			TIM_Cmd(TIM1, ENABLE);
			TIM_SetCompare1(TIM1, 0);
				TIM_SetCompare2(TIM1, 0);
}
void UART_Init()
{

	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* USARTx configured as follow:
				- BaudRate = speed parameter above
				- Word Length = 8 Bits
				- One Stop Bit
				- No parity
				- Hardware flow control disabled (RTS and CTS signals)
				- Receive and transmit enabled
	*/

	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	/* Enable USART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* USART configuration */
	USART_InitStructure.USART_BaudRate = 19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Connect PXx to USARTx_Tx */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_5);

	/* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_5);

	/* Enable USART */
	USART_Cmd(UART4, ENABLE);
}

void motorcode(long double x, long double y,long double gear)
{
	x = x - 4999.5;
	y = 4999.5 - y;

	/*if(y/x>=12.8&&y/x<=-12.8)
	x=0;
	if(y/x<=0.390625&&y/x>=-0.390625)
	y=0;
*/
	x=map(x,-4999.5,4999.5,-1.0,1.0);
	y=map(y,-4999.5,4999.5,-1.0,1.0);
	long double xans,yans;
	ellipticalSquareToDisc(x, y, &xans, &yans);
	x = (xans * 0.707) + (yans * 0.707);
	y = (-xans * 0.707) + (yans * 0.707);
	ellipticalDiscToSquare(x, y, &xans, &yans);
	x=(int)map(xans,-0.991273,0.991273,-499,499);
	y=(int)map(yans,-0.991273,0.991273,-499,499);

	if(x>0)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_13);

		TIM_SetCompare1(TIM1, x);
	}
	else
	{
		GPIO_ResetBits(GPIOE,GPIO_Pin_13);

		TIM_SetCompare1(TIM1, -x);
	}

	if(y>0)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_12);

		TIM_SetCompare2(TIM1, y);
	}
	else
	{
		GPIO_ResetBits(GPIOE,GPIO_Pin_12);

		TIM_SetCompare2(TIM1, -y);
	}

}

int main(void)
{
	long double x=0;
	long double y=0;
	gpioinit();
	pwminit();
	UART_Init();
	while(1)
	{
		if(USART_ReceiveData(UART4)=='m')
			{
			GPIO_SetBits(GPIOE, GPIO_Pin_10);
			int gear=uartreceive()-'0';
			if(uartreceive()=='x')
					{
						x=(uartreceive()-'0')*1000+(uartreceive()-'0')*100+(uartreceive()-'0')*10+(uartreceive()-'0');
					}
			else

			{
				GPIO_ResetBits(GPIOE, GPIO_Pin_10);
				continue;
			}
			if(uartreceive()=='y')
					{
						y=(uartreceive()-'0')*1000+(uartreceive()-'0')*100+(uartreceive()-'0')*10+(uartreceive()-'0');
					}

			else
			{
				GPIO_ResetBits(GPIOE, GPIO_Pin_10);
				continue;
			}
			motorcode(x,y,gear);
			}

		else
		{
			GPIO_ResetBits(GPIOE, GPIO_Pin_10);

		}
	}
}

