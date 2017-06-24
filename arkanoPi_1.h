/** 
  * Author: Daniel Suarez Souto y Alberto Jimenez Aliste

  * File Name          : arkanoPi_1.h
*/
#ifndef _ARKANOPI_H_
#define _ARKANOPI_H_

/* Includes ------------------------------------------------------------------*/
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>//para la mejora
#include "fsm.h"
#include "kbhit.h" // para poder detectar teclas pulsadas sin bloqueo y leer las teclas pulsadas
#include "tmr.h"
#include "arkanoPiLib.h"
#include "pong.h"
#include <softTone.h>


/* Define ------------------------------------------------------------------*/
#define FLAG_TECLA 0x01
#define FLAG_PELOTA 0x02
#define FLAG_RAQUETA_DERECHA 0x04
#define FLAG_RAQUETA_IZQUIERDA 0x08
#define FLAG_FINAL_JUEGO 0x10
#define FLAG_PAUSE 0x20
#define FLAG_ADC 0x40
#define FLAG_ARKANOPI 0x80
#define FLAG_DIFICULTAD 0x100
#define FLAG_PONG 0x200
#define FLAG_RAQUETA_DERECHA_PONG 0x400
#define FLAG_RAQUETA_IZQUIERDA_PONG 0x800
#define FLAG_PAUSE_PONG 0x1000

#define GPIO_FILA_0 24
#define GPIO_FILA_1 1
#define GPIO_FILA_2 2
#define GPIO_FILA_3 3
#define GPIO_FILA_4 4
#define GPIO_FILA_5 7
#define GPIO_FILA_6 23
#define GPIO_COL_0 14
#define GPIO_COL_1 17
#define GPIO_COL_2 18
#define GPIO_COL_3 22

#define GPIO_TECLADO_COL_1 0
#define GPIO_TECLADO_FILA_1 5
#define GPIO_TECLADO_FILA_2 6
#define GPIO_TECLADO_FILA_3 12
#define GPIO_TECLADO_FILA_4 13

#define GPIO_BOTON_I 21
#define GPIO_BOTON_D 20
#define DEBOUNCE_TIME 200

#define GPIO_SOFTTONE 25

#define FLAGS_KEY 1
#define STD_IO_BUFFER_KEY 2
#define PELOTA_KEY 3
#define CLK_MS 30 // PERIODO DE ACTUALIZACION DE LA MAQUINA ESTADOS

/* Definition Types of variables ------------------------------------------------------------------*/
typedef enum {
	WAIT_START=0,
	WAIT_PUSH=1,
	WAIT_END=2,
	WAIT_PAUSE=3,
	WAIT_JUEGO=4,
	WAIT_START_PONG=5,
	WAIT_PUSH_PONG=6} tipo_estados_juego;
typedef struct {
	tipo_arkanoPi arkanoPi;
	tipo_pong pong;
	tipo_estados_juego estado;
	char teclaPulsada;

} tipo_juego;

/* Action Functions ------------------------------------------------------------------*/
void InicializaDificultad (fsm_t* this);
void InicializaJuego (fsm_t* this);
void InicializaJuegoPong (fsm_t* this);
void MueveRaquetaIzquierda (fsm_t* this);
void MueveRaquetaDAbajo(fsm_t* this);
void MueveRaquetaDerecha (fsm_t* this);
void MueveRaquetaDArriba (fsm_t* this);
void MueveRaquetaIAbajo (fsm_t* this);
void MueveRaquetaIArriba (fsm_t* this);
void MovimientoPelota (fsm_t* this);
void PausarJuego(fsm_t* this);
void FinalJuego (fsm_t* this);
void FinalJuegoPong (fsm_t* this);
void ReseteaJuego (fsm_t* this);
void lectura_ADC (fsm_t* this);
/* Input Functions ------------------------------------------------------------------*/
int CompruebaArkanoPi(fsm_t* this);
int CompruebaPong(fsm_t* this);
int CompruebaDificultad(fsm_t* this);
int CompruebaPause(fsm_t* this);
int CompruebaPausePong(fsm_t* this);
int CompruebaTeclaPulsada(fsm_t* this);
int CompruebaTeclaPelota(fsm_t* this);
int CompruebaTeclaRaquetaDerecha(fsm_t* this);
int CompruebaTeclaRaquetaDerechaPong(fsm_t* this);
int CompruebaTeclaRaquetaIzquierda(fsm_t* this);
int CompruebaFinalJuego(fsm_t* this);
int CompruebaADC(fsm_t* this) ;
/* Interruption Attention Functions ------------------------------------------------------------------*/
PI_THREAD (thread_explora_teclado);

/* Timer Functions ------------------------------------------------------------------*/
void timer_isr (union sigval value);
void timer_isr_pelota (union sigval value);
void timer_isr_joystick (union sigval value);

/* Interrupt Functions ------------------------------------------------------------------*/
void myInterruptIzq(void);
void myInterruptDer(void);

/* Set up Functions ------------------------------------------------------------------*/
int systemSetup (void);

#endif /* ARKANOPI_H_ */
