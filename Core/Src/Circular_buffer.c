#include "Circular_buffer.h"
#include "cmsis_os.h"
#define ARREY_SIZE 2048

static uint16_t CurentPos = 0;//текущая позиция
static uint16_t CurentBack = 0;//концовка
static uint16_t CurentLenght = 0;//колличество данных
static uint8_t buf[ARREY_SIZE];

uint16_t getCurentLenght(void)
{
	return CurentLenght;
}

uint8_t readByte(void)//вычитка данных из буфера
{
	uint8_t *pointr = buf;
		if (CurentLenght > 0)
		{
			CurentLenght--;
			pointr = buf + CurentBack;
			CurentBack++;
			CurentBack %= ARREY_SIZE;
		}
	return *pointr;
}

bool sendByte(uint8_t *date)
{
	bool state = false;
		if((CurentLenght+1) < ARREY_SIZE )//проверка на переполнение буфера
		{
			buf[CurentPos++] = *(uint8_t*)date; // данные
			CurentPos %= ARREY_SIZE;// проверка на выход за границы
			CurentLenght++;
			state = true;

			static int coint = 0;
				coint++;
				*(int*)(buf+10 )= coint;
		}
	return state;
}
uint16_t getFreeSize(void)
{
	return ARREY_SIZE - 1 - CurentLenght;
}
