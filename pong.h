/*
 * Author: Daniel Suarez Souto y Alberto Jimenez Aliste
 * File Name          : pong.h
 */
#ifndef _PONG_H_
#define _PONG_H_

#include <stdio.h>
#include "arkanoPiLib.h"

#define RAQUETA_ALTO_PONG		3
#define RAQUETA_ANCHO_PONG 		1

typedef struct {
  tipo_pantalla pantalla;
  tipo_raqueta raquetaD;
  tipo_raqueta raquetaI;
  tipo_pelota pelota;
} tipo_pong;


/* Set up/Reset Functions ------------------------------------------------------------------*/
void ReseteaMatrizPong(tipo_pantalla *p_pantalla);
void ReseteaPelotaPong(tipo_pelota *p_pelota);
void ReseteaRaquetaPong(tipo_raqueta *p_raquetaD, tipo_raqueta *p_raquetaI) ;

/* Display Functions ------------------------------------------------------------------*/
void PintaPantallaPorTerminalPong (tipo_pantalla *p_pantalla);
void PintaRaquetaPong(tipo_raqueta *p_raquetaD ,tipo_raqueta *p_raquetaI, tipo_pantalla *p_pantalla);
void PintaPelotaPong(tipo_pelota *p_pelota, tipo_pantalla *p_pantalla);
void ActualizaPantallaPong(tipo_pong *p_pong);
void InicializaPong(tipo_pong *p_pong);


#endif /* _PONG_H_ */

