/** 
  * Author: Daniel Suarez Souto y Alberto Jimenez Aliste
  * File Name          : arkanoPiLib.c
*/

/* Includes ------------------------------------------------------------------*/
#include "arkanoPiLib.h"
#include "string.h"
/* Private variables ------------------------------------------------------------------*/
int ladrillos_basico[MATRIZ_ANCHO][MATRIZ_ALTO] = {
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
};

/* Set up/Reset Functions ------------------------------------------------------------------*/

/**
 * void ReseteaMatriz(tipo_pantalla *p_pantalla):metodo que pone a 0 todos los elemendos 
 * del atributo matriz del objeto tipo pantalla que le pasas como parametro.
*/
void ReseteaMatriz(tipo_pantalla *p_pantalla) {
	int i, j = 0;

	for(i=0;i<MATRIZ_ANCHO;i++) {
		for(j=0;j<MATRIZ_ALTO;j++) {
			p_pantalla->matriz[i][j] = 0;
		}
	}
}

/**
 * void ReseteaLadrillos(tipo_pantalla *p_ladrillos):metodo que iguala el atributo matriz del 
 * objeto tipo pantalla que le pasas como parametro con la variable global ladrillos_basico.
*/
void ReseteaLadrillos(tipo_pantalla *p_ladrillos) {
	int i, j = 0;

	for(i=0;i<MATRIZ_ANCHO;i++) {
		for(j=0;j<MATRIZ_ALTO;j++) {
			p_ladrillos->matriz[i][j] = ladrillos_basico[i][j];
		}
	}
}

/**
 * void ReseteaPelota(tipo_pelota *p_pelota):metodo que modifica la posicion y trayectoria inicial 
 * del objeto tipo pelota que le pasas como parametro.
*/
void ReseteaPelota(tipo_pelota *p_pelota) {
	/* Pelota inicialmente en el centro de la pantalla */
	p_pelota->x = MATRIZ_ANCHO/2 - 1;
	p_pelota->y = MATRIZ_ALTO/2 - 1;

	/* Trayectoria inicial */
	p_pelota->yv = 1;
	p_pelota->xv = 0;
}

/**
 * void ReseteaRaqueta(tipo_raqueta *p_raqueta)):metodo que modifica el ancho, alto y las posiciones 
 * x e y iniciales del objeto tipo raqueta que le pasas como parametro.
*/
void ReseteaRaqueta(tipo_raqueta *p_raqueta) {
	/* Raqueta inicialmente en el centro de la pantalla */
	p_raqueta->x = MATRIZ_ANCHO/2 - p_raqueta->ancho/2;
	p_raqueta->y = MATRIZ_ALTO - 1;
	p_raqueta->ancho = RAQUETA_ANCHO;
	p_raqueta->alto = RAQUETA_ALTO;
}

/* Display Functions ------------------------------------------------------------------*/

/**
 * void PintaMensajeInicialPantalla (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_inicial): 
 * metodo encargado de aprovechar el display para presentar un mensaje de bienvenida al usuario 
 * pasado como parametro.
*/
void PintaMensajeInicialPantalla (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_inicial) {
	int i, j = 0;


			for(i=0;i<MATRIZ_ALTO;i++) {
				for(j=0;j<MATRIZ_ANCHO;j++) {
					p_pantalla->matriz[j][i] = p_pantalla_inicial->matriz [j][i];
				}
			}
}
/**
 * void PintaMensajeFinalPantalla (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_final):
 * metodo encargado de aprovechar el display para presentar un mensaje de bienvenida al usuario
 * pasado como parametro.
*/
void PintaMensajeFinalPantalla (tipo_pantalla *p_pantalla, tipo_pantalla *p_pantalla_final) {
	int i, j = 0;


			for(i=0;i<MATRIZ_ALTO;i++) {
				for(j=0;j<MATRIZ_ANCHO;j++) {
					p_pantalla->matriz[j][i] = p_pantalla_final->matriz [j][i];
				}
			}
}
/**
 * void PintaPantallaPorTerminal  (tipo_pantalla *p_pantalla): metodo encargado de mostrar
 * el contenido o la ocupacion de la matriz de leds en la ventana de
 * terminal o consola. Este metodo es fundamental para facilitar
 * la labor de depuracion de errores.
*/
void PintaPantallaPorTerminal  (tipo_pantalla *p_pantalla) {
	printf("[PANTALLA]");
	printf("\n");
	int i, j = 0;


					for(i=0;i<MATRIZ_ALTO;i++) {
						for(j=0;j<MATRIZ_ANCHO;j++) {
						printf("%d",p_pantalla->matriz [j][i]);
					}
					printf("\n");
				}
}

/**
 * void PintaLadrillos(tipo_pantalla *p_ladrillos, tipo_pantalla *p_pantalla): funcion encargada de 
 * pintar los ladrillos en sus correspondientes posiciones dentro del area de juego.
*/
void PintaLadrillos(tipo_pantalla *p_ladrillos, tipo_pantalla *p_pantalla) {
	int i, j = 0;

	for(i=0;i<MATRIZ_ANCHO;i++) {
		for(j=0;j<MATRIZ_ALTO;j++) {
			p_pantalla->matriz[i][j] = p_ladrillos->matriz[i][j];
		}
    }
}

/**
 * void PintaRaqueta(tipo_raqueta *p_raqueta, tipo_pantalla *p_pantalla): funcion encargada de pintar 
 * la raqueta en su posicion correspondiente dentro del area de juego
*/
void PintaRaqueta(tipo_raqueta *p_raqueta, tipo_pantalla *p_pantalla) {
	int i, j = 0;

	for(i=0;i<p_raqueta->ancho;i++) {
		for(j=0;j<RAQUETA_ALTO;j++) {
			if (( (p_raqueta->x+i >= 0) && (p_raqueta->x+i < MATRIZ_ANCHO) ) &&
					( (p_raqueta->y+j >= 0) && (p_raqueta->y+j < MATRIZ_ALTO) ))
				p_pantalla->matriz[p_raqueta->x+i][p_raqueta->y+j] = 1;
		}
	}
}

/**
 * void PintaPelota(tipo_pelota *p_pelota, tipo_pantalla *p_pantalla): funcion encargada de pintar 
 * la pelota en su posicion correspondiente dentro del area de juego
*/ 
void PintaPelota(tipo_pelota *p_pelota, tipo_pantalla *p_pantalla) {
	if( (p_pelota->x >= 0) && (p_pelota->x < MATRIZ_ANCHO) ) {
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
 * void PintaPaquete(tipo_paquete *p_paquete, tipo_pantalla *p_pantalla): funcion encargada de pintar 
 * un paquete en su posicion correspondiente dentro del area de juego
*/ 
void PintaPaquete(tipo_paquete *p_paquete, tipo_pantalla *p_pantalla){
    
    /* La forma del paquete es cuadrado de 2x2 */
    p_pantalla->matriz[p_paquete->x][p_paquete->y] = 1;
    p_pantalla->matriz[(p_paquete->x)+1][p_paquete->y] = 1;
    p_pantalla->matriz[p_paquete->x][(p_paquete->y)-1] = 1;
    p_pantalla->matriz[(p_paquete->x)+1][(p_paquete->y)-1] = 1;   

}

/**
 * void ActualizaPantalla(tipo_arkanoPi* p_arkanoPi): metodo cuya ejecucion estara ligada a cualquiera 
 * de los movimientos de la raqueta o de la pelota y que sera el encargado de actualizar convenientemente 
 * la estructura de datos en memoria que representa el area de juego y su correspondiente estado.
*/
void ActualizaPantalla(tipo_arkanoPi* p_arkanoPi) {

    /* Borro toda la pantalla */
	ReseteaMatriz((tipo_pantalla*)(&(p_arkanoPi->pantalla)));

	/* Pinta los diferentes elementos del juego en la matriz de la pantalla del juego */
	PintaLadrillos(&p_arkanoPi->ladrillos,&(p_arkanoPi->pantalla));
	PintaRaqueta(&(p_arkanoPi->raqueta),&( p_arkanoPi->pantalla));
	PintaPelota(&(p_arkanoPi->pelota),&(p_arkanoPi->pantalla));
    /* Si hay un paquete generado lo pinto en la pantalla */
    if(p_arkanoPi->paquete.encendido==1){
        PintaPaquete(&p_arkanoPi->paquete,&(p_arkanoPi->pantalla));
    }
    
	/* Pinta la pantalla por el terminal */
	PintaPantallaPorTerminal(&(p_arkanoPi->pantalla));
}

/**
 * void InicializaArkanoPi(tipo_arkanoPi *p_arkanoPi): metodo encargado de la inicializacion
 * de toda variable o estructura de datos especificamente ligada al desarrollo del juego y su 
 * visualizacion.
*/
void InicializaArkanoPi(tipo_arkanoPi *p_arkanoPi) {

	ReseteaMatriz((tipo_pantalla*)(&(p_arkanoPi->pantalla)));
	ReseteaLadrillos((tipo_pantalla*)(&(p_arkanoPi->ladrillos)));
	ReseteaPelota((tipo_pelota*)(&(p_arkanoPi->pelota)));
	ReseteaRaqueta((tipo_raqueta*)(&(p_arkanoPi->raqueta)));
    p_arkanoPi->paquete.encendido=0;
	ActualizaPantalla(p_arkanoPi);

}

/**
 * int CalculaLadrillosRestantes(tipo_pantalla *p_ladrillos): funcion encargada de evaluar
 * el estado de ocupacion del area de juego por los ladrillos y devolver el numero de estos.
*/
int CalculaLadrillosRestantes(tipo_pantalla *p_ladrillos) {
	int num_ladrillos_restantes = 0;

	int i, j = 0;

		for(i=0;i<MATRIZ_ANCHO;i++) {
			for(j=0;j<MATRIZ_ALTO;j++) {
				if(p_ladrillos->matriz[i][j]==1){
					num_ladrillos_restantes++;
				}
			}
	    }

	return num_ladrillos_restantes;
}

