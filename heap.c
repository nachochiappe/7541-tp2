#include "heap.h"
#include "cola.h"
#include <stdlib.h>
#include <stdbool.h>

#define TAM_INICIAL 5
#define FACTOR_AUMENTAR 0.7
#define AUMENTAR_TAM 3

		// Definicion estructuras //

struct heap{
	size_t cantidad;
	size_t tamanio;
	void** tabla_heap;
	cmp_func_t cmp;
};

		// Funciones Auxiliares //


/* Ordena el heap de forma recursiva hacia arriba, segun
 * el orden de prioridad establecido en la funcion 'heap->cmp'.
 */
void up_heap(heap_t* heap, size_t pos_actual){
	if (pos_actual == 0) return; // El actual es la raiz del heap (es maximo).
	void* actual = heap->tabla_heap[pos_actual];
	void* padre = heap->tabla_heap[(pos_actual - 1)/ 2];
	int comparar_claves = heap->cmp(actual, padre);
	if (comparar_claves > 0){ // La clave del actual es mayor a la de su padre.
		heap->tabla_heap[pos_actual] = padre;
		heap->tabla_heap[(pos_actual - 1)/ 2] = actual;
		pos_actual = (pos_actual - 1)/ 2;
		up_heap(heap, pos_actual);
	}
	// En otro caso (menor o igual), dejamos el heap como esta.
}

/* Aumenta el tamanio del heap segun AUMENTAR_TAM. Devuelve
 * true si pudo copiar los datos en la tabla nueva, o false si no.
 */
bool redimensionar_heap(heap_t* heap){
	size_t nuevo_tam = heap->tamanio * AUMENTAR_TAM;
	void** tabla_nueva = realloc(heap->tabla_heap, nuevo_tam * sizeof(void*));
	if (!tabla_nueva) return false;
	heap->tabla_heap = tabla_nueva;
	heap->tamanio = nuevo_tam;
	return true;
}


/* Inserta en la ultima posicion el elemento pasado por
 * parametro. En caso de fallar, devuelve false. Si no, devuelve
 * true, habiendo ordenado el heap de forma tal que se mantenga
 * la prioridad establecida luego de insertar el valor.
 */
bool insertar_con_prioridad(heap_t* heap, void* elemento){
	if (heap->cantidad / heap->tamanio >= FACTOR_AUMENTAR){
		bool redimension = redimensionar_heap(heap);
		if (heap->cantidad / heap->tamanio == 1 && !redimension) return false;
	}
	heap->tabla_heap[heap->cantidad] = elemento;
	up_heap(heap, heap->cantidad);
	heap->cantidad++;
	return true;
}

/* Devuelve el mayor de los hijos del valor en la posicion pasada.
 * En caso de no tener hijos izquierdos, devuelve la cantidad del heap
 * (es un nodo hoja). Caso contrario, devuelve la posicion correspondiente.
 */
size_t buscar_max_hijos(heap_t* heap, size_t pos_actual){
	size_t pos_hijo_izq = 2 * pos_actual + 1;
	size_t pos_hijo_der = pos_hijo_izq + 1;
	if (pos_hijo_izq >= heap->cantidad) return heap->cantidad; // Final del arbol
	if (pos_hijo_der >= heap->cantidad) return pos_hijo_izq;
	int comparacion = heap->cmp(heap->tabla_heap[pos_hijo_izq], heap->tabla_heap[pos_hijo_der]);
	if (comparacion > 0) return pos_hijo_izq;
	return pos_hijo_der;
}

/* Ordena el heap desde la posicion pasada por parametro hacia abajo
 * (en caso de ser necesario) segun la funcion de comparacion dada.
 */
void down_heap(heap_t* heap, size_t pos_actual){
	size_t pos_hijo_mayor = buscar_max_hijos(heap, pos_actual);
	if (pos_hijo_mayor == heap->cantidad) return; // Sucede al llegar al final del arbol.
	void* actual = heap->tabla_heap[pos_actual];
	void* hijo_mayor = heap->tabla_heap[pos_hijo_mayor];
	if (heap->cmp(actual, hijo_mayor) < 0){
		heap->tabla_heap[pos_actual] = hijo_mayor;
		heap->tabla_heap[pos_hijo_mayor] = actual;
		down_heap(heap, pos_hijo_mayor);
	}
}

/* Elimina el elemento de mayor prioridad en el heap. Se verifica
 * que el heap quede ordenado por prioridad luego del borrado.
 * Finalmente, devuelve el elemento.
 */
void* eliminar_primero(heap_t* heap){
	void* elemento = heap->tabla_heap[0];
	heap->tabla_heap[0] = heap->tabla_heap[heap->cantidad - 1];
	heap->tabla_heap[heap->cantidad - 1] = NULL;
	heap->cantidad--;
	down_heap(heap, 0);
	return elemento;
}

		// Primitivas Heap //

/* Crea un heap. Recibe como único parámetro la función de comparación a
 * utilizar. Devuelve un puntero al heap, el cual debe ser destruido con
 * heap_destruir(), o NULL en caso de fallar.
 */
heap_t* heap_crear(cmp_func_t cmp){
	heap_t* heap = malloc(sizeof(heap_t));
	if (!heap) return NULL;
	void** tabla_heap = malloc(sizeof(void*) * TAM_INICIAL);
	if (!tabla_heap){
		free(heap);
		return NULL;
	}
	heap->cantidad = 0;
	heap->tamanio = TAM_INICIAL;
	heap->tabla_heap = tabla_heap;
	heap->cmp = cmp;
	return heap;
}

/* Devuelve true si la cantidad de elementos que hay en el heap es 0,
 * false en caso contrario.
 */
bool heap_esta_vacio(const heap_t* heap){
	if (!heap) return true;
	return heap->cantidad == 0;
}

/* Devuelve la cantidad de elementos que hay en el heap.
 */
size_t heap_cantidad(const heap_t* heap){
	if (heap_esta_vacio(heap)) return 0; // En caso de pasar un heap invalido, la funcion 'heap_esta_vacio' devuelve true.
	return heap->cantidad;
}

/* Agrega un elemento al heap. El elemento no puede ser NULL.
 * Devuelve true si fue una operación exitosa, o false en caso de error. 
 * Pre: el heap fue creado.
 * Post: se agregó un nuevo elemento al heap.
 */
bool heap_encolar(heap_t* heap, void* elem){
	if (!elem || !heap) return false;
	if (heap_esta_vacio(heap)){
		heap->tabla_heap[0] = elem;
		heap->cantidad++;
		return true;
	}
	return insertar_con_prioridad(heap, elem);
}

/* Devuelve el elemento con máxima prioridad. Si el heap esta vacío, devuelve
 * NULL. 
 * Pre: el heap fue creado.
 */
void* heap_ver_max(const heap_t* heap){
	if (heap_esta_vacio(heap)) return NULL;
	return heap->tabla_heap[0];
}

/* Elimina el elemento con máxima prioridad, y lo devuelve.
 * Si el heap esta vacío, devuelve NULL.
 * Pre: el heap fue creado.
 * Post: el elemento desencolado ya no se encuentra en el heap. 
 */
void* heap_desencolar(heap_t* heap){
	if (heap_esta_vacio(heap)) return NULL; // El heap esta vacio o es invalido.
	if (heap->cantidad == 1){
		void* elemento = heap->tabla_heap[0];
		heap->tabla_heap[0] = NULL;
		heap->cantidad--;
		return elemento;
	}
	return eliminar_primero(heap);
}

/* Elimina el heap, llamando a la función dada para cada elemento del mismo.
 * El puntero a la función puede ser NULL, en cuyo caso no se llamará.
 * Post: se llamó a la función indicada con cada elemento del heap. El heap
 * dejó de ser válido.
 */
void heap_destruir(heap_t* heap, void destruir_elemento(void* e)){
	while (!heap_esta_vacio(heap)){
		void* valor_a_borrar = heap_desencolar(heap);
		if (destruir_elemento) destruir_elemento(valor_a_borrar);
	}
	free(heap->tabla_heap);
	free(heap);
}

		// Primitiva Heap Sort //

/* Función de heapsort genérica. Esta función ordena mediante heap_sort
 * un arreglo de punteros opacos, para lo cual requiere que se
 * le pase una función de comparación. Modifica el arreglo "in-place".
 */
void heap_sort(void *elementos[], size_t cant, cmp_func_t cmp){
	heap_t* arreglo_heap = heap_crear(cmp);
	if (!arreglo_heap) return;
	size_t i;
	for (i = 0; i < cant; i++){
		heap_encolar(arreglo_heap, elementos[i]);
	} // Tenemos el heap del arreglo ordenado segun la prioridad dada.
	for (i = 0; i < cant; i++){
		elementos[cant - 1 - i] = heap_desencolar(arreglo_heap);
	}
	heap_destruir(arreglo_heap, NULL);
}
