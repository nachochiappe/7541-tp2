#ifndef ABB_H
#define ABB_H

#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

typedef struct abb abb_t;

typedef int (*abb_comparar_clave_t) (const char *, const char *);

typedef void (*abb_destruir_dato_t) (void *);

/* ******************************************************************
 *                  PRIMITIVAS BÁSICAS DEL ÁRBOL
 * *****************************************************************/

// Crea un nuevo árbol
// Pre: Ninguna.
// Post: Devuelve un nuevo árbol vacío.
abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato);

// Guarda una clave en el árbol
// Pre: El árbol existe.
// Post: Devuelve TRUE si pudo guardar la clave, FALSE si no.
bool abb_guardar(abb_t *arbol, const char *clave, void *dato);

// Borra un nodo del árbol.
// Pre: El árbol existe.
// Post: Devuelve el dato asociado a la clave borrada (si existe), NULL si no existe.
void *abb_borrar(abb_t *arbol, const char *clave);

// Obtiene el dato asociado a una clave del árbol.
// Pre: El árbol existe.
// Post: Devuelve el dato asociado a la clave (si existe), NULL si no existe.
void *abb_obtener(const abb_t *arbol, const char *clave);

// Verifica si una clave pertenece a un árbol.
// Pre: El árbol existe.
// Post: Devuelve TRUE si la clave pertenece al árbol, FALSE si no.
bool abb_pertenece(const abb_t *arbol, const char *clave);

// Devuelve la cantidad de nodos de un árbol.
// Pre: El árbol existe.
// Post: Devuelve la cantidad de nodos del árbol.
size_t abb_cantidad(abb_t *arbol);

// Destruye un árbol.
// Pre: El árbol existe.
// Post: Se destruyó el árbol y todo lo que contiene.
void abb_destruir(abb_t *arbol);

/* ******************************************************************
 *                     PRIMITIVAS DE ITERACIÓN
 * *****************************************************************/

// Primitivas iterador interno

void abb_in_order(abb_t *arbol, bool visitar(const char *, void *, void *), void *extra);

// Primitivas iterador externo

typedef struct abb_iter abb_iter_t;

abb_iter_t *abb_iter_in_crear(const abb_t *arbol);

bool abb_iter_in_avanzar(abb_iter_t *iter);

const char *abb_iter_in_ver_actual(const abb_iter_t *iter);

bool abb_iter_in_al_final(const abb_iter_t *iter);

void abb_iter_in_destruir(abb_iter_t* iter);

#endif // ABB_H
