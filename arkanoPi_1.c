/** 
  * Author: Daniel Suarez Souto y Alberto Jimenez Aliste
  * File Name          : arkanoPi_1.c
*/

/*  Includes ------------------------------------------------------------------*/
#include "arkanoPi_1.h"
#include "arkanoPiLib.h"
#include "pong.h"
#include <stdio.h>
#include <stdlib.h>

/*  Private variables ---------------------------------------------------------*/
static tipo_juego juego;
volatile int flag=0;
static tmr_t* timer;
static tmr_t* timer_pelota;
static tmr_t* timer_joystick;
static tmr_t* timer_matricial;
int col=0; /*  Contador de columnas del timer  */
int array_f[]={GPIO_FILA_0,GPIO_FILA_1,GPIO_FILA_2,GPIO_FILA_3,GPIO_FILA_4,GPIO_FILA_5,GPIO_FILA_6};
int debounceTimeIzq=0;
int debounceTimeDer=0;
volatile int tiempo_pelota=0;

/* Mejora del potenciometro */
#define SPI_ADC_CH 0
#define SPI_ADC_FREQ 1000000
int tiempo_joystick=75;
float voltaje_anterior=0.0;
int rango=0;
int juego_seleccionado=1; /* Vale '1' si seleccionamos ArkanoPi y '0' si seleccionamos Pong */
/* Mejora de paquete con poder */
int nladrillos=0;/* Ladrillos que ha destruido la raqueta */
int ladrillos_necesarios=0;/* Ladrillos que para conseguir el paquete */
static tmr_t* timer_poder;
int tiempo_poder=30000;/* Duracion del poder 30 segundos */
/* Mejora del teclado matricial */
int tiempo_matricial=50;
int array_matricial_f[]={GPIO_TECLADO_FILA_1,GPIO_TECLADO_FILA_2,GPIO_TECLADO_FILA_3,GPIO_TECLADO_FILA_4};
/* Mejora del sonido de rebotes */
static tmr_t* timer_sonido;

int mensaje_inicial[MATRIZ_ANCHO][MATRIZ_ALTO] = {
	    {1,1,1,1,1,1,1},
		{0,0,0,1,0,0,0},
		{0,0,0,1,0,0,0},
		{0,0,0,1,0,0,0},
		{1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0},
		{1,1,1,1,0,0,1},
};
int mensaje_final_pong1[MATRIZ_ANCHO][MATRIZ_ALTO] = {
		{0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0},
		{0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0},
		{0,0,1,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,1,1,1,0,0},
		{0,0,0,0,1,0,0},
		{0,0,0,0,0,0,0},
};		
int mensaje_final_pong2[MATRIZ_ANCHO][MATRIZ_ALTO] = {
		{0,0,0,0,0,0,0},
		{0,0,1,1,1,0,0},
		{0,0,0,0,1,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0},
		{0,0,1,0,0,0,0},
		{0,0,0,1,0,0,0},
		{0,0,1,0,0,0,0},		
};		
/*  Action Functions ---------------------------------------------------------*/
/**
 * void InicializaDificultad (fsm_t* this):funcion encargada de imprimir por pantalla
 * los diferentes niveles de dificultad.
 */
void InicializaDificultad (fsm_t* this) {
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_ARKANOPI;/* Desactivo FLAG_ARKANOPI */
	flag &= ~FLAG_PONG;/* Desactivo FLAG_ARKANOPI */
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	printf("\n Seleccione el nivel de dificultad: \n");
	printf(" Teclado: - Facil :   'f'     Teclado Matricial: - Facil :   '1'\n");
	printf("          - Medio :   'm'                        - Medio :   '4'\n");
	printf("          - Dificil : 'd'                        - Dificil : '7'\n");
	printf(" Puede pausar el juego y reanudarlo con la tecla 'A' del teclado matricial \n");

	piUnlock (STD_IO_BUFFER_KEY);
}

/**
 * void InicializaJuego (void): funcion encargada de llevar a cabo
 * la oportuna inicializacion de toda variable o estructura de datos
 * que resulte necesaria para el desarrollo del juego.
*/
void InicializaJuego (fsm_t* this) {
		piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		flag &= ~FLAG_DIFICULTAD;/* Desactivo FLAG_DIFICULTAD */
		piUnlock (FLAGS_KEY);
		/* Inicializa los atributos de Juego  */
		juego_seleccionado=1;
		InicializaArkanoPi(&(juego.arkanoPi));
		juego.teclaPulsada='\0';
        ladrillos_necesarios=random()%10;/*10 porwue el numero de ladrillos en un principio son 20*/
        tmr_startms(timer_pelota,tiempo_pelota);
}
/**
 * void InicializaJuegoPong (void): funcion encargada de llamar al metodo
 * encargado de realizar la oportuna inicializacion de toda variable o
 * estructura de datos que resulte necesaria para el desarrollo del Pong.
*/
void InicializaJuegoPong (fsm_t* this) {
		piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		flag &= ~FLAG_DIFICULTAD;/* Desactivo FLAG_DIFICULTAD */
		piUnlock (FLAGS_KEY);
		juego_seleccionado=0;
	  	InicializaPong(&(juego.pong));
	  	tmr_startms(timer_pelota,tiempo_pelota);

}
/** 
 * void MueveRaquetaIzquierda (void): funcion encargada de ejecutar
 * el movimiento hacia la izquierda contemplado para la raqueta.
 * Debe garantizar la viabilidad del mismo mediante la comprobacion
 * de que la nueva posicion correspondiente a la raqueta no suponga
 * que esta rebase o exceda los limites definidos para el area de juego
 * (i.e. al menos uno de los leds que componen la raqueta debe permanecer
 * visible durante  el transcurso de la partida).
*/
void MueveRaquetaIzquierda (fsm_t* this) {
	
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_RAQUETA_IZQUIERDA;/* Desactivo FLAG_RAQUETA_IZQUIERDA */
	piUnlock (FLAGS_KEY);
	/* DEBUG  */
	/* piLock (STD_IO_BUFFER_KEY);
	printf("Entra en Raqueta Izquierda");
	piUnlock (STD_IO_BUFFER_KEY); */
	/*  /DEBUG  */

	/*  Comprobamos si podemos modificar la posicion de la raqueta la izquierda */
	if(juego.arkanoPi.raqueta.x>-(juego.arkanoPi.raqueta.ancho-1)){
		/* Movemos la raqueta a la izquierda */
		juego.arkanoPi.raqueta.x--;
		/* Actualizamos la pantalla con la nueva posicion */
		piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
		ActualizaPantalla(&(juego.arkanoPi));
		/* DEBUG */
		/* printf("Ha entrado en ActualizaPantalla"); */
		/* /DEBUG */
		piUnlock (STD_IO_BUFFER_KEY);
	}

}
/**
 * void MueveRaquetaDAbajo (void): funcion encargada de ejecutar
 * el movimiento hacia abajo contemplado para la raqueta derecha del Pong.
 * Debe garantizar la viabilidad del mismo mediante la comprobacion
 * de que la nueva posicion correspondiente a la raqueta no suponga
 * que esta rebase o exceda los limites definidos para el area de juego
*/
void MueveRaquetaDAbajo(fsm_t* this) {
		piLock(FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
        flag &= ~FLAG_RAQUETA_IZQUIERDA; /* Desactivo FLAG_RAQUETA_IZQUIERDA */
        piUnlock(FLAGS_KEY);
        /*  Comprobamos si podemos modificar la posicion de la raqueta hacia abajo */
		if(juego.pong.raquetaD.y<4/*6*/){
			/* Movemos la raqueta hacia abajo */
			juego.pong.raquetaD.y++;
			/* Actualizamos la pantalla con la nueva posicion */
			piLock(STD_IO_BUFFER_KEY);
			ActualizaPantallaPong( &(juego.pong));
			piUnlock(STD_IO_BUFFER_KEY);
		}
		
}
/**
 * void MueveRaquetaIArriba (void): funcion encargada de ejecutar
 * el movimiento hacia arriba contemplado para la raqueta izquierda del Pong.
 * Debe garantizar la viabilidad del mismo mediante la comprobacion
 * de que la nueva posicion correspondiente a la raqueta no suponga
 * que esta rebase o exceda los limites definidos para el area de juego
*/
void MueveRaquetaIArriba (fsm_t* this){
	 	piLock(FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
      	flag &= ~FLAG_RAQUETA_IZQUIERDA_PONG;/* Desactivo FLAG_RAQUETA_IZQUIERDA_PONG */
        piUnlock(FLAGS_KEY);
        /*  Comprobamos si podemos modificar la posicion de la raqueta hacia arriba */
		if(juego.pong.raquetaI.y>0){
			/* Movemos la raqueta hacia arriba */
			juego.pong.raquetaI.y--;
			piLock(STD_IO_BUFFER_KEY);
			/* Actualizamos la pantalla con la nueva posicion */
			ActualizaPantallaPong( &(juego.pong));
			piUnlock(STD_IO_BUFFER_KEY);
		}
		
}
/** 
 * void MueveRaquetaDerecha (void): funcion similar a la anterior
 * encargada del movimiento hacia la derecha.
*/
void MueveRaquetaDerecha (fsm_t* this) {
	
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_RAQUETA_DERECHA;/* Desactivo FLAG_RAQUETA_DERECHA */
	piUnlock (FLAGS_KEY);

	/* Compruebo si puedo mover la raqueta a la derecha */
	if(juego.arkanoPi.raqueta.x<9){
		/* Muevo la raqueta a la derecha */
		juego.arkanoPi.raqueta.x++;
		/* Actualizamos la pantalla con la nueva posicion */
		piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
		ActualizaPantalla(&(juego.arkanoPi));
		piUnlock (STD_IO_BUFFER_KEY);
	}


}
/**
 * void MueveRaquetaDArriba (void): funcion encargada de ejecutar
 * el movimiento hacia arriba contemplado para la raqueta derecha del Pong.
 * Debe garantizar la viabilidad del mismo mediante la comprobacion
 * de que la nueva posicion correspondiente a la raqueta no suponga
 * que esta rebase o exceda los limites definidos para el area de juego
*/
void MueveRaquetaDArriba (fsm_t* this) {
	
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_RAQUETA_DERECHA;/* Desactivo FLAG_RAQUETA_DERECHA */
	piUnlock (FLAGS_KEY);

	/* Compruebo si puedo mover la raqueta hacia arriba */
	if(juego.pong.raquetaD.y>0){
		/* Muevo la raqueta hacia arriba */
		juego.pong.raquetaD.y--;
		piLock(STD_IO_BUFFER_KEY);
		/* Actualizamos la pantalla con la nueva posicion */
		ActualizaPantallaPong( &(juego.pong));
		piUnlock(STD_IO_BUFFER_KEY);
	}
	

}
/**
 * void MueveRaquetaIAbajo (void): funcion encargada de ejecutar
 * el movimiento hacia abajo contemplado para la raqueta izquierda del Pong.
 * Debe garantizar la viabilidad del mismo mediante la comprobacion
 * de que la nueva posicion correspondiente a la raqueta no suponga
 * que esta rebase o exceda los limites definidos para el area de juego
*/
void MueveRaquetaIAbajo (fsm_t* this) {
	piLock(FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_RAQUETA_DERECHA_PONG;/* Desactivo FLAG_RAQUETA_DERECHA_PONG */
    piUnlock(FLAGS_KEY);
    /* Compruebo si puedo mover la raqueta hacia abajo */
	if(juego.pong.raquetaI.y<4){
		/* Muevo la raqueta hacia abajo */
		juego.pong.raquetaI.y++;
		piLock(STD_IO_BUFFER_KEY);
		/* Actualizamos la pantalla con la nueva posicion */
		ActualizaPantallaPong( &(juego.pong));
		piUnlock(STD_IO_BUFFER_KEY);
	
	}
		
}
/**
 * void MovimientoPelota (void): funcion encargada de actualizar la
 * posicion de la pelota conforme a la trayectoria definida para esta.
 * Para ello debera identificar los posibles rebotes de la pelota para,
 * en ese caso, modificar su correspondiente trayectoria (los rebotes
 * detectados contra alguno de los ladrillos implicaran adicionalmente
 * la eliminacion del ladrillo). Del mismo modo, debera tambien
 * identificar las situaciones en las que se da por finalizada la partida:
 * bien porque el jugador no consiga devolver la pelota, y por tanto esta
 * rebase el limite inferior del area de juego, bien porque se agoten
 * los ladrillos visibles en el area de juego.
*/
void MovimientoPelota (fsm_t* this) {
	
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_PELOTA;/* Desactivo FLAG_PELOTA */
	piUnlock (FLAGS_KEY);

	/* Posicion de la pelota */
	int x_pelota=juego.arkanoPi.pelota.x;
	int y_pelota=juego.arkanoPi.pelota.y;
	/* Velocidad de la pelota */
	int xv_pelota=juego.arkanoPi.pelota.xv;
	int yv_pelota=juego.arkanoPi.pelota.yv;

	/* Posicion de la raqueta */
	int x_raqueta=juego.arkanoPi.raqueta.x;
	int y_raqueta=juego.arkanoPi.raqueta.y;
	int ancho_raqueta=juego.arkanoPi.raqueta.ancho;

	/* Comprobar si se ha perdido o eliminado todos los ladrillos */
	if((CalculaLadrillosRestantes(&(juego.arkanoPi.ladrillos))==0)||
	    (y_pelota==6)){
	    	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	    	/* Activo FLAG_FINAL_JUEGO */
	    	flag |=FLAG_FINAL_JUEGO;
	    	piUnlock (FLAGS_KEY);
	    }

	/* Comprueba el rebote con las paredes laterales */
	if((x_pelota ==0 && xv_pelota<0) || /* Rebota con la pared izquierda */
			(x_pelota ==9 && xv_pelota>0)){ /* Rebota con la pared derecha */

		/* Modificamos la componente x de la velocidad de la pelota */
		juego.arkanoPi.pelota.xv=-xv_pelota;
		/* Producimos un sonido de rebote */
		softToneWrite(GPIO_SOFTTONE,750);
		tmr_startms(timer_sonido,200);
		/* Actualizamos la pantalla con la nueva posicion */
		piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
		ActualizaPantalla(&(juego.arkanoPi));
		piUnlock (STD_IO_BUFFER_KEY);
		/* Salimos del metodo */
		return;
	}

	/* Comprueba el rebote con ladrillos  */
	if(juego.arkanoPi.ladrillos.matriz
			[x_pelota+xv_pelota][y_pelota+yv_pelota]==1){/* Rebota */

		juego.arkanoPi.ladrillos.matriz /* Eliminar el ladrillo */
					[x_pelota+xv_pelota][y_pelota+yv_pelota]=0;
        /* Incremento el numero de ladrillos destruidos */
        nladrillos++;
        /* Producimos un sonido de rebote */
        softToneWrite(GPIO_SOFTTONE,250);
        tmr_startms(timer_sonido,200);
        /* Compruebo si se han conseguido los ladrillos necesarios para generar un paquete */
        if(nladrillos==ladrillos_necesarios){
            juego.arkanoPi.paquete.encendido=1;
            juego.arkanoPi.paquete.x=x_pelota;
            juego.arkanoPi.paquete.y=y_pelota;
        }
		/* Modificamos la componente y de la velocidad de la pelota */
		juego.arkanoPi.pelota.yv=1;
		/* Actualizamos la pantalla con la nueva posicion */
		piLock (STD_IO_BUFFER_KEY);/*  Bloqueo este mutex para poder imprimir por pantalla */
		ActualizaPantalla(&(juego.arkanoPi));
		piUnlock (STD_IO_BUFFER_KEY);
		/* Salimos del metodo  */
		return;
	}
	/* Comprueba el rebote con la pared superior(sin ladrillo) */
	if((y_pelota==0)&&(yv_pelota==-1)){
		/* Modificamos la componente y de la velocidad de la pelota */
		juego.arkanoPi.pelota.yv=1;
		/* Producimos un sonido de rebote */
		softToneWrite(GPIO_SOFTTONE,750);
		tmr_startms(timer_sonido,200);
		/* Actualizamos la pantalla con la nueva posicion */
		piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
		ActualizaPantalla(&(juego.arkanoPi));
		piUnlock (STD_IO_BUFFER_KEY);
		/* Salimos del metodo */
		return;
	}
	/* Comprueba el rebote con la raqueta */
	if(juego.arkanoPi.pantalla.matriz
				[x_pelota+xv_pelota][y_pelota+yv_pelota]==1 && (y_pelota)==5){/* Rebota con la raqueta */
		/* Comprobamos si hay un paquete creado */
		if(juego.arkanoPi.paquete.encendido==1){
				/* Comprobacion de si estamos en la situacion de que raqueta coge el paquete */
		        if(juego.arkanoPi.paquete.y+1==y_raqueta){
		            if(juego.arkanoPi.paquete.x==x_raqueta || juego.arkanoPi.paquete.x==(x_raqueta+ancho_raqueta-1)||juego.arkanoPi.paquete.x==(x_raqueta+ancho_raqueta-2)||juego.arkanoPi.paquete.x+1==x_raqueta){
		                /* Apagamos el paquete */
		                juego.arkanoPi.paquete.encendido=0;
		                /* Aumento el ancho */
		                juego.arkanoPi.raqueta.ancho=4;
		                /* Iniciamos el tiempo de poder */
		                tmr_startms(timer_poder,tiempo_poder);

		            }
		        }
		}
		/* Producimos un sonido de rebote */
		softToneWrite(GPIO_SOFTTONE,750);
		tmr_startms(timer_sonido,200);
		if(((x_pelota+xv_pelota)==x_raqueta)&&
				((y_pelota+yv_pelota)==y_raqueta)){/*  Pelota rebota con raqueta[0] */

			/* Modificamos la componente x e y de la velocidad de la pelota */
			juego.arkanoPi.pelota.xv=-1;
			juego.arkanoPi.pelota.yv=-1;
			/* Actualizamos la pantalla con la nueva posicion */
			piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
			ActualizaPantalla(&(juego.arkanoPi));
			piUnlock (STD_IO_BUFFER_KEY);
			/* Salimos del metodo */
			return;
		}
		if(((x_pelota+xv_pelota)==(x_raqueta+ancho_raqueta-1))&&
				((y_pelota+yv_pelota)==y_raqueta)){/*  Pelota rebota con raqueta[2] */
			/* Modificamos la componente x e y de la velocidad de la pelota */
			juego.arkanoPi.pelota.xv=1;
			juego.arkanoPi.pelota.yv=-1;
			/* Actualizamos la pantalla con la nueva posicion */
			piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
			ActualizaPantalla(&(juego.arkanoPi));
			piUnlock (STD_IO_BUFFER_KEY);
			/* Salimos del metodo */
			return;
		}
		/* Pelota rebota con raqueta[1] */
		/* Modificamos la componente x e y de la velocidad de la pelota */
		juego.arkanoPi.pelota.xv=0;
		juego.arkanoPi.pelota.yv=-1;
		/* Actualizamos la pantalla con la nueva posicion */
		piLock (STD_IO_BUFFER_KEY); /* Bloqueo este mutex para poder imprimir por pantalla */
		ActualizaPantalla(&(juego.arkanoPi));
		piUnlock (STD_IO_BUFFER_KEY);
		/* Salimos del metodo */
		return;
	}
	/* No rebota con nada */
    /*  Comprobamos si hay un paquete creado */
    if(juego.arkanoPi.paquete.encendido==1){
    	/* Comprobacion de si estamos en la situacion de que raqueta coge el paquete */
        if(juego.arkanoPi.paquete.y+1==y_raqueta){
            if(juego.arkanoPi.paquete.x==x_raqueta || juego.arkanoPi.paquete.x==(x_raqueta+ancho_raqueta-1)||juego.arkanoPi.paquete.x==(x_raqueta+ancho_raqueta-2)||juego.arkanoPi.paquete.x+1==x_raqueta){
                /* Apagamos el paquete */
                juego.arkanoPi.paquete.encendido=0;
                /* Aumento el ancho */
                juego.arkanoPi.raqueta.ancho=4;
                /* Iniciamos el timer de poder */
                tmr_startms(timer_poder,tiempo_poder);
                
            }
        }
        /* Comprobamos que el paquete no esta fuera de los limites de juego */
        if(juego.arkanoPi.paquete.y==6){
        	/* Apagamos el paquete */
        	juego.arkanoPi.paquete.encendido=0;
        	/* Reseteamos el numero de ladrillos destruidos por la raqueta */
        	 nladrillos=0;
        	 /* Reseteamos el numero de ladrillos necesarios para que caiga un paquete */
        	 ladrillos_necesarios=2;//random()%10;
        	 }
        /* Actualizamos la posicion del paquete */	 
        juego.arkanoPi.paquete.y++;
    }



	/*  Actualizo posicion de pelota */
	juego.arkanoPi.pelota.x+=xv_pelota;
	juego.arkanoPi.pelota.y+=yv_pelota;

	/* Actualizamos la pantalla con la nueva posicion */
	piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
    ActualizaPantalla(&(juego.arkanoPi));
	piUnlock (STD_IO_BUFFER_KEY);
}
/**
 * void MovimientoPelotaJuegoPong (void): funcion encargada de actualizar la
 * posicion de la pelota conforme a la trayectoria definida para esta.
 * Para ello debera identificar los posibles rebotes de la pelota para,
 * en ese caso, modificar su correspondiente trayectoria. Del mismo modo,
 * debera tambien identificar las situaciones en las que se da por finalizada la partida.
*/
void MovimientoPelotaJuegoPong (fsm_t* this){

	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_PELOTA;/* Desactivo FLAG_PELOTA */
	piUnlock (FLAGS_KEY);

	/* Posicion de la pelota */
	int x_pelota=juego.pong.pelota.x;
	int y_pelota=juego.pong.pelota.y;
	/* Velocidad de la pelota */
	int xv_pelota=juego.pong.pelota.xv;
	int yv_pelota=juego.pong.pelota.yv;

	/* Posicion de la raqueta Derecha */
	int x_raquetaD=juego.pong.raquetaD.x;
	int y_raquetaD=juego.pong.raquetaD.y;
	
    
	/* Posicion de la raqueta Izquierda */
	int x_raquetaI=juego.pong.raquetaI.x;
	int y_raquetaI=juego.pong.raquetaI.y;
	
	


    /* Comprueba si la pelota rebota con las paredes inferior y superior */
    if((y_pelota==0 && yv_pelota==-1)|| (y_pelota==6 && yv_pelota==1)){
    	/* Modificamos la componente velocidad de la pelota */
    	juego.pong.pelota.yv = -(yv_pelota);
    	/* Producimos un sonido de rebote */
    	softToneWrite(GPIO_SOFTTONE,750);
    	tmr_startms(timer_sonido,200);
    	return;
    }


	//choque con la raqueta
	if(((x_pelota==1)&& (xv_pelota==-1))||((x_pelota==8)&& (xv_pelota==1))) {
		if(juego.pong.pantalla.matriz[x_pelota+ xv_pelota][y_pelota + yv_pelota] == 1){ //rebota con raqueta
			/* Producimos un sonido de rebote */
			softToneWrite(GPIO_SOFTTONE,750);
			tmr_startms(timer_sonido,200);
			/* Comprobamos con que posicion de la raqueta rebota */
			if((((y_pelota + yv_pelota) == (y_raquetaD)) && ((x_pelota + xv_pelota) == (x_raquetaD))) || (((y_pelota + yv_pelota) == (y_raquetaI)) && ((x_pelota + xv_pelota) == (x_raquetaI)))){ //rebota con raqueta[0]
				/* Modificamos la componente velocidad de la pelota */
				juego.pong.pelota.xv=-(xv_pelota);
				juego.pong.pelota.yv=1;
				piLock(STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
				/* Actualizamos la pantalla con la nueva posicion */
				ActualizaPantallaPong( &(juego.pong));
				piUnlock(STD_IO_BUFFER_KEY);
				return;
			}
			else if((((y_pelota + yv_pelota) == (y_raquetaD +1)) && ((x_pelota + xv_pelota)== (x_raquetaD))) || (((y_pelota + yv_pelota) == (y_raquetaI +1)) && ((x_pelota+ xv_pelota)== (x_raquetaI)))){ //rebota con raqueta[1]
				/* Modificamos la componente velocidad de la pelota */
				juego.pong.pelota.xv=-(xv_pelota);
				juego.pong.pelota.yv=0;
				piLock(STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
				/* Actualizamos la pantalla con la nueva posicion */
				ActualizaPantallaPong( &(juego.pong));
				piUnlock(STD_IO_BUFFER_KEY);
				return;
			}
			else{ //rebota con raqueta[2]
				/* Modificamos la componente velocidad de la pelota */
				juego.pong.pelota.xv=-(xv_pelota);
				juego.pong.pelota.yv=-1;
				piLock(STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
				/* Actualizamos la pantalla con la nueva posicion */
				ActualizaPantallaPong( &(juego.pong));
				piUnlock(STD_IO_BUFFER_KEY);
				return;
			}
		}
	}

	/*  Actualizo posicion de pelota */
	juego.pong.pelota.x += xv_pelota;
	juego.pong.pelota.y += yv_pelota;

	/* Comprobar si se ha finalizado el juego */
	if(x_pelota<=0 || x_pelota>=9){
		flag|=FLAG_FINAL_JUEGO;
	}

	piLock(STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
	/* Actualizamos la pantalla con la nueva posicion */
	ActualizaPantallaPong(&(juego.pong));
	piUnlock(STD_IO_BUFFER_KEY);



}
/**
 * void PausarJuego (fsm_t* this): funcion encargada de parar el tiempo de la pelota para 
 * el estado de pausa del juego.
*/
void PausarJuego (fsm_t* this){

	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_PAUSE;/* Desactivo FLAG_PAUSE */
	flag &= ~FLAG_PAUSE_PONG;/* Desactivo FLAG_PAUSE_PONG */
	piUnlock (FLAGS_KEY);
	/* DEBUG */
	/*piLock (STD_IO_BUFFER_KEY);
	printf("Juego pausado \n");
	piUnlock (STD_IO_BUFFER_KEY);*/
	/* /DEBUG */
	piLock (STD_IO_BUFFER_KEY);
	printf("Menu de pausa:\n");
	printf(" Puede cambiar el nivel de dificultado pulsando: \n");
	printf(" Teclado: - Facil :   'f'     Teclado Matricial: - Facil :   '1'\n");
	printf("          - Medio :   'm'                        - Medio :   '4'\n");
	printf("          - Dificil : 'd'                        - Dificil : '7'\n");
	printf(" Puede pausar el juego y reanudarlo con la tecla 'A' del teclado matricial \n");
	piUnlock (STD_IO_BUFFER_KEY);
	/* Paramos el timer de la pelota */
	tmr_stop(timer_pelota);
}
/**
 * void ReanudarJuego (fsm_t* this): funcion encargada de reiniciar el timer de la pelota 
 * para reanudar el juego.
*/
void ReanudarJuego (fsm_t* this){

	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_PAUSE;/* Desactivo FLAG_PAUSE */
	flag &= ~FLAG_PAUSE_PONG;/* Desactivo FLAG_PAUSE_PONG */
	flag &= ~FLAG_DIFICULTAD;/* Desactivo FLAG_DIFICULTAD */
	piUnlock (FLAGS_KEY);

	/* DEBUG */
	/*piLock (STD_IO_BUFFER_KEY);
	printf("Timer destruido \n");
	piUnlock (STD_IO_BUFFER_KEY);*/

	/* Reinicio el timer de la pelota */
	tmr_init(timer_pelota,timer_isr_pelota);
	tmr_startms(timer_pelota,tiempo_pelota);


}
/**
 * void FinalJuego (void): funcion encargada de mostrar en la ventana de
 * terminal los mensajes necesarios para informar acerca del resultado del juego.
*/ 
void FinalJuego (fsm_t* this) {

	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_FINAL_JUEGO;/* Desactivo FLAG_FINAL_JUEGO */
	piUnlock (FLAGS_KEY);

	int ladrillos_restantes=CalculaLadrillosRestantes(&(juego.arkanoPi.ladrillos));
	if(ladrillos_restantes==0){/* Victoria */
		piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
		printf("\nVictoria!!\n");
		piUnlock (STD_IO_BUFFER_KEY);
	}else{/* Derrota */
		piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
		printf("\nOH!Intentalo de nuevo\n");
		piUnlock (STD_IO_BUFFER_KEY);
	}
	tipo_pantalla pantalla_inicial;/* Variable tipo pantalla para pintar el mensaje inicial */
/* Modifico el atributo matriz de pantalla_inicial para que sea igual a la matriz mensaje_inicial */
	memcpy((void*) pantalla_inicial.matriz,(const void*) mensaje_inicial, sizeof(mensaje_inicial));
	/* Pinto mensaje inicial por pantalla */
	piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
	PintaMensajeInicialPantalla (&(juego.arkanoPi.pantalla), &(pantalla_inicial));
	printf(" Seleccione el juego: \n");
	printf(" - ArkanoPi : 'a' \n");
	printf(" - Pong : 'p' \n");
	piUnlock (STD_IO_BUFFER_KEY);
}
/**
 * void FinalJuegoPong (void): funcion encargada de mostrar en la ventana de
 * terminal los mensajes necesarios para informar acerca del resultado del juego.
*/
void FinalJuegoPong (fsm_t* this) {

	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_FINAL_JUEGO;/* Desactivo FLAG_FINAL_JUEGO */
	piUnlock (FLAGS_KEY);
	if(juego.pong.pelota.x<=0){
		tipo_pantalla pantalla_final;/* Variable tipo pantalla para pintar el mensaje final */
		/* Modifico el atributo matriz de pantalla_final para que sea igual a la matriz mensaje_final_pong2 */
		memcpy((void*) pantalla_final.matriz,(const void*) mensaje_final_pong2, sizeof(mensaje_final_pong2));
		PintaMensajeFinalPantalla (&juego.pong.pantalla,&(pantalla_final));
	}else{
		tipo_pantalla pantalla_final;/* Variable tipo pantalla para pintar el mensaje final */
		/* Modifico el atributo matriz de pantalla_final para que sea igual a la matriz mensaje_final_pong1 */
		memcpy((void*) pantalla_final.matriz,(const void*) mensaje_final_pong1, sizeof(mensaje_final_pong1));
		PintaMensajeInicialPantalla (&juego.pong.pantalla,&(pantalla_final));
	}
	piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
	printf(" Seleccione el juego: \n");
	printf(" - ArkanoPi : 'a' \n");
	printf(" - Pong : 'p' \n");
	piUnlock (STD_IO_BUFFER_KEY);

}

/**
 * void ReseteaJuego (void): funcion encargada de llevar a cabo la
 * reinicializacion de cuantas variables o estructuras resulten
 * necesarias para dar comienzo a una nueva partida.
*/
void ReseteaJuego (fsm_t* this) {

	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_ARKANOPI;/* Desactivamos FLAG_ARKANOPI  */
	flag &= ~FLAG_PONG;/* Desactivamos FLAG_PONG  */
	piUnlock (FLAGS_KEY);
	/* Variable local */
	tipo_pantalla pantalla_inicial;/* Variable tipo pantalla para pintar el mensaje inicial */
	/* Modifico el atributo matriz de pantalla_inicial para que sea igual a la matriz mensaje_inicial */
	memcpy((void*) pantalla_inicial.matriz,(const void*) mensaje_inicial, sizeof(mensaje_inicial));
	/* Pinto mensaje inicial por pantalla */
	piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
	PintaMensajeInicialPantalla (&(juego.arkanoPi.pantalla), &(pantalla_inicial));
	piUnlock (STD_IO_BUFFER_KEY);
	piLock (STD_IO_BUFFER_KEY);
	printf("\n Seleccione el nivel de dificultad: \n");
	printf("\n Seleccione el nivel de dificultad: \n");
	printf(" Teclado: - Facil :   'f'     Teclado Matricial: - Facil :   '1'\n");
	printf("          - Medio :   'm'                        - Medio :   '4'\n");
	printf("          - Dificil : 'd'                        - Dificil : '7'\n");
	printf(" Puede pausar el juego y reanudarlo con la tecla 'A' del teclado matricial \n");
	piUnlock (STD_IO_BUFFER_KEY);
}
/*
* void lectura_ADC (fsm_t* this):funcion que lee el valor de entrada en el ADC MAX1246, y en funcion de este
* valor activo FLAG_RAQUETA_DERECHA o FLAG_RAQUETA_IZQUIERDA.
* voltaje_medido>voltaje_anterior+error -> derecha
* voltaje_medido<voltaje_anterior-error -> izquierda
*/
void lectura_ADC (fsm_t* this){
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	flag &= ~FLAG_ADC;/* Desactivamos FLAG_ADC  */
	piUnlock (FLAGS_KEY);
	/* Reinicio el timer */
	tmr_startms(timer_joystick,tiempo_joystick);
	 /* Buffer lectura escritura SPI */
	 unsigned char ByteSPI[3];
	 /* Control operacion SPI */ 
	 //int resultado_SPI = 0;
	 /* Valor medido */
	 float voltaje_medido = 0.0; 
	 /* Configuracion ADC (10011111 unipolar, 0-2.5v, canal 0, salida 1), bipolar 0b10010111 */
	 ByteSPI[0] = 0b10011111; 
	 ByteSPI[1] = 0b0;
	 ByteSPI[2] = 0b0;
	 /* Enviamos y leemos tres bytes */
	  wiringPiSPIDataRW (SPI_ADC_CH, ByteSPI, 3);
	 usleep(20);
	 /* Nos quedamos con los bits que nos interesan para la conversion */
	 int salida_SPI = ((ByteSPI[1] << 6) | (ByteSPI[2] >> 2 )) & 0xFFF;
	 /* Pasamos los bits que recividos del ADC a voltios */
	 voltaje_medido = 2*2.50 * (((float) salida_SPI)/4095.0);

	 /* DEBUG */
	 /*piLock (STD_IO_BUFFER_KEY);
	 printf("voltaje_medido: %f",voltaje_medido);
	 piUnlock (STD_IO_BUFFER_KEY);*/
	 /* /DEBUG */

	 /* En caso de que estemos en la primeramedida debemos inicializar la variable global voltaje_anterior */
	 if(voltaje_anterior==0.0){voltaje_anterior=voltaje_medido;}

	 /* DEBUG */
	 /*piLock (STD_IO_BUFFER_KEY);
	 printf("voltaje_anterior: %f",voltaje_anterior);
	 piUnlock (STD_IO_BUFFER_KEY);*/
	 /* /DEBUG */

	 /* Si la medida es mayor que la anterior activamos el FLAG_RAQUETA_DERECHA */
	 if(voltaje_medido>voltaje_anterior+0.15){
		 	piLock (FLAGS_KEY);
		 	flag |= FLAG_RAQUETA_DERECHA;
		 	piUnlock (FLAGS_KEY);
		 	voltaje_anterior=voltaje_medido;
		 	return;
	 }
	 /* Si la medida es menor que la anterior activamos el FLAG_RAQUETA_IZQUIERDA */
	 if(voltaje_medido<voltaje_anterior-0.15){
	 		 	piLock (FLAGS_KEY);
	 		 	flag |= FLAG_RAQUETA_IZQUIERDA;
	 		 	piUnlock (FLAGS_KEY);
	 		 	voltaje_anterior=voltaje_medido;
	 		 	return;
	 }

	  /* DEBUG */	
	  /*printf("Lectura ADC MAX1246: %d\n", resultado_SPI);
	  printf("Primer byte: %02X \n", ByteSPI[0]);
	  printf("Segundo Byte: %02X \n", ByteSPI[1]);
	  printf("Tercer byte: %02X \n", ByteSPI[2]);
	  printf("Valor entero: %i \n", salida_SPI);
	  printf("Voltaje medido: %f \n",voltaje_medido);
	  fflush(stdout);*/
	  /* /DEBUG */	  
}


/*  Input Functions ---------------------------------------------------------*/
/**
 * int CompruebaArkanoPi(fsm_t* this): funcion de entrada que comprueba si el FLAG_ArkanoPi esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaArkanoPi(fsm_t* this){
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		result = (flag & FLAG_ARKANOPI);/* Comprobamos si el FLAG_ARKANOPI esta activado */
		piUnlock (FLAGS_KEY);
		return result;
}
/**
 * int CompruebaPong(fsm_t* this): funcion de entrada que comprueba si el FLAG_PONG esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaPong(fsm_t* this){
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		result = (flag & FLAG_PONG);/* Comprobamos si el FLAG_PONG esta activado */
		piUnlock (FLAGS_KEY);
		return result;
}
/**
 * int CompruebaDificultad(fsm_t* this): funcion de entrada que comprueba si el FLAG_DIFICULTAD esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaDificultad(fsm_t* this){
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		result = (flag & FLAG_DIFICULTAD);/* Comprobamos si el FLAG_DIFICULTAD esta activado */
		piUnlock (FLAGS_KEY);
		return result;
}
/**
 * int CompruebaPause(fsm_t* this: funcion de entrada que comprueba si el FLAG_PAUSE esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaPause(fsm_t* this){
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		result = (flag & FLAG_PAUSE);/* Comprobamos si el FLAG_PAUSE esta activado */
		piUnlock (FLAGS_KEY);
		return result;
}
/**
 * int CompruebaPausePong(fsm_t* this: funcion de entrada que comprueba si el FLAG_PAUSE_PONG esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaPausePong(fsm_t* this){
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		result = (flag & FLAG_PAUSE_PONG);/* Comprobamos si el FLAG_PAUSE_PONG esta activado */
		piUnlock (FLAGS_KEY);
		return result;
}
/**
 * int CompruebaTeclaPulsada(fsm_t* this): funcion de entrada que comprueba si el FLAG_TECLA esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaTeclaPulsada(fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_TECLA);/* Comprobamos si el FLAG_TECLA esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}

/**
 * int CompruebaTeclaPelota(fsm_t* this): funcion de entrada que comprueba si el FLAG_PELOTA esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo estA.
*/
int CompruebaTeclaPelota(fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_PELOTA);/* Comprobamos si el FLAG_PELOTA esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}

/**
 * int CompruebaTeclaRaquetaDerecha(fsm_t* this): funcion de entrada que comprueba si el FLAG_RAQUETA_DERECHA esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaTeclaRaquetaDerecha(fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_RAQUETA_DERECHA); /* Comprobamos si el FLAG_RAQUETA_DERECHA esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}
/**
 * int CompruebaTeclaRaquetaDerechaPong(fsm_t* this): funcion de entrada que comprueba si el FLAG_RAQUETA_DERECHA_PONG
 * esta activado. Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaTeclaRaquetaDerechaPong(fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_RAQUETA_DERECHA_PONG); /* Comprobamos si el FLAG_RAQUETA_DERECHA_PONG esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}
/**
 * int CompruebaTeclaRaquetaIzquierda(fsm_t* this): funcion de entrada que comprueba si el FLAG_RAQUETA_IZQUIERDA esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaTeclaRaquetaIzquierda(fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_RAQUETA_IZQUIERDA);/* Comprobamos si el FLAG_RAQUETA_IZQUIERDA esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}
/**
 * int CompruebaTeclaRaquetaIzquierdaPong(fsm_t* this): funcion de entrada que comprueba si el FLAG_RAQUETA_IZQUIERDA_PONG esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaTeclaRaquetaIzquierdaPong (fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_RAQUETA_IZQUIERDA_PONG); /* Comprobamos si el FLAG_RAQUETA_IZQUIERDA_PONG esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}

/**
 * int CompruebaFinalJuego(fsm_t* this): funcion de entrada que comprueba si el FLAG_FINAL_JUEGO esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaFinalJuego(fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_FINAL_JUEGO);/* Comprobamos si el FLAG_FINAL_JUEGO esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}
/**
 * int CompruebaADC(fsm_t* this): funcion de entrada que comprueba si el FLAGS_ADC esta activado.
 * Devuelve un 0 si no esta activado y un 1 si lo esta.
*/
int CompruebaADC(fsm_t* this) {
	/* Variable local */
	int result;
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	result = (flag & FLAG_ADC);/* Comprobamos si el FLAG_ADC esta activado */
	piUnlock (FLAGS_KEY);
	return result;
}
/*  Set up Functions ---------------------------------------------------------*/
/** 
 * void delay_until (unsigned int next): Espera hasta la proxima activacion del reloj
*/
void delay_until (unsigned int next) {
	/* Variable local */
	unsigned int now = millis();
	if (next > now) {
		delay (next - now);
	}
}

/** 
 * int systemSetup (void): procedimiento de configuracion del sistema.
 * Realizara, entra otras, todas las operaciones necesarias para:
 * configurar el uso de posibles librerias (e.g. Wiring Pi),
 * configurar las interrupciones externas asociadas a los pines GPIO,
 * configurar las interrupciones periodicas y sus correspondientes temporizadores,
 * crear, si fuese necesario, los threads adicionales que pueda requerir el sistema
*/
int systemSetup (void) {
	/* Variable local */
	int x = 0;
	piLock (STD_IO_BUFFER_KEY); /* Bloqueo este mutex para poder modificar el valor del flag */

	/*  Sets up the wiringPi library  */
	if (wiringPiSetupGpio () < 0) {
		/* DEBUG */
		printf ("Unable to setup wiringPi\n");
		piUnlock (STD_IO_BUFFER_KEY);
		/* /DEBUG */
		return -1;
	 }
	/*  Sets up the wiringPiSPI library  */ 
	/* Conexion del canal 0 (GPIO 08 en numeracion BCM) a 1 MHz */
	if (wiringPiSPISetup (SPI_ADC_CH, SPI_ADC_FREQ) < 0) { 
		/* DEBUG */
	 	printf ("No se pudo inicializar el dispositivo SPI (CH 0)") ;
	 	/* /DEBUG */
	 	exit (1);
	 	return -2;
	}
	/*  Sets up the softTone library  */
		if (softToneCreate (GPIO_SOFTTONE) != 0) {
			/* DEBUG */
			printf ("Unable to setup softTone\n");
			piUnlock (STD_IO_BUFFER_KEY);
			/* /DEBUG */
			return -3;
		 }
	/*  Creamos y lanzamos thread para exploracion del teclado convencional del PC */
	x = piThreadCreate (thread_explora_teclado);
	if (x != 0) {
		/* DEBUG */
		printf ("it didn't start!!!\n");
		/* /DEBUG */
		return -4;
	}
	piUnlock (STD_IO_BUFFER_KEY);
	return 1;
}

/**
 * void fsm_set(fsm_t* juego_fsm):funcion que inicializa la maquina de estados.
*/
void fsm_setup(fsm_t* juego_fsm) {
	/* Bloqueo este mutex para inicializar la variable global flag */
	piLock (FLAGS_KEY);
	flag = 0;
	piUnlock (FLAGS_KEY);
	/* Bloqueo este mutex para poder imprimir por pantalla */
	piLock (STD_IO_BUFFER_KEY);
	/* Imprimir el mensaje para empezar el juego */
	printf("\n Bienvenido. Seleccione el tipo de juego: \n");
	printf(" - ArkanoPi : 'a' \n");
	printf(" - Pong : 'p' \n");
	piUnlock (STD_IO_BUFFER_KEY);
}

/*  Interruption Attention Functions ------------------------------------------------------------------*/
/**
 * PI_THREAD (thread_explora_teclado): Hebra que se encarga de leer el teclado y en funcion de 
 * ello activar el flag correspondiente para la ejecucion de la maquina de estados.
 * Como argumento le pasamos el nombre de la thread.
*/
PI_THREAD (thread_explora_teclado) {
	piHiPri(0);

	/* Variable local */
	int teclaPulsada;

	/* Bucle infinito */
	while(1) {

			delay(1000); /*  Wiring Pi function: pauses program execution for at least 10 ms */
			piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
			if(kbhit()) { /* Si se pulsa una tecla */
				/* Guardo el valor de la tecla pulsada */
				teclaPulsada = kbread();

				switch(teclaPulsada) {
					case 'i': /* teclaPulsada=i */
						piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
						/* Activo FLAG_RAQUTA_IZQUIERDA */
						flag |= FLAG_RAQUETA_IZQUIERDA;
						piUnlock (FLAGS_KEY);
						/* DEBUG */
						//printf("Has pulsado i");
						/* /DEBUG */
						break;
					case 'o':/* teclaPulsada=o */
						piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
						/* Activo FLAG_RAQUETA_DERECHA */
						flag |= FLAG_RAQUETA_DERECHA;
						piUnlock (FLAGS_KEY);
						/* DEBUG */
						//printf("Has pulsado o");
						/* /DEBUG */
						break; 
					case 'a':/* teclaPulsada=a */	
						piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
						/* Activo FLAG_ARKANOPI */
						flag |= FLAG_ARKANOPI;
						piUnlock (FLAGS_KEY);
						break;
					case 'p':/* teclaPulsada=p */
						piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
						/* Activo FLAG_PONG */
						flag |= FLAG_PONG;
						piUnlock (FLAGS_KEY);
						/* DEBUG */
						//printf("Has pulsado p");
						/* /DEBUG */
						break;
					case 'f':
					    piLock(PELOTA_KEY);/* Bloqueo este mutex para poder modificar el valor de tiempo_pelota */
						/* Modificamos el tiempo de refresco de la posicion de la pelota */
						tiempo_pelota=900;
						piUnlock(PELOTA_KEY);
						piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
						/* Activo FLAG_DIFICULTAD */
						flag |= FLAG_DIFICULTAD;
					    piUnlock (FLAGS_KEY);
						break;
				    case 'm':
				    	piLock(PELOTA_KEY);/* Bloqueo este mutex para poder modificar el valor de tiempo_pelota */
						/* Modificamos el tiempo de refresco de la posicion de la pelota */
				    	tiempo_pelota=600;
				    	piUnlock(PELOTA_KEY);
				    	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
				    	/* Activo FLAG_TECLA */
				    	flag |= FLAG_DIFICULTAD;
				    	piUnlock (FLAGS_KEY);
				    	break;
				    case 'd':
				    	piLock(PELOTA_KEY);/* Bloqueo este mutex para poder modificar el valor de tiempo_pelota */
						/* Modificamos el tiempo de refresco de la posicion de la pelota */
				    	tiempo_pelota=400;
				    	piUnlock(PELOTA_KEY);
				    	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
				    	/* Activo FLAG_TECLA */
				    	flag |= FLAG_DIFICULTAD;
				    	piUnlock (FLAGS_KEY);
				    	break;
					case 'q':/* teclaPulsada=q */
						exit(0);
						break;
					case 's':/* teclaPulsada=s */
						piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
						/* Activo FLAG_TECLA */
						flag |= FLAG_PAUSE_PONG;
						piUnlock (FLAGS_KEY);
						break;
					default:/* Resto de casos */
						printf("INVALID KEY!!!\n");
						break;
				}
			}
			piUnlock (STD_IO_BUFFER_KEY);
		}
}

/*  Timer Functions ------------------------------------------------------------------*/
/**
 * void timer_isr (union sigval value): Metodo que refresca la matriz de leds en funci�n del valor
 * de la matriz de la pantalla del juego.
 * Para realizar su funci�n el metodo escribe 1 y 0 en los pines.
 * Para encender un led los pines de la columna deben contener el numero de la columna el binario 
 * para que el decodificador escriba un 1 en su salida correspondiente y en el pin de su fila escribir un 0.
*/
void timer_isr (union sigval value){
	piHiPri(3);

	/* Reinicio el timer */
	tmr_startms(timer, 1);

		/* DEBUG */
		//printf("Tiempo \n");
		/* /DEBUG */

		switch(col){
				case 0:/* columna=0 */
					/* Escribimos en los pines (0 0 0 0) */
					digitalWrite(GPIO_COL_0,LOW);
					digitalWrite(GPIO_COL_1,LOW);
					digitalWrite(GPIO_COL_2,LOW);
					digitalWrite(GPIO_COL_3,LOW);

					break;

				case 1:/* columna=1 */
					/* Escribimos en los pines (1 0 0 0) */
					digitalWrite(GPIO_COL_0,HIGH);
					digitalWrite(GPIO_COL_1,LOW);
					digitalWrite(GPIO_COL_2,LOW);
					digitalWrite(GPIO_COL_3,LOW);

					break;
				case 2:/* columna=2 */
					/* Escribimos en los pines (0 1 0 0) */
					digitalWrite(GPIO_COL_0,LOW);
					digitalWrite(GPIO_COL_1,HIGH);
					digitalWrite(GPIO_COL_2,LOW);
					digitalWrite(GPIO_COL_3,LOW);

					break;
				case 3:/* columna=3 */
					/* Escribimos en los pines (1 1 0 0) */
					digitalWrite(GPIO_COL_0,HIGH);
					digitalWrite(GPIO_COL_1,HIGH);
					digitalWrite(GPIO_COL_2,LOW);
					digitalWrite(GPIO_COL_3,LOW);

					break;
				case 4:/* columna=4 */
					/* Escribimos en los pines (0 0 1 0) */
					digitalWrite(GPIO_COL_0,LOW);
					digitalWrite(GPIO_COL_1,LOW);
					digitalWrite(GPIO_COL_2,HIGH);
					digitalWrite(GPIO_COL_3,LOW);

					break;
				case 5:/* columna=5 */
					/* Escribimos en los pines (1 0 1 0) */
					digitalWrite(GPIO_COL_0,HIGH);
					digitalWrite(GPIO_COL_1,LOW);
					digitalWrite(GPIO_COL_2,HIGH);
					digitalWrite(GPIO_COL_3,LOW);

					break;
				case 6:/* columna=6 */
					/* Escribimos en los pines (0 1 1 0) */
					digitalWrite(GPIO_COL_0,LOW);
					digitalWrite(GPIO_COL_1,HIGH);
					digitalWrite(GPIO_COL_2,HIGH);
					digitalWrite(GPIO_COL_3,LOW);

					break;
				case 7:/* columna=7 */
					/* Escribimos en los pines (1 1 1 0) */
					digitalWrite(GPIO_COL_0,HIGH);
					digitalWrite(GPIO_COL_1,HIGH);
					digitalWrite(GPIO_COL_2,HIGH);
					digitalWrite(GPIO_COL_3,LOW);

					break;
				case 8:/* columna=8 */
					/* Escribimos en los pines (0 0 0 1) */
					digitalWrite(GPIO_COL_0,LOW);
					digitalWrite(GPIO_COL_1,LOW);
					digitalWrite(GPIO_COL_2,LOW);
					digitalWrite(GPIO_COL_3,HIGH);

					break;
				case 9:/* columna=9 */
					/* Escribimos en los pines (1 0 0 1) */
					digitalWrite(GPIO_COL_0,HIGH);
					digitalWrite(GPIO_COL_1,LOW);
					digitalWrite(GPIO_COL_2,LOW);
					digitalWrite(GPIO_COL_3,HIGH);

					break;
				default:/* Por defecto */
					/* Escribimos en los pines (0 0 0 0) */
					digitalWrite(GPIO_COL_0,LOW);
					digitalWrite(GPIO_COL_1,LOW);
					digitalWrite(GPIO_COL_2,LOW);
					digitalWrite(GPIO_COL_3,LOW);

					break;
		}


			/* Variable local para el for */
			int i;
		if(juego_seleccionado==1){
			/* Recorremos las filas de la columna */
			for(i=0;i<7;i++){
				if(juego.arkanoPi.pantalla.matriz[col][i]==1){/* Si en la matriz de lapantalla hay un 1 */
					/* Escribimos en el pin GPIO_ROW_i un 1 */  /* activamos el led */
					digitalWrite(array_f[i],LOW);
				}else{
					/* Escribimos en el pin GPIO_ROW_i un 0 */
					digitalWrite(array_f[i],HIGH);
				}
			}
		}
		if(juego_seleccionado==0){
			/* Recorremos las filas de la columna */
			for(i=0;i<7;i++){
				if(juego.pong.pantalla.matriz[col][i]==1){/* Si en la matriz de lapantalla hay un 1 */
						/* Escribimos en el pin GPIO_ROW_i un 1 */  /* activamos el led */
						digitalWrite(array_f[i],LOW);
				}else{
						/* Escribimos en el pin GPIO_ROW_i un 0 */
						digitalWrite(array_f[i],HIGH);
				}
			}
		}
	/* Si llegamos al valor maximo de columnas empezamos de nuevo */
	if(col==9){
		col=0;
	}else{
		col++;/* Incrementamos la columna para la proxima ejecucion */
	}


}
/**
 * void timer_isr_pelota (union sigval value): metodo que atiende al final de cuenta del timer de la pelota
 * y que activa el FLAG_PELOTA.
*/
void timer_isr_pelota (union sigval value){
	piHiPri(1);
	/* Reinicio el timer de la pelota */
	tmr_startms(timer_pelota,tiempo_pelota);
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	/* Activo FLAG_PELOTA */
	flag |= FLAG_PELOTA;
	piUnlock (FLAGS_KEY);

	/* DEBUG */
	//printf("Has pulsado p");
	/* /DEBUG */
}
/**
 * void timer_isr_joystick (union sigval value): metodo que atiende al final de cuenta del timer del ADC
 * y que activa el FLAG_ADC.
*/
void timer_isr_joystick (union sigval value){
	/* Reinicio el timer del joystick */
	tmr_startms(timer_joystick,tiempo_joystick);
	piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
	/* Activo FLAG_PELOTA */
	flag |= FLAG_ADC;
	piUnlock (FLAGS_KEY);
}
/**
 * void timer_isr_matricial (union sigval value): metodo que atiende
 * al final de cuenta del timer del teclado matricial y que lee posibles
 * pulsaciones.
*/
void timer_isr_matricial(union sigval value){
	digitalWrite(GPIO_TECLADO_COL_1,HIGH);
	int i;
	for(i=0;i<4;i++){
		if(digitalRead(array_matricial_f[i])==1){
			if(i==0){
				piLock(PELOTA_KEY);/* Bloqueo este mutex para poder modificar el valor de tiempo_pelota */
				/* Modificamos el tiempo de refresco de la posicion de la pelota */
				tiempo_pelota=900;
				piUnlock(PELOTA_KEY);
				piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
				/* Activo FLAG_TECLA */
				flag |= FLAG_DIFICULTAD;
				piUnlock (FLAGS_KEY);
			}
			if(i==1){
				piLock(PELOTA_KEY);/* Bloqueo este mutex para poder modificar el valor de tiempo_pelota */
				/* Modificamos el tiempo de refresco de la posicion de la pelota */
				tiempo_pelota=600;
				piUnlock(PELOTA_KEY);
				piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
				/* Activo FLAG_TECLA */
				flag |= FLAG_DIFICULTAD;
				piUnlock (FLAGS_KEY);
			}
			if(i==2){
				piLock(PELOTA_KEY);/* Bloqueo este mutex para poder modificar el valor de tiempo_pelota */
				/* Modificamos el tiempo de refresco de la posicion de la pelota */
				tiempo_pelota=400;
				piUnlock(PELOTA_KEY);
				piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
				/* Activo FLAG_TECLA */
				flag |= FLAG_DIFICULTAD;
				piUnlock (FLAGS_KEY);
			}
			if(i==3){
				if(juego_seleccionado==1){
					piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
					flag |= FLAG_PAUSE;/* Activo FLAG_PAUSE */
					piUnlock (FLAGS_KEY);
				}
				if(juego_seleccionado==0){
					piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
					flag |= FLAG_PAUSE_PONG;/* Activo FLAG_PAUSE_PONG */
					piUnlock (FLAGS_KEY);
				}
			}
			delay(100);
		}
	}
	tmr_startms(timer_matricial,tiempo_matricial);

}
/**
 * void timer_isr_fin_poder (union sigval value): metodo que atiende al final de cuenta del timer del poder 
 * de la raqueta.
*/
void timer_isr_fin_poder (union sigval value){
    /* Reseteo el ancho de la raqueta */
    juego.arkanoPi.raqueta.ancho=3;
    /* Reseteo el numero necesario de ladrillos para que caiga el paquete */
    ladrillos_necesarios=random()%10;
    /* Reseteo el numero de ladrillos destruidos */
    nladrillos=0;
}
/**
 * void timer_isr_sonido (union sigval value): metodo que atiende al
 * final de cuenta del timer del sonido del rebote.
*/
void timer_isr_sonido (union sigval value){
	softToneWrite(GPIO_SOFTTONE,0);
}
/* Interrupt Functions ------------------------------------------------------------------*/
/**
 * void myInterruptIzq(void):funcion que atiende a la interrupcion de pulsar el pulsador izquierda.
*/ 
void myInterruptIzq(void){
	/* Condicion que tiene como objetivo eliminar os rebotes del pulsador */
	if(millis() < debounceTimeIzq){
		debounceTimeIzq=millis()+DEBOUNCE_TIME;
		return;
	}
	/* Reinicio el umbral de rebote */
	debounceTimeIzq=0;
	if(juego_seleccionado==1){
		/* Atencion a la interrupcion */
		piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		/* Activo FLAG_RAQUTA_IZQUIERDA */
		flag |= FLAG_RAQUETA_IZQUIERDA;
		piUnlock (FLAGS_KEY);
	}
	if(juego_seleccionado==0){
		/* Atencion a la interrupcion */
				piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
				/* Activo FLAG_RAQUTA_IZQUIERDA_PONG */
				flag |= FLAG_RAQUETA_IZQUIERDA_PONG;
				piUnlock (FLAGS_KEY);
	}
	/* DEBUG */
	//piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
	//printf("Has pulsado boton izquierda");
	//piUnlock (STD_IO_BUFFER_KEY);
	/* /DEBUG */

	/* Estructura en la que se queda el programa mientras el usuario pulsa el pulsador */
	while(digitalRead(GPIO_BOTON_I)==HIGH){
		/* Comprobacion para saber si estan los dos pulsadores pulsados */
		if(digitalRead(GPIO_BOTON_D)==HIGH){
			piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
			flag |= FLAG_PAUSE;/* Activo FLAG_PAUSE */
			flag &= ~FLAG_RAQUETA_DERECHA; /* Desactivo FLAG_PAUSE */
			flag &= ~FLAG_RAQUETA_IZQUIERDA;/* Desactivo FLAG_PAUSE */
			piUnlock (FLAGS_KEY);
		}
		/* Tiempo para eliminar los rebotes */
		delay(200);
	}
	/* Actualizacion del umbral */
	debounceTimeIzq=millis()+DEBOUNCE_TIME;
}
/**
 * void myInterruptDer(void):funcion que atiende a la interrupcion de pulsar el pulsador derecha.
*/
void myInterruptDer(void){
	/* Condicion que tiene como objetivo eliminar os rebotes del pulsador */
	if(millis() < debounceTimeDer){
			debounceTimeDer=millis()+DEBOUNCE_TIME;
			return;
	}
	/* Reinicio el umbral de rebote */
	debounceTimeDer=0;
	if(juego_seleccionado==1){
		/* Atencion a la interrupcion */
		piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		/* Activo FLAG_RAQUETA_DERECHA */
		flag |= FLAG_RAQUETA_DERECHA;
		piUnlock (FLAGS_KEY);
	}
	if(juego_seleccionado==0){
		/* Atencion a la interrupcion */
		piLock (FLAGS_KEY);/* Bloqueo este mutex para poder modificar el valor del flag */
		/* Activo FLAG_RAQUETA_DERECHA_PONG */
		flag |= FLAG_RAQUETA_DERECHA_PONG;
		piUnlock (FLAGS_KEY);
	}
	/* DEBUG */
	//piLock (STD_IO_BUFFER_KEY);/* Bloqueo este mutex para poder imprimir por pantalla */
	//printf("Has pulsado boton derecha");
	//piUnlock (STD_IO_BUFFER_KEY);
	/* /DEBUG */

	/* Estructura en la que se queda el programa mientras el usuario pulsa el pulsador */
	while(digitalRead(GPIO_BOTON_D)==HIGH){
			delay(1);
	}
	/* Actualizacion del umbral */
	debounceTimeDer=millis()+DEBOUNCE_TIME;
}

/**
 *int main(): Subrutina donde se ejecuta el programa principal.
*/
int main ()
{
	
	systemSetup();
	piHiPri(2);

	/* Variable local */
	unsigned int next;

	tipo_pantalla pantalla_inicial;/* Variable tipo pantalla para pintar el mensaje inicial */
	/* Modifico el atributo matriz de pantalla_inicial para que sea igual a la matriz mensaje_inicial */
	memcpy((void*) pantalla_inicial.matriz,(const void*) mensaje_inicial, sizeof(mensaje_inicial));
	piLock (STD_IO_BUFFER_KEY); /* Bloqueo este mutex para poder imprimir por pantalla */
	/* Pinto mensaje inicial y lo sacor por pantalla */
	PintaMensajeInicialPantalla (&juego.arkanoPi.pantalla,&(pantalla_inicial));
	PintaPantallaPorTerminal(&juego.arkanoPi.pantalla);
	piUnlock (STD_IO_BUFFER_KEY);
	/* Inicializo y enciendo el timer de refresco leds */
	timer= tmr_new(timer_isr);
	tmr_startms(timer,1);

	/** 
	 * Maquina de estados: lista de transiciones
	 * {EstadoOrigen,Funci�nDeEntrada,EstadoDestino,FuncionDeSalida}
	 */ 
	fsm_trans_t tabla_estados[] = {
		/* ArkanoPi */
		{ WAIT_JUEGO,CompruebaArkanoPi,WAIT_START, InicializaDificultad},
		{ WAIT_START,CompruebaDificultad,WAIT_PUSH, InicializaJuego},
		{ WAIT_PUSH,CompruebaPause,WAIT_PAUSE,PausarJuego},
		{ WAIT_PAUSE,CompruebaPause,WAIT_PUSH, ReanudarJuego},
		{ WAIT_PUSH,CompruebaTeclaPelota,WAIT_PUSH, MovimientoPelota},
		{ WAIT_PUSH,CompruebaTeclaRaquetaDerecha,WAIT_PUSH, MueveRaquetaDerecha},
		{ WAIT_PUSH,CompruebaTeclaRaquetaIzquierda,WAIT_PUSH, MueveRaquetaIzquierda},
		{ WAIT_PUSH,CompruebaFinalJuego,WAIT_END, FinalJuego},
		{ WAIT_END,CompruebaArkanoPi,WAIT_START, ReseteaJuego},
		{ WAIT_PUSH,CompruebaADC,WAIT_PUSH, lectura_ADC},
		/* Pong */
		{ WAIT_JUEGO,CompruebaPong,WAIT_START_PONG, InicializaDificultad},
		{ WAIT_START_PONG,CompruebaDificultad,WAIT_PUSH_PONG, InicializaJuegoPong},
		{ WAIT_PUSH_PONG,CompruebaPausePong,WAIT_PAUSE,PausarJuego},
		{ WAIT_PAUSE,CompruebaPausePong,WAIT_PUSH_PONG, ReanudarJuego},
		{ WAIT_PUSH_PONG,CompruebaTeclaPelota,WAIT_PUSH_PONG, MovimientoPelotaJuegoPong},
		{ WAIT_PUSH_PONG,CompruebaTeclaRaquetaDerecha,WAIT_PUSH_PONG, MueveRaquetaDArriba},
		{ WAIT_PUSH_PONG,CompruebaTeclaRaquetaDerechaPong,WAIT_PUSH_PONG, MueveRaquetaIAbajo},
		{ WAIT_PUSH_PONG,CompruebaTeclaRaquetaIzquierda,WAIT_PUSH_PONG, MueveRaquetaDAbajo},
		{ WAIT_PUSH_PONG,CompruebaTeclaRaquetaIzquierdaPong, WAIT_PUSH_PONG, MueveRaquetaIArriba},
		{ WAIT_PUSH_PONG,CompruebaFinalJuego,WAIT_END, FinalJuegoPong},
		{ WAIT_END,CompruebaPong,WAIT_START_PONG, ReseteaJuego},
		{ WAIT_PUSH_PONG,CompruebaADC,WAIT_PUSH_PONG, lectura_ADC},

		{ -1, NULL, -1, NULL },
	};
	/* Creamos una maquina de estados */
	fsm_t* juego_fsm = fsm_new (WAIT_JUEGO,tabla_estados,timer);


		/* Modificamos el modo de los pines a modo ENTRADA/SALIDA */
		pinMode (GPIO_FILA_0, OUTPUT);
		pinMode (GPIO_FILA_1, OUTPUT);
		pinMode (GPIO_FILA_2, OUTPUT);
		pinMode (GPIO_FILA_3, OUTPUT);
		pinMode (GPIO_FILA_4, OUTPUT);
		pinMode (GPIO_FILA_5, OUTPUT);
		pinMode (GPIO_FILA_6, OUTPUT);
		pinMode (GPIO_COL_0, OUTPUT);
		pinMode (GPIO_COL_1, OUTPUT);
		pinMode (GPIO_COL_2, OUTPUT);
		pinMode (GPIO_COL_3, OUTPUT);
		pinMode (GPIO_BOTON_I,INPUT);
		pinMode (GPIO_BOTON_D,INPUT);

		pinMode (GPIO_TECLADO_COL_1, OUTPUT);
		pinMode (GPIO_TECLADO_FILA_1, INPUT);
		pinMode (GPIO_TECLADO_FILA_2, INPUT);
		pinMode (GPIO_TECLADO_FILA_3, INPUT);
		pinMode (GPIO_TECLADO_FILA_4, INPUT);
	/* Configuramos las interrupciones de los interruptores a flanco de subida */
	wiringPiISR(GPIO_BOTON_I,INT_EDGE_RISING,myInterruptIzq);
	wiringPiISR(GPIO_BOTON_D,INT_EDGE_RISING,myInterruptDer);


	/* Configuracion e inicializacion del sistema y maquina ade estados */
	fsm_setup (juego_fsm);
	/* Inicializamos y enciendo el timer de refresco de pelota */
	timer_pelota= tmr_new(timer_isr_pelota);
	tmr_startms(timer_pelota,1);
	/* Inicializamos timer para leer el valore del potenciometro */
	timer_joystick=tmr_new(timer_isr_joystick);
	tmr_startms(timer_joystick,tiempo_joystick);
	/* Inicializamos timer para leer valores del teclado matricial */
	timer_matricial=tmr_new(timer_isr_matricial);
	tmr_startms(timer_matricial,tiempo_matricial);
    /* Inicializamos el timer para controlar el tiempo de poder(raqueta mas ancha) */
    timer_poder=tmr_new(timer_isr_fin_poder);
    /* Inicializo el timer de duracion del sonido */
    timer_sonido=tmr_new(timer_isr_sonido);
	/* Bucle infinito que ejecuta la m�quina de estados */
	next = millis();
	while (1) {
		fsm_fire (juego_fsm);/* Intenta ejecutar una transicion en funcion del valor de las entradas. */
		next += CLK_MS;
		delay_until (next); /* Reposo durante 10 ms */
	}
	/* Finalizamos el timer  */
	tmr_destroy((tmr_t*)timer);



}

