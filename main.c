/*######################### Descripción del proyecto: #########################
#   Programa en C que simula una caja registradora.
#   Descripción:
#    - Se creará un programa que simule una caja registradora que va consumiendo productos.
#    - Código desarrollado mediante metodología de programación modular, con el objetivo de reutilizar bibliotecas de funciones y/o métodos.
#
#   Autores:
#       - Aracely Peréz (https://github.com/AracelyPerez)
#       - Nicolás Aburto (https://github.com/NicolasAburto)
#
#   Licencia:
#       - Julio 2022. Apache 2.0.
#
###########################################################################################################*/

/* Programa de ejemplo de semaphore. */
/* para compilar usar: gcc -o tarea3_ejemplo tarea3_ejemplo.c -lpthread */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct {
  int cinta;
  float* almacen;
  int rondas;
  int demora_min_cliente;
  int demora_max_cliente;
  int demora_min_caja;
  int demora_max_caja;
  int cantidad_productos;
  sem_t puede_producir;
  sem_t puede_consumir;
  sem_t espera;
} datos_compartidos_t;

void* produce(void* data);
void* consume(void* data);
int random_entre(int min, int max);

int main(int argc, char* argv[]) {
  int error;
  struct timespec tiempo_ini;
  struct timespec tiempo_fin;
  pthread_t productor, consumidor;
  datos_compartidos_t datos_compartidos;

  srandom(time(NULL));

  if (argc == 5) {
    datos_compartidos.demora_min_cliente=atoi(argv[1]);
    datos_compartidos.demora_max_cliente=atoi(argv[2]);
    datos_compartidos.demora_min_caja=atoi(argv[3]);
    datos_compartidos.demora_max_caja=atoi(argv[4]);
  } else {
    printf("Usar: %s demora_min_cliente demora_max_cliente demora_min_caja demora_max_caja\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  datos_compartidos.cinta = random_entre(5,15);
  datos_compartidos.rondas = random_entre(1,10);
  datos_compartidos.cantidad_productos = random_entre(1,20);

  printf("ronda: %i\n",datos_compartidos.rondas);
  printf("capacidad_almacen: %i\n",datos_compartidos.cinta);
  printf("cantidad_productos: %i\n",datos_compartidos.cantidad_productos);

  datos_compartidos.almacen = (float*) calloc(datos_compartidos.cinta, sizeof(float));
  sem_init(&datos_compartidos.puede_producir, 0, datos_compartidos.cinta);
  sem_init(&datos_compartidos.puede_consumir, 0, 0);
  sem_init(&datos_compartidos.espera,0,datos_compartidos.cantidad_productos);
 

  clock_gettime(CLOCK_MONOTONIC, &tiempo_ini);

  error = pthread_create(&productor, NULL, produce, &datos_compartidos);
  if (error == 0) {
    error = pthread_create(&consumidor, NULL, consume, &datos_compartidos);
    if (error != 0) {
      fprintf(stderr, "error: no puede crear consumidor\n");
      error = 1;
    }
  } else {
    fprintf(stderr, "error: no puede crear productor\n");
    error = 1;
  }
  if (error == 0) {
    pthread_join(productor, NULL);
    pthread_join(consumidor, NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);

  float periodo = (tiempo_fin.tv_sec - tiempo_ini.tv_sec) + 
          (tiempo_fin.tv_nsec - tiempo_ini.tv_nsec) * 1e-9;
  printf("Tiempo de ejecución: %.9lfs\n", periodo);

  sem_destroy(&datos_compartidos.puede_consumir);
  sem_destroy(&datos_compartidos.puede_producir);
  sem_destroy(&datos_compartidos.espera);
  free(datos_compartidos.almacen);

  return EXIT_SUCCESS;
}

void* produce(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  int contador = 0;
  for (int ronda = 0; ronda < datos_compartidos->rondas; ++ronda) {
    printf("INICIO RONDA P: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos->cantidad_productos; ++indice) {
      sem_wait(&datos_compartidos->puede_producir);
      usleep(1000 * random_entre(datos_compartidos->demora_min_cliente, datos_compartidos->demora_max_cliente));
      datos_compartidos->almacen[indice] = ++contador;
      printf("Indice almacen %i se produce %lg\n", indice, datos_compartidos->almacen[indice]);
      sem_post(&datos_compartidos->puede_consumir);
    }
    sem_wait(&datos_compartidos->espera);
  }
  return NULL;
}


void* consume(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  for (int ronda = 0; ronda < datos_compartidos->rondas; ++ronda) {
    printf("\t\tINICIO RONDA C: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos->cantidad_productos; ++indice) {
      sem_wait(&datos_compartidos->puede_consumir);
      float value = datos_compartidos->almacen[indice];
      usleep(1000 * random_entre(datos_compartidos->demora_min_caja
        , datos_compartidos->demora_max_caja));
      printf("\t\tIndice almacen %i se consume %lg\n", indice, value);
      sem_post(&datos_compartidos->puede_producir);
      }
    }
  sem_post(&datos_compartidos->espera);
return NULL;
}



int random_entre(int min, int max) {
  int aux=0;
  if (max > min)
    aux=random() % (max - min);
  return min + aux;
}
