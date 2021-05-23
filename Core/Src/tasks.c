#include "tasks.h"
#include "main.h"
#include "queue.h"
#include "Circular_buffer.h"
#include <stdio.h>
#include "stm32f1xx_it.h"
#include "crc.h"

UART_HandleTypeDef huart1;

void UARTIRQHandlerC()
{
	getFreeSize();
	sendByte(&USART1->DR);
	USART1->SR &= ~USART_SR_RXNE;
}

static void UARTTask(void *parm);
static void DriveTask(void *parm);
static void Custom_SPI_WriteByte(uint8_t data);
static xQueueHandle queueDerection;

void builsTasks(void)
{
	queueDerection = xQueueCreate(1,(sizeof(uint8_t)));
	xTaskCreate(UARTTask, "UARTTask", 128, NULL, osPriorityNormal, NULL);
	xTaskCreate(DriveTask, "DriveTask", 128, NULL, osPriorityHigh, NULL);
}

void printAT(char *str)
{
	static char print[100];

	int size = sprintf(print,"%s\r\n",str);

	HAL_UART_Transmit_IT(&huart1, (uint8_t*)print,(uint16_t)size);
}

void Custom_SPI_WriteByte(uint8_t data)
{
	GPIOA->BSRR = GPIO_BSRR_BR4;

	for(uint8_t i = 0; i<8; i++)
	{
		if(data & (1<<(7-i))) GPIOA->BSRR = GPIO_BSRR_BR6;
		else GPIOA->BSRR = GPIO_BSRR_BS6;

		GPIOA->BSRR = GPIO_BSRR_BS5;
		osDelay(1);
		GPIOA->BSRR = GPIO_BSRR_BR5;
		osDelay(1);
		GPIOA->BSRR = GPIO_BSRR_BR6;
	}

	GPIOA->BSRR = GPIO_BSRR_BS4;
}

int findToArray(char *buf, int str)
{
	int i = 0;
	int k = -1;

	for (i=1; i<sizeof(buf);i++)
	{
		if (buf[i]==str) k=i;
	}
	return k;
}

char buf[1024];
char bufW[9]="PACK";// +test ;

#pragma pack(push, 1)
typedef struct
{
	uint8_t direction;
	uint8_t crc;
} test;
#pragma pack(pop)

typedef union
{
	uint8_t array [sizeof(test)];
	test st;
} getDataUn;


static void UARTTask(void *parm)
{
	printAT("AT");
	osDelay(1000);

	printAT("AT+RST");
	osDelay(1000);

	printAT("AT+SLEEP=0");
	osDelay(1000);

	printAT("AT+CWMODE_DEF=1");
	osDelay(1000);

	printAT("AT+CWJAP=\"Donot\",\"Kompplaytatyana2606\"");
	osDelay(5000);

	printAT("AT+CIFSR");
	osDelay(5000);

	printAT("AT+CIPSTART=\"TCP\",\"192.168.4.1\",333");
	osDelay(5000);

	GPIOA->BSRR = GPIO_BSRR_BR7;

	getDataUn test1;
	uint8_t OK=0x0d0a4f4b;
	uint32_t Template=0x442c323a;
	uint32_t com;
	while(1)
	{
		uint32_t crc;
		uint8_t direction;
		uint32_t crc1;
		if(getCurentLenght())
		{
			com = com << 8;
			com |= (uint32_t)readByte();
		/*	if(com == OK)
			{
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
				osDelay(200);
			}*/
			if(com == Template)
			{
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
				for(int i = 0; i<sizeof(test1) ; i++)
				{
					if(getCurentLenght())
					{
						test1.array[i]=readByte();

					}
					else
					{
						osDelay(1000);
						i--;
					}
				}
				//xQueueReceive(queueDerection, NULL, 0);
				xQueueSend(queueDerection, &test1.st.direction, 0);
			}
		}
	}

}
typedef enum
{
	DIRECTION_left = 0,
	DIRECTION_right,
	DIRECTION_front,
	DIRECTION_back
} DIRECTION;

static void DriveTask(void *parm)
{
	uint8_t derection;
	while(1)
	{
		xQueueReceive(queueDerection, &derection, portMAX_DELAY);

		asm("NOP");
		switch(derection)
		{
		case (1<<DIRECTION_right):
			Custom_SPI_WriteByte((1<<7));
			osDelay(2000);
			Custom_SPI_WriteByte(0);
			osDelay(1000);
		break;
		case (1<<DIRECTION_back):
			Custom_SPI_WriteByte((1<<3));
			osDelay(2000);
			Custom_SPI_WriteByte(0);
			osDelay(1000);
		break;
		case (1<<DIRECTION_front):
			Custom_SPI_WriteByte((1<<2));
			osDelay(2000);
			Custom_SPI_WriteByte(0);
			osDelay(1000);
		break;
		case (1<<DIRECTION_left):
			Custom_SPI_WriteByte((1<<5));
			osDelay(2000);
			Custom_SPI_WriteByte(0);
			osDelay(1000);
			break;
		default:
			Custom_SPI_WriteByte(0);
			osDelay(1000);
		}
	}

}

