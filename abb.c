#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "abb.h"
#include "pila.h"

typedef struct abb_nodo abb_nodo_t;

struct abb_nodo {
	const char* clave;
	void* dato;
	abb_nodo_t* izq;
	abb_nodo_t* der;
};

struct abb {
	abb_nodo_t* raiz;
	size_t cantidad;
	abb_comparar_clave_t comparar;
	abb_destruir_dato_t destruir;
};

struct abb_iter {
	pila_t* pila;
	abb_nodo_t* actual;
};

/* *****************************************************************
 *                    FUNCIONES AUXILIARES
 * *****************************************************************/

// Crea una copia de la clave pasada por parametro
// Pre: Ninguna.
// Post: Devuelve una copia de la clave, NULL si no funcionó malloc().
char* crear_clave(const char* clave){
	char *copia = malloc(sizeof(char) * (strlen(clave) + 1));
	if (!copia) return NULL;
	strcpy(copia, clave);
	return copia;
}

// Crea un nuevo nodo del árbol
// Pre: Ninguna.
// Post: Devuelve un nuevo nodo hoja.
abb_nodo_t* abb_nodo_crear(const char* clave, void* dato) {
	abb_nodo_t* nodo = malloc(sizeof(abb_nodo_t));
	if (!nodo) return NULL;
	nodo->clave = crear_clave(clave);
	nodo->dato = dato;
	nodo->izq = NULL,
	nodo->der = NULL;
	return nodo;
}

// Destruye un nodo del árbol
// Pre: El nodo existe.
// Post: Destruye el nodo y su contenido.
void* abb_nodo_destruir(abb_nodo_t* nodo) {
	if (!nodo) return NULL;
	void* dato = nodo->dato;
	free((char*)nodo->clave);
	free(nodo);
	return dato;
}

// Busca una clave en un árbol recursivamente.
// Pre: Ninguna.
// Post: Devuelve el nodo buscado (si existe), NULL si no existe.
abb_nodo_t *abb_buscar_r(const abb_t *arbol, abb_nodo_t* nodo, const char *clave) {
	// Si el nodo es NULL, llegué al final del árbol y no lo encontré
	if (!nodo) return NULL;
	int r = arbol->comparar(nodo->clave, clave);
	// Si las claves son iguales, devuelvo el nodo
	if (r == 0) return nodo;
	// Si la clave del nodo es menor a la del parámetro, busco por la derecha
	else if (r == -1) return abb_buscar_r(arbol, nodo->der, clave);
	// Si es mayor, por la izquierda
	return abb_buscar_r(arbol, nodo->izq, clave);
}

// Borra un nodo que tiene un solo hijo
void* abb_borrar_con_hijo_unico(abb_nodo_t* nodo, abb_nodo_t* padre, int lado, abb_t* arbol){
	// Si tiene hijo izquierdo
	if ((!nodo->der) && (nodo->izq)){
		if (!padre) arbol->raiz = nodo->izq;
		else if (lado == -1) padre->der = nodo->izq;
		else if (lado == 1)	padre->izq = nodo->izq;
		arbol->cantidad--;
		return abb_nodo_destruir(nodo);
	}
	// Si tiene hijo derecho
	else if ((nodo->der) && (!nodo->izq)){
		if (!padre) arbol->raiz = nodo->der;
		else if (lado == -1) padre->der = nodo->der;
		else if (lado == 1)	padre->izq = nodo->der;
		arbol->cantidad--;
		return abb_nodo_destruir(nodo);
	}
	return NULL;
}

// Funcion que dado un nodo busca sus descendientes izquierdos. Devuelve el padre del más izquierdo
abb_nodo_t* buscar_padre_mas_izq(abb_nodo_t* reemplazante){
	abb_nodo_t* padre_del_reemplazante = NULL; 
	while (reemplazante->izq != NULL){
		padre_del_reemplazante = reemplazante;
		reemplazante = reemplazante->izq;
	}
	return padre_del_reemplazante;
}

// Función que borra un nodo con dos hijos
void* abb_borrar_con_2hijos(abb_nodo_t* nodo, abb_nodo_t* padre, int lado, abb_t* arbol){
	abb_nodo_t* padre_del_reemplazante = buscar_padre_mas_izq(nodo->der);
	abb_nodo_t* reemplazante;
	if (!padre_del_reemplazante) {
		reemplazante = nodo->der;
		reemplazante->izq = nodo->izq;
	}
	else {
		reemplazante = padre_del_reemplazante->izq;
		padre_del_reemplazante->izq = reemplazante->der;
		reemplazante->izq = nodo->izq;
		reemplazante->der = nodo->der;
	}
	if (!padre) arbol->raiz = reemplazante;
	else if (lado == -1) padre->der = reemplazante;
	else if (lado == 1)	padre->izq = reemplazante;
	arbol->cantidad--;
	return abb_nodo_destruir(nodo);
}

/* *****************************************************************
 *                    PRIMITIVAS DEL ÁRBOL
 * *****************************************************************/

// Crea un nuevo árbol
// Pre: Ninguna.
// Post: Devuelve un nuevo árbol vacío.
abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato) {
	abb_t* abb = malloc(sizeof(abb_t));
	if (!abb) return NULL;
	abb->raiz = NULL;
	abb->cantidad = 0;
	abb->comparar = cmp;
	abb->destruir = destruir_dato;
	return abb;
}

// Función que busca recursivamente la ubicación donde guardar el nodo, y lo guarda.
// Pre: El árbol existe.
// Post: Devuelve TRUE si pudo guardar el nodo, FALSE si no.
bool abb_guardar_r(abb_t *arbol, abb_nodo_t* nodo_actual, const char *clave, void *dato) {
	int r = arbol->comparar(nodo_actual->clave, clave);
	// Si la clave ya pertenece al árbol, destruyo su dato (si hace falta) y lo reemplazo
	if (r == 0) {
		if (arbol->destruir) {
			arbol->destruir(nodo_actual->dato);
		}
		nodo_actual->dato = dato;
		return true;
	}
	// Si la clave de la raíz es mayor a la que se debe guardar, busco por la izquierda
	else if (r == 1) {
		// Si no hay nada a la izquierda, guardo el nodo
		if (!nodo_actual->izq) {
			abb_nodo_t* nuevo_nodo = abb_nodo_crear(clave, dato);
			nodo_actual->izq = nuevo_nodo;
			arbol->cantidad++;
			return true;
		}
		return abb_guardar_r(arbol, nodo_actual->izq, clave, dato);
	}
	// Si es menor, por la derecha
	else if (r == -1) {
		// Si no hay nada a la derecha, guardo el nodo
		if (!nodo_actual->der) {
			abb_nodo_t* nuevo_nodo = abb_nodo_crear(clave, dato);
			nodo_actual->der = nuevo_nodo;
			arbol->cantidad++;
			return true;
		}
		return abb_guardar_r(arbol, nodo_actual->der, clave, dato);
	}
	return false;
}

// Guarda una clave en el árbol
// Pre: El árbol existe.
// Post: Devuelve TRUE si pudo guardar la clave, FALSE si no.
bool abb_guardar(abb_t *arbol, const char *clave, void *dato) {
	if (!arbol) return false;
	// Si el árbol está vacío, lo guardo en la raíz
	if (abb_cantidad(arbol) == 0) {
		abb_nodo_t* nuevo_nodo = abb_nodo_crear(clave, dato);
		if (!nuevo_nodo) return false;
		arbol->raiz = nuevo_nodo;
		arbol->cantidad++;
		return true;
	}
	return abb_guardar_r(arbol, arbol->raiz, clave, dato);
}

// Funcion recursiva de borrar
void* abb_borrar_r(abb_nodo_t* nodo, abb_nodo_t* padre, int lado, const char* clave, abb_t* arbol){
	int r = arbol->comparar(nodo->clave, clave);
	if (r == 0) {
		// Si nodo es hoja
		if (!nodo->izq && !nodo->der) {
			if (padre == NULL) arbol->raiz = NULL;
			// Si nodo es el hijo izquierdo de su padre
			else if (lado == 1)	padre->izq = NULL;
			// Si nodo es el hijo derecho de su padre
			else if (lado == -1) padre->der = NULL;
			arbol->cantidad--;
			return abb_nodo_destruir(nodo);
		}
		// Si tiene un solo hijo
		else if ((!nodo->der && nodo->izq) || (!nodo->izq && nodo->der)) return abb_borrar_con_hijo_unico(nodo, padre, lado, arbol);
		// Si tiene dos hijos
		else if (nodo->der && nodo->izq) return abb_borrar_con_2hijos(nodo, padre, lado, arbol);
	}
	else if (r > 0) return abb_borrar_r(nodo->izq, nodo, 1, clave, arbol);
	else if (r < 0) return abb_borrar_r(nodo->der, nodo, -1, clave, arbol);
	return NULL;
}

// Borra un nodo del árbol.
// Pre: El árbol existe.
// Post: Devuelve el dato asociado a la clave borrada (si existe), NULL si no existe.
void* abb_borrar(abb_t *arbol, const char *clave){
	if (!arbol) return NULL;
	if (!abb_pertenece(arbol, clave))
		return NULL;
	else
		return abb_borrar_r(arbol->raiz, NULL, 0, clave, arbol);
}

// Obtiene el dato asociado a una clave del árbol.
// Pre: El árbol existe.
// Post: Devuelve el dato asociado a la clave (si existe), NULL si no existe.
void *abb_obtener(const abb_t *arbol, const char *clave) {
	if (!arbol) return NULL;
	abb_nodo_t* nodo = abb_buscar_r(arbol, arbol->raiz, clave);
	if (!nodo) return NULL;
	return nodo->dato;
}

// Verifica si una clave pertenece a un árbol.
// Pre: El árbol existe.
// Post: Devuelve TRUE si la clave pertenece al árbol, FALSE si no.
bool abb_pertenece(const abb_t *arbol, const char *clave) {
	if (!arbol) return false;
	abb_nodo_t* nodo = abb_buscar_r(arbol, arbol->raiz, clave);
	if (!nodo) return false;
	return true;
}

// Devuelve la cantidad de nodos de un árbol.
// Pre: El árbol existe.
// Post: Devuelve la cantidad de nodos del árbol.
size_t abb_cantidad(abb_t *arbol) {
	if (!arbol) return 0;
	return arbol->cantidad;
}

// Función recursiva para destruir el contenido de un árbol.
// Pre: El árbol existe.
// Post: Se destruyó el contenido del árbol.
void abb_destruir_r(abb_nodo_t* nodo, abb_destruir_dato_t destruir){
	if (nodo->izq) abb_destruir_r(nodo->izq, destruir);
	if (nodo->der) abb_destruir_r(nodo->der, destruir);
	void* dato = abb_nodo_destruir(nodo);
	if (destruir) destruir(dato);
	return;
}

// Destruye un árbol.
// Pre: El árbol existe.
// Post: Se destruyó el árbol y todo lo que contiene.
void abb_destruir(abb_t *arbol) {
	if (!arbol) return;
	if (arbol->raiz) abb_destruir_r(arbol->raiz, arbol->destruir);
	free(arbol);
}

/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR INTERNO
 * *****************************************************************/

void abb_in_order_r(abb_nodo_t* nodo, bool funcion(const char*, void*, void*), void* extra){
	if (nodo->izq) abb_in_order_r(nodo->izq, funcion, extra);
	if (!funcion(nodo->clave, nodo->dato, extra)) return;
	if (nodo->der) abb_in_order_r(nodo->der, funcion, extra);
	return;
}

void abb_in_order(abb_t *arbol, bool funcion(const char *, void *, void *), void *extra){
	if (!arbol || !arbol->raiz) return;
	abb_in_order_r(arbol->raiz, funcion, extra);
	return;
}

/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/

// Crea un nuevo iterador sobre un árbol existente.
// Pre: El árbol existe.
// Post: Se creó el iterador.
abb_iter_t *abb_iter_in_crear(const abb_t *arbol) {
	if (!arbol) return NULL;
	abb_iter_t* iter = malloc(sizeof(abb_iter_t));
	if (!iter) return NULL;
	iter->pila = pila_crear();
	if (!iter->pila) return NULL;
	abb_nodo_t* nodo = arbol->raiz;
	if (arbol->raiz){
		while (nodo->izq) {
			pila_apilar(iter->pila, nodo);
			nodo = nodo->izq;
		}
	}
	iter->actual = nodo;
	return iter;
}

// Avanza a la siguiente posición del árbol. Devuelve TRUE si pudo avanzar,
// FALSE si se encuentra al final.
// Pre: El iterador existe.
bool abb_iter_in_avanzar(abb_iter_t *iter) {
	if (abb_iter_in_al_final(iter)) return false;
	if (!iter->actual->der)
		iter->actual = pila_desapilar(iter->pila);
	else {
		iter->actual = iter->actual->der;
		while (iter->actual->izq) {
			pila_apilar(iter->pila, iter->actual);
			iter->actual = iter->actual->izq;
		}
	}
	return true;
}

// Devuelve la clave actual a la cual apunta el iterador
// Pre: El iterador existe.
const char *abb_iter_in_ver_actual(const abb_iter_t *iter) {
	if (!iter->actual) return NULL;
	return iter->actual->clave;
}

// Devuelve TRUE si el iterador se encuentra al final, FALSE si no
// Pre: El iterador existe.
bool abb_iter_in_al_final(const abb_iter_t *iter) {
	if (!iter->actual) return true;
	return false;
}

// Destruye el iterador.
// Pre: El iterador existe.
// Post: Se destruyó el iterador
void abb_iter_in_destruir(abb_iter_t* iter) {
	pila_destruir(iter->pila);
	free(iter);
	return;
}
