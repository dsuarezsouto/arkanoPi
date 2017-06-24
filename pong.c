/**
 * Author: Daniel Suarez Souto y Alberto Jimenez Aliste
 * File Name          : pong.c
 */

#include "pong.h"
#include <stdio.h>

/* Set up/Reset Functions ------------------------------------------------------------------*/
/**
 * void ReseteaMatrizPong(tipo_pantalla *p_pantalla):metodo que pone a 0 todos los elemendos
 * del atributo matriz del objeto tipo pantalla que le pasas como parametro.
*/
void ReseteaMatrizPong(tipo_pantalla *p_pantalla) {
	int i, j = 0;

	for(i=0;i<MATRIZ_ANCHO;i++) {
		for(j=0;j<MATRIZ_ALTO;j++) {
			p_pantalla->matriz[i][j] = 0;
		}
	}
}

/**
 * void ReseteaPelotaPong(tipo_pelota *p_pelota):metodo que modifica la posicion y trayectoria inicial
 * del objeto tipo pelota que le pasas como parametro.
*/
void ReseteaPelotaPong(tipo_pelota *p_pelota) {
	/* Pelota inicialmente en el centro de la pantalla */
	p_pelota->x = MATRIZ_ANCHO/2 - 1;
	p_pelota->y = MATRIZ_ALTO/2 - 1;

	/* Trayectoria inicial */
	p_pelota->yv = 0;
	p_pelota->xv = -1;
}
/**
 * void ReseteaRaquetaPong(tipo_raqueta *p_raquetaD, tipo_raqueta *p_raquetaI):metodo que modifica el ancho, alto y
 * las posiciones x e y iniciales de los objetos tipo raqueta que le pasas como parametro.
*/
void ReseteaRaquetaPong(tipo_raqueta *p_raquetaD, tipo_raqueta *p_raquetaI) {
	// Raqueta inicialmente en el centro de la pantalla
	p_raquetaI->x = 0;
	p_raquetaI->y= 2;
	p_raquetaI->ancho = RAQUETA_ANCHO_PONG;
	p_raquetaI->alto = RAQUETA_ALTO_PONG;

	p_raquetaD->x = 9;
	p_raquetaD->y = 2;
	p_raquetaD->ancho = RAQUETA_ANCHO_PONG;
	p_raquetaD->alto = RAQUETA_ALTO_PONG;

	
}

/* Display Functions ------------------------------------------------------------------*/

/**
 * void PintaMensajeInicialPantallaPong (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_inicial):
 * metodo encargado de aprovechar el display para presentar un mensaje de bienvenida al usuario
 * pasado como parametro.
*/
void PintaMensajeInicialPantallaPong (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_inicial) {
	int i, j = 0;
		for(i=0; i<MATRIZ_ANCHO;i++){
			for(j=0; j<MATRIZ_ALTO;j++){
			p_pantalla -> matriz[i][j] = p_pantalla_inicial-> matriz[i][j];
		}
	}

}

/**
 * void PintaPantallaPorTerminalPong  (tipo_pantalla *p_pantalla): metodo encargado de mostrar
 * el contenido o la ocupacion de la matriz de leds en la ventana de
 * terminal o consola. Este metodo es fundamental para facilitar
 * la labor de depuracion de errores.
*/
void PintaPantallaPorTerminalPong  (tipo_pantalla *p_pantalla) {
	printf("PANTALLA");
	printf("\n");
	int i, j = 0;
	for(i=0; i<MATRIZ_ALTO;i++){
			for(j=0; j<MATRIZ_ANCHO;j++){
			printf("%d",p_pantalla -> matriz[j][i]);
		}
		printf("\n");
}
}
/**
 * void PintaRaquetaPong(tipo_raqueta *p_raquetaD ,tipo_raqueta *p_raquetaI, tipo_pantalla *p_pantalla): funcion encargada
 * de pintar la raqueta en su posicion correspondiente dentro del area de juego
*/
void PintaRaquetaPong(tipo_raqueta *p_raquetaD ,tipo_raqueta *p_raquetaI, tipo_pantalla *p_pantalla) {
	int i, j = 0;

	for(i=0;i<RAQUETA_ANCHO_PONG;i++) {
		for(j=0;j<RAQUETA_ALTO_PONG;j++) {
			if (( (p_raquetaD->x+i >= 0) && (p_raquetaD->x+i < MATRIZ_ANCHO) ) &&
					( (p_raquetaD->y+j >= 0) && (p_raquetaD->y+j < MATRIZ_ALTO) )){
				    p_pantalla->matriz[p_raquetaD->x+i][p_raquetaD->y+j] = 1;
			}
			if (( (p_raquetaI->x+i >= 0) && (p_raquetaI->x+i < MATRIZ_ANCHO) ) &&
					( (p_raquetaI->y+j >= 0) && (p_raquetaI->y+j < MATRIZ_ALTO) )){
					p_pantalla->matriz[p_raquetaI->x+i][p_raquetaI->y+j] = 1;
		}
	}
}
}

/**
 * void PintaPelota(tipo_pelota *p_pelota, tipo_pantalla *p_pantalla): funcion encargada de pintar
 * la pelota en su posicion correspondiente dentro del area de juego
*/
void PintaPelotaPong(tipo_pelota *p_pelota, tipo_pantalla *p_pantalla) {
	if( (p_pelota->x >= 0) && (p_pelota->x< MATRIZ_ANCHO) ) {
		if( (p_pelota->y >= 0) && (p_pelota->y < MATRIZ_ALTO) ) {
			p_pantalla->matriz[p_pelota->x][p_pelota->y] = 1;
		}
		else {
			printf("\n\nPROBLEMAS!!!! posicion y=%d de la pelota INVALIDA!!!\n\n", p_pelota->y);
			fflush(stdout);
		}
	}
	else {
		printf("\n\nPROBLEMAS!!!! posicion x=%d de la pelota INVALIDA!!!\n\n", p_pelota->x);
		fflush(stdout);
	}
}


/**
 * void ActualizaPantallaPong(tipo_pong* p_pong): metodo cuya ejecucion estara ligada a cualquiera
 * de los movimientos de las raquetas o de la pelota y que sera el encargado de actualizar convenientemente
 * la estructura de datos en memoria que representa el area de juego y su correspondiente estado.
*/
void ActualizaPantallaPong(tipo_pong *p_pong) {
    // Borro toda la pantalla
	ReseteaMatrizPong((tipo_pantalla*)(&(p_pong->pantalla)));
    PintaRaquetaPong((&(p_pong->raquetaD)),(&(p_pong->raquetaI)),(&(p_pong->pantalla)));
    PintaPelotaPong((&(p_pong->pelota)),(&(p_pong->pantalla)));
    PintaPantallaPorTerminalPong((&(p_pong->pantalla)));
}

/**
 * void InicializaPong(tipo_pong *p_pong): metodo encargado de la inicializacion
 * de toda variable o estructura de datos especificamente ligada al desarrollo del juego y su
 * visualizacion.
*/
void InicializaPong(tipo_pong *p_pong) {
	ReseteaMatrizPong((tipo_pantalla*)(&(p_pong->pantalla)));
	ReseteaPelotaPong((tipo_pelota*)(&(p_pong->pelota)));
	ReseteaRaquetaPong((tipo_raqueta*)(&(p_pong->raquetaD)), (tipo_raqueta*)(&(p_pong->raquetaI)));
	ActualizaPantallaPong(p_pong);
}




