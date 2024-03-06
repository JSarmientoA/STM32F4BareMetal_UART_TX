#include <stdint.h>
#include "stm32f4xx.h"

#define GPIOAEN			(1U<<0)
#define UART2EN			(1U<<17)
#define SYS_FREQ		16000000//Reloj confugurado por defaul en el sistema
#define APB1_CLK		SYS_FREQ
#define BAUDRATE		115200
#define CR1_TE			(1U<<3)
#define CR1_UE			(1U<<13)
#define SR_TXE			(1U<<7)

static void uart_set_bautrate(USART_TypeDef *USARTx, uint32_t clkPerife, uint32_t BaudRate);//
static uint16_t compute_uart_bd(uint32_t clkPerife, uint32_t BaudRate);
void uart2_tx_init(void);
void uart2_write(int ch);
int main(void)
{
	uart2_tx_init();
	while(1)
	{
		uart2_write('M');
	}
}
void uart2_tx_init(void)
{
	/***************Configurción del puerto UART**************/
	/*Active el acceso al reloj para los pines del puerto (gpioa)*/
	RCC->AHB1ENR |= GPIOAEN;
	/*Congigure el pin correpondiente en modo ALTERNATIVO, en este caso el pin PA2*/
	GPIOA->MODER &=~ (1U<<4);//Bit 4 del registro MODER en el periferico GPIOA en 0. &=~ es invertir
	GPIOA->MODER |=  (1U<<5);//Bit 5 del registro MODER en el periferico GPIOA en 1.
	/*Congigure el pin correpondiente segun en el modo altenativo que se quiera usar en este caso como uart (AF07)*/
	GPIOA->AFR[0] |=  (1U<<8);
	GPIOA->AFR[0] |=  (1U<<9);
	GPIOA->AFR[0] |=  (1U<<10);
	GPIOA->AFR[0] &= ~(1U<<11);
	/*En el periferico GPIOA se configura el registro AFRL(Regitro d funcion alternativa) (Dado que el registro tiene L y H,
	 con L se configuran los pines del 0 al 7 y H los pines del 8 al 16) por lo cual en la estructura
	 de datos esta configurado como un vector de dos posiciones con L=0(AFR[0]) y H=1(AFR[1]). El vaor del registro AFR[L H]
	 se optiene de la Table 11. Alternate function pag 57 hoja tecnica*/

	/***************Configurción del modulo UART**************/
	/*Habilitar el acceso al reloj de uart*/
	RCC->APB1ENR |= UART2EN;/* En el periferico RCC se accede al registro APB1ENR y se cmodifica el
	registro (1U<<17)*/
	/*Confugurar la velocidad en baudios (baudrate) del uart*/
	uart_set_bautrate(USART2, APB1_CLK, BAUDRATE);//
	/*Configurar la dirección de trnasferencia para este caso solo trnasmision*/
	USART2->CR1 = CR1_TE;/* Activa la transmisión. No se uso el oprador |= (OR) por que se aprobecho para limpiar
	todala uart por lo que la operacion (1U<<3) al colocar el bit 3 en 1 el resto
	los pone en cero por lo  que es la confihuracion predeterminada de la uart*/
	/*Habilitar el modulo uart*/
	USART2->CR1 |= CR1_UE;/*Cuando se borra este bit, los preescaladores y salidas USART se detienen y finaliza
	la transferencia de bytes actual para reducir el consumo de energía. Este bit se establece y borra por software.
	UE = 0: Prescaler USART y salidas deshabilitadas
	UE = 1: USART habilitado */
}

void uart2_write(int ch)
{
	/*Validar que el registro de datos este vacio antes de trnasmitir*/
	while(!(USART2->SR & SR_TXE)){}// Esta operación devuelve 1 si el bit TXE esta establecido
	/*Bit 7 TXE: registro de datos de transmisión vacío
	Este bit lo establece el hardware cuando el contenido del registro TDR se ha transferido al registro de desplazamiento.
	Se genera una interrupción si el bit TXEIE = 1 en el registro USART_CR1. Eso
	se borra mediante una escritura en el registro USART_DR.
	0: Los datos no se transfieren al registro de desplazamiento
	1: Los datos se transfieren al registro de desplazamiento)
	Nota: Este bit se utiliza durante la transmisión de un solo búfer.*/
	/*Escribir para trnasmitir en el registro de datos*/
	USART2->DR =(ch & 0xFF);/*Envia buffer*/
}

static void uart_set_bautrate(USART_TypeDef *USARTx, uint32_t clkPerife, uint32_t BaudRate)//
{
	USARTx->BRR = compute_uart_bd(clkPerife, BaudRate);
}

static uint16_t compute_uart_bd(uint32_t clkPerife, uint32_t BaudRate)
{
	return((clkPerife + (BaudRate/2U))/BaudRate);
	/*El valor que devuelve esta función se escribira en el
	 registro de velovidad de baudios BuadRate de la UART*/
}
