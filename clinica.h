#include "abb.h"
#include "cola.h"
#include "csv.h"
#include "hash.h"
#include "heap.h"
#include "lista.h"
#include "pila.h"
#include "mensajes.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***********************************
 *       ESTRUCTURAS DE DATOS      *
 ***********************************/

typedef struct doctor doctor_t;
typedef struct paciente paciente_t;
typedef struct especialidad especialidad_t;
typedef struct parametros parametros_t;

/***********************************
 *      PRIMITIVAS PRINCIPALES     *
 ***********************************/

// Función que genera un hash de doctores a partir de un archivo CSV (cuya ruta es el parámetro de la función).
// Pre: Ninguna.
// Post: Devuelve el hash de doctores si se pudo procesar el archivo, NULL si no pudo realizarse por algún motivo.
hash_t* generar_hash_doctores(char* archivo_doctores);

// Función que genera un hash de pacientes a partir de un archivo CSV (cuya ruta es el parámetro de la función).
// Pre: Ninguna.
// Post: Devuelve el hash de pacientes si se pudo procesar el archivo, NULL si no pudo realizarse por algún motivo.
hash_t* generar_hash_pacientes(char* archivo_pacientes);

// Función que genera un hash de especialidades a partir del hash de doctores previamente creado.
// Pre: El hash de doctores existe.
// Post: Devuelve el hash de especialidades, NULL si hubo algún error.
hash_t* generar_hash_especialidades(hash_t* hash_doctores);

// Función que permite solicitar un turno para un paciente para una determinada especialidad.
// Pre: Los hashes de pacientes y especialidades existen.
// Post: Se encola un paciente en el heap de la especialidad ingresada por teclado, según su total contribuído.
//
// Salida por pantalla:
//
// Paciente NOMBRE_PACIENTE encolado
// N paciente(s) en espera para NOMBRE_ESPECIALIDAD
void pedir_turno(parametros_t* parametros, hash_t* hash_pacientes, hash_t* hash_especialidades);

// Función que permite atender a un médico atender al paciente que esté primero en la cola de prioridad de su especialidad.
// Pre: Los hashes de doctores y especialidades existen.
// Post: El paciente es desencolado
//
// Salida por pantalla:
//
// Se atiende a NOMBRE_PACIENTE
// N paciente(s) en espera para NOMBRE_ESPECIALIDAD
void atender_siguiente(parametros_t* parametros, hash_t* hash_doctores, hash_t* hash_especialidades);

// Función que imprime la lista de doctores en orden alfabético, junto con su especialidad y el número de pacientes que atendieron desde que arrancó el sistema.
// Pre: El hash de doctores existe.
// Post: Ninguna.
//
// Salida por pantalla:
//
// N doctor(es) en el sistema
// 1: NOMBRE, especialidad ESPECIALIDAD, X paciente(s) atendido(s)
// 2: NOMBRE, especialidad ESPECIALIDAD, Y paciente(s) atendido(s)
// ...
// N: NOMBRE, especialidad ESPECIALIDAD, Z paciente(s) atendido(s)
void mostrar_informe(hash_t* hash_doctores);

