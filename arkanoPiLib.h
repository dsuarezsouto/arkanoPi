/** 
  * Author: Daniel Suarez Souto y Alberto Jimenez Aliste
  * File Name          : arkanoPiLib.h
*/

#ifndef _ARKANOPILIB_H_
#define _ARKANOPILIB_H_

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

/* CONSTANTES DEL JUEGO */
#define MATRIZ_ANCHO 	10
#define MATRIZ_ALTO 	7
#define LADRILLOS_ANCHO 10
#define LADRILLOS_ALTO 	2
#define RAQUETA_ANCHO 	3
#define RAQUETA_ALTO 	1

/* Definition Types of variables ------------------------------------------------------------------*/
typedef struct {
	/* Posicion */
	int x;
	int y;
	/* Forma */
	int ancho;
	int alto;
} tipo_raqueta;

typedef struct {
	/* Posicion */
	int x;
	int y;
	/* Trayectoria */
	int xv;
	int yv;
} tipo_pelota;

/*Tipo paquete para mejora */
typedef struct{
    int encendido;// Si es '1' hay un paquete generado si está a '0' no hay paquete generado
    int x;
    int y;
}tipo_paquete;

typedef struct {
	/**
	 * Matriz de ocupacion de las distintas posiciones que conforman el display
	 * (correspondiente al estado encendido/apagado de cada uno de los leds)
	*/
	int matriz[MATRIZ_ANCHO][MATRIZ_ALTO];
} tipo_pantalla;

typedef struct {
  tipo_pantalla ladrillos; /* Notese que, por simplicidad, los ladrillos comparten tipo con la pantalla */
  tipo_pantalla pantalla;
  tipo_raqueta raqueta;
  tipo_pelota pelota;
  tipo_paquete paquete;
} tipo_arkanoPi;

extern tipo_pantalla pantalla_inicial;

/* Set up/Reset Functions ------------------------------------------------------------------*/
void ReseteaMatriz(tipo_pantalla *p_pantalla);
void ReseteaLadrillos(tipo_pantalla *p_ladrillos);
void ReseteaPelota(tipo_pelota *p_pelota);
void ReseteaRaqueta(tipo_raqueta *p_raqueta);

/* Display Functions ------------------------------------------------------------------*/
void PintaMensajeInicialPantalla (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_inicial);
void PintaMensajeFinalPantalla (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_inicial);
void PintaPantallaPorTerminal (tipo_pantalla *p_pantalla);
void PintaLadrillos(tipo_pantalla *p_ladrillos, tipo_pantalla *p_pantalla);
void PintaRaqueta(tipo_raqueta *p_raqueta, tipo_pantalla *p_pantalla);
void PintaPelota(tipo_pelota *p_pelota, tipo_pantalla *p_pantalla);
void PintaPaquete(tipo_paquete *p_paquete, tipo_pantalla *p_pantalla);
void ActualizaPantalla(tipo_arkanoPi* p_arkanoPi);
void InicializaArkanoPi(tipo_arkanoPi *p_arkanoPi);
int CalculaLadrillosRestantes(tipo_pantalla *p_ladrillos);

#endif /* _ARKANOPILIB_H_ */
