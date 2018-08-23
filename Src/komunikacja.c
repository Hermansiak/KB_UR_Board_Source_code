#include <komunikacja.h>
#include "string.h"
#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_conf.h"
#include <unistd.h>
extern UART_HandleTypeDef huart1;

//Zmienne definiowane przez użytkownika
#define size 10
uint8_t bufor_testowy[200];
uint8_t bufor[size+1]; /*bufor z transmisji przychdozącej przeznaczony do rozpoznawania komend*/
int index=0; /*index wskazujący bieżącą pozycję w buforze*/
int flag_get=0;//flaga wykrycia komendy get
int flag_all=0;//flaga wykrycia komendy all
int flag_in=0;//flaga wykrycia komendy in
int flag_on=0;//flaga wykrycia komendy on
int flag_input_num=0;//flaga wykrycia numeru wejscia
int flag_device_num=0;// flaga wykrycia adresu urządzenia
enum COMMANDS prev_cmd=NO_CMD;	// zmienna zapamiętująca poprzednią przyjętą komendę
int input_number=0; //numer wejścia urządzenia
int device_number=0;// adres urządzenia
int q=0;
int answer_index=0;
int voltage=10;
const char *answer="komenda poprawna! \n";

struct wiersz
{
	/*
	 * struktura dwuelementowa
	 * stworzona do przechowania elementów, które będą
	 * porównane z komendą zapisaną w Enum COMMANDS*/
	const enum COMMANDS cmd;
	const char *ptr_to_char;
};


const struct wiersz enum_tab[]={
		/*tablica struktur o nazwie wiersz
		 * wskazująca konkretne komendy*/
		{GET,"GET"},
		{ALL,"ALL"},
		{IN,"IN"},
		{ON,"ON"},
		{NO_CMD,"NO_CMD"}

};

void collect2 (char c)
{

	static enum READ_STATE rState = RS_START;
	bool flag_enter = false;

	/*Funkcja pobierania kolejnych znaków do bufora transmisji przychodzącej
	 *spacje/puste znaki są pomijane, dopiero gdy pojawi się jakieś znaki
	 *są one zapisywane na kolejne miejsca w buforze aż do pojawienia się kolejnej
	 *spacji/pustego znaku, wtedy wywoływana jest funkcja check_cmd przeznaczona
	 * do sprawdzenia odebranego tekstu pod względem pasującej komendy */

	if(!isspace(c))    {
		if(index<size){
			bufor[index]=toupper(c);
			index++;
		}else {
			rState = RS_ERROR;    // blad bo przekroczono bufor na pojedyncze slowo/liczbe
		}
	}else{    // byla przerwa, czyli analizowac komende
		int a;
		bufor[index]='\0';
		index=0;
		flag_enter = (c == '\n')? true : false;

		switch(rState){
		case RS_START:    //
			rState = (check_cmd() == GET) ? RS_GET : RS_ERROR;
			break;
		case RS_GET:    //
			switch(check_cmd()){
			case IN:
				rState = RS_GET_IN;
				break;
			case ALL:
				rState = RS_GET_ALL;
				break;
			default:
				rState=RS_ERROR;
			}
			break;
			case RS_GET_IN:    //
				a=atoi((const char *)bufor);
				rState = (a != 0) ? RS_GET_IN_VAL1 : RS_ERROR;
				break;
			case RS_GET_IN_VAL1:    //
				rState = (check_cmd() == ON) ? RS_GET_IN_ON : RS_ERROR;
				break;
			case RS_GET_IN_ON:    //
				a=atoi((const char *)bufor);
				rState = (a != 0) ? RS_GET_IN_ON_VAL2 : RS_ERROR;
				break;
				//            case RS_GET_IN_ON_VAL2:    // jest cala komenda GET IN xx ON yy, czekamy tylko na enter
				//                if (c == '\n') {
				//                    execute_cmd();
				//                    rState = RS_START;    // reset maszyny stanow
				//                }
				//                break;
			case RS_GET_ALL:    //
				rState = (check_cmd() == ON) ? RS_GET_ALL_ON : RS_ERROR;
				break;
			case RS_GET_ALL_ON:    //
				a=atoi((const char *)bufor);
				rState = (a != 0) ? RS_GET_ALL_ON_VAL1 : RS_ERROR;
				break;
				//            case RS_GET_ALL_ON_VAL1:    // jest cala komenda GET ALL ON yy, czekamy tylko na enter
				//                if (c == '\n') {
				//                    execute_cmd();    // powinno byc execute_cmd_GetAll();
				//                    rState = RS_START;    // reset maszyny stanow
				//                }
				//                break;

//			case RS_ERROR:    // jak bledna komenda to tylko oczekiwanie na enter
//					wrong_cmd();
//				}
		}

		if ( flag_enter){
			switch(rState){
			case RS_GET_ALL_ON_VAL1:
				execute_cmd();
				break;
			case RS_GET_ALL_ON_VAL1:
				execute_cmd();
				break;
			case RS_ERROR:
			default:
				wrong_cmd();
				break;
			}
			rState = RS_START;
		}

	}
}

/*
void collect (char c)
{
	bufor_testowy[q]=c;
	q++;

	/*Funkcja pobierania kolejnych znaków do bufora transmisji przychodzącej
 *spacje/puste znaki są pomijane, dopiero gdy pojawi się jakieś znaki
 *są one zapisywane na kolejne miejsca w buforze aż do pojawienia się kolejnej
 *spacji/pustego znaku, wtedy wywoływana jest funkcja check_cmd przeznaczona
 * do sprawdzenia odebranego tekstu pod względem pasującej komendy

	if(!isspace(c))
	{

		if(index<size)
		{

			bufor[index]=toupper(c);
			index++;
		}
	}else if(index>=10)
	{
		wrong_cmd();
	}else
	{
		//if(c=='\n')
		bufor[index]='\0';
		if(prev_cmd==IN || prev_cmd==ON)
		{
			enum COMMANDS cmd2=check_cmd();
			if(cmd2==NO_CMD)
			{
				if(flag_all==1 && flag_in==1)
					wrong_cmd();
				else
				{
					int a=atoi(bufor);
					if(a!=0)
					{
						if(prev_cmd==IN)
						{
							flag_input_num=1;
							input_number=a;
							prev_cmd=NO_CMD;
							index=0;
						}else if(prev_cmd==ON)
						{
							flag_device_num=1;
							device_number=a;
							index=0;

							if((flag_get==1 && flag_in==1 && flag_input_num==1 && flag_on==1 && flag_device_num==1) ||
									(flag_get==1 && flag_all==1 && flag_on==1 && flag_device_num==1))
							{
								execute_cmd();
							}
						}
					}else wrong_cmd();
				}
			}else wrong_cmd();
		}else{
			enum COMMANDS cmd1=check_cmd();
			prev_cmd=cmd1;
			index=0;
			switch (cmd1)			/*wykrywanie konkretnych ciągów komend
			{
			case GET:
				if(flag_get==1)
				{
					wrong_cmd();
				}
				else
				{
					flag_get=1;
				}
				break;
			case IN:
				if(flag_get==1)
				{
					if(flag_in==1)
					{
						wrong_cmd();
					}else if(flag_all==1)
					{
						wrong_cmd();
					}else flag_in=1;
				}else wrong_cmd();
				break;
			case ALL:
				if(flag_get==1)
				{
					if(flag_all==1)
					{
						wrong_cmd();
					}else if(flag_in==1)
					{
						wrong_cmd();
					}else flag_all=1;
				}else wrong_cmd();
				break;
			case ON:
				if(flag_get==1)
				{
					if(flag_all==1)
					{
						if(flag_on==1)
						{
							wrong_cmd();
						}else flag_on=1;

					}else if(flag_in==1)
					{
						if(flag_input_num==1)
						{
							if(flag_on==1)
							{
								wrong_cmd();
							}else flag_on=1;


						}else wrong_cmd();
					}else wrong_cmd();

				}else wrong_cmd();
				break;

			default:
				wrong_cmd();
				break;

			}
		}
	}
}*/
//bool check_cmd()
//{
//	strcmp(bufor, wzor_get);
//	return true;
//}

enum COMMANDS check_cmd()
{
	/**
	 * Funkcja przeznaczona do porównywania tekstu z bufora z komendami zapisanymi w enum 'COMMANDS'
	 * po znalezieniu odpowiadającej komendy zwraca ją, w razie niepowodzenia, zwraca wartosc 'NO_CMD'
	 */
	for(int i=0; i<sizeof(enum_tab)/sizeof(struct wiersz);i++)
	{
		if(strcmp((const char *)bufor,(const char *)enum_tab[i].ptr_to_char)==0)
		{
			return enum_tab[i].cmd;
		}
	}
	return NO_CMD;
}


void execute_cmd()
{

	/*
	 * Funkcja do wykonywania odebranej komendy
	 * */
	//USART_ISR_TXE=ENABLE;


	if(flag_in==1)
	{
		answer="komenda poprawna!\nWartosc napiecia na wejsciu  jest rowna " ;
	}
	else if(flag_all=1)
	{
		answer="komenda poprawna!\nWartosc napiecia:\nwejscie 1: \nwejscie2: \nwejscie3: \nwejscie 4: \nwejscie5: \nwejscie6: \n ";
	}
	huart1.Instance->CR1|= USART_CR1_TXEIE;
	prev_cmd=NO_CMD;
	flag_get=0;
	flag_in=0;
	flag_all=0;
	flag_input_num=0;
	flag_on=0;
	flag_device_num=0;
}

void wrong_cmd()
{
	flag_get=0;
	flag_all=0;
	flag_in=0;
	flag_on=0;
	flag_input_num=0;
	flag_device_num=0;
	bufor[index]='\0';
	q++;
	bufor_testowy[q]='X';
	index=0;
	prev_cmd=NO_CMD;
	answer="komenda zla\n";
	huart1.Instance->CR1|= USART_CR1_TXEIE;
}

void My_IRQ_Handler(UART_HandleTypeDef *huart)
{
	uint32_t isrflags   = READ_REG(huart->Instance->ISR);
	uint32_t cr1its     = READ_REG(huart->Instance->CR1);
	uint32_t cr3its;
	uint32_t errorflags;

	/* If no error occurs */
	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
	if (errorflags == RESET)
	{
		/* UART in mode Receiver ---------------------------------------------------*/
		if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{


			uint16_t* tmp;
			//	uint16_t  uhMask = huart->Mask;
			static uint16_t  uhdata;

			/* Check that a Rx process is ongoing */

			uhdata = (uint16_t) READ_REG(huart->Instance->RDR);

			//*huart->pRxBuffPtr++ = (uint8_t)(uhdata & (uint8_t)uhMask);

			collect2((char)uhdata);

			/* Disable the UART Parity Error Interrupt and RXNE interrupt*/
			//  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

			/* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
			// CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

			/* Rx process is completed, restore huart->RxState to Ready */
			//huart->RxState = HAL_UART_STATE_READY;

			//HAL_UART_RxCpltCallback(huart);
			/**
			 * @}
			 */
			return;
		}
	}else bufor[5]='n';

	if(((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|sterowanie_wyj_ciem_Pin, GPIO_PIN_SET);
		if(answer[answer_index]!='\0')
		{
			huart->Instance->TDR = (uint8_t)answer[answer_index];
			answer_index++;
		}
		else{
			CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);

			/* Enable the UART Transmit Complete Interrupt */
			SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);


		}
	}
	if(((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		CLEAR_BIT(huart->Instance->CR1, USART_CR1_TCIE);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|sterowanie_wyj_ciem_Pin, GPIO_PIN_RESET);
		answer_index=0;
	}
	/* End if some error occurs */
}
