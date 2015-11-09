#include "heap.h"
#include "cola.h"
#include <stdlib.h>
#include <stdbool.h>

        // Definicion estructuras //

typedef struct nodo_heap nodo_heap_t;

struct nodo_heap{
	void* dato;
	nodo_heap_t* izq;
	nodo_heap_t* der;
    nodo_heap_t* padre;
};

struct heap{
    size_t cantidad;
    nodo_heap_t* raiz;
    cmp_func_t cmp;
};


        // Funciones Auxiliares //

/* Crea un nuevo nodo para el heap, segun el elemento
 * pasado por parametro. En caso de fallar, devuelve
 * NULL, si no, un puntero al nuevo nodo.
 */
nodo_heap_t* crear_nodo_heap(void* elemento){
    nodo_heap_t* nuevo_nodo = malloc(sizeof(nodo_heap_t));
    if (!nuevo_nodo) return NULL;
    nuevo_nodo->dato = elemento;
    nuevo_nodo->izq = NULL;
    nuevo_nodo->der = NULL;
    nuevo_nodo->padre = NULL;
    return nuevo_nodo;
}

/* Ordena el heap de forma recursiva hacia arriba, segun
 * el orden de prioridad establecido en la funcion 'heap->cmp'.
 */
void up_heap(heap_t* heap, nodo_heap_t* actual){
    nodo_heap_t* padre = actual->padre;
    if (!padre) return; // El actual es la raiz del heap (es maximo).
    int comparar_claves = heap->cmp(actual->dato, padre->dato);
    if (comparar_claves > 0){ // La clave del actual es mayor a la de su padre.
        void* aux_dato = actual->dato;
        actual->dato = padre->dato;
        padre->dato = aux_dato;
        actual = padre;
        up_heap(heap, actual);
    }
    // En otro caso (menor o igual), dejamos el heap como esta.
}

/* Busca y devuelve un padre sin alguno de los dos hijos.
 * Al encontrarlo, lo devuelve. En caso de tener un solo
 * nodo (un heap solo con la raiz), devuelve NULL.
 */
nodo_heap_t* buscar_padre(heap_t* heap){
    cola_t* cola_en_nivel = cola_crear();
    if (!cola_en_nivel) return NULL;
    nodo_heap_t* padre = heap->raiz; // Se controla en 'heap_encolar' que exista (el heap no esta vacio).
    while (padre){
        if (!padre->izq || !padre->der) break;
        cola_encolar(cola_en_nivel, padre->izq);
        cola_encolar(cola_en_nivel, padre->der);
        padre = cola_desencolar(cola_en_nivel);
    }
    cola_destruir(cola_en_nivel, NULL);
    return padre;
}

/* Inserta, en el lugar correspondiente, el nodo pasado por
 * parametro. En caso de fallar, devuelve false. Si no, devuelve
 * true, habiendo ordenado el heap de forma tal que se mantenga
 * la prioridad establecida luego de insertar el nodo.
 */
bool insertar_con_prioridad(heap_t* heap, nodo_heap_t* nodo){
    // Primero insertamos al final del heap (cuando se encuentra la hoja vacia correspondiente).
    nodo_heap_t* auxiliar = buscar_padre(heap);
    if (!auxiliar) return false;
    if (!auxiliar->izq)
        auxiliar->izq = nodo;
    else
        auxiliar->der = nodo;
    heap->cantidad++;
    nodo->padre = auxiliar;
    // Ordenamos el heap segun la prioridad dada por la funcion cmp.
    up_heap(heap, nodo);
    return true;
}

/* Busca y devuelve el ultimo nodo del heap, siendo este en el
 * nivel mas inferior y a la derecha del mismo. En caso de fallar,
 * devuelve NULL.
 */
nodo_heap_t* buscar_ultima_hoja(heap_t* heap){
    cola_t* cola_en_nivel = cola_crear();
    if (!cola_en_nivel) return NULL;
    nodo_heap_t* ultima_hoja = heap->raiz; // Se controla en 'heap_desencolar' que exista (el heap no esta vacio).
    do{
        if (ultima_hoja->izq)
            cola_encolar(cola_en_nivel, ultima_hoja->izq);
        if (ultima_hoja->der)
            cola_encolar(cola_en_nivel, ultima_hoja->der);
        ultima_hoja = cola_desencolar(cola_en_nivel);
    }while (!cola_esta_vacia(cola_en_nivel));
    cola_destruir(cola_en_nivel, NULL);
    return ultima_hoja;
}

/* Devuelve el mayor de los hijos de un nodo dado por parametro.
 * En caso de no tener hijos izquierdos, devuelve NULL (es un nodo
 * hoja). Caso contrario, devuelve el nodo correspondiente.
 */
nodo_heap_t* buscar_max_hijos(heap_t* heap, nodo_heap_t* padre){
    if (!padre->izq) return NULL; // El nodo padre pasado no tiene hijos.
    if (!padre->der) return padre->izq;
    int comparacion = heap->cmp(padre->izq->dato, padre->der->dato);
    if (comparacion >= 0) return padre->izq;
    return padre->der;
}

/* Ordena el heap desde el nodo pasado por parametro hacia abajo
 * (en caso de ser necesario) segun la funcion de comparacion dada.
 */
void down_heap(heap_t* heap, nodo_heap_t* actual){
    nodo_heap_t* hijo_mayor = buscar_max_hijos(heap, actual);
    if (!hijo_mayor) return; // Sucede al llegar al final del arbol.
    if (heap->cmp(actual->dato, hijo_mayor->dato) < 0){
        void* dato = actual->dato;
        actual->dato = hijo_mayor->dato;
        hijo_mayor->dato = dato;
        down_heap(heap, hijo_mayor);
    }
}

/* Elimina el nodo de mayor prioridad en el heap. Se verifica
 * que el heap quede ordenado por prioridad luego del borrado.
 * Finalmente, devuelve el elemento asociado al nodo.
 */
void* eliminar_primero(heap_t* heap){
    void* elemento = heap->raiz->dato;
    nodo_heap_t* ultima_hoja = buscar_ultima_hoja(heap);
    if (!ultima_hoja) return NULL;
    heap->raiz->dato = ultima_hoja->dato;
    nodo_heap_t* padre_ultima_hoja = ultima_hoja->padre;
    if (padre_ultima_hoja->izq == ultima_hoja)
        padre_ultima_hoja->izq = NULL;
    else
        padre_ultima_hoja->der = NULL;
    free(ultima_hoja);
    heap->cantidad--;
    down_heap(heap, heap->raiz);
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
    heap->cantidad = 0;
    heap->raiz = NULL;
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
    nodo_heap_t* nuevo_nodo = crear_nodo_heap(elem);
    if (!nuevo_nodo) return false;
    if (heap_esta_vacio(heap)){
        heap->raiz = nuevo_nodo;
        heap->cantidad++;
        return true;
    }
    return insertar_con_prioridad(heap, nuevo_nodo);
}

/* Devuelve el elemento con máxima prioridad. Si el heap esta vacío, devuelve
 * NULL. 
 * Pre: el heap fue creado.
 */
void* heap_ver_max(const heap_t* heap){
    if (heap_esta_vacio(heap)) return NULL;
    return heap->raiz->dato;
}

/* Elimina el elemento con máxima prioridad, y lo devuelve.
 * Si el heap esta vacío, devuelve NULL.
 * Pre: el heap fue creado.
 * Post: el elemento desencolado ya no se encuentra en el heap. 
 */
void* heap_desencolar(heap_t* heap){
    if (heap_esta_vacio(heap)) return NULL; // El heap esta vacio o es invalido.
    if (heap->cantidad == 1){
        void* elemento = heap->raiz->dato;
        free(heap->raiz);
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
    free(heap);
}

        // Primitiva Heap Sort //

/* Función de heapsort genérica. Esta función ordena mediante heap_sort
 * un arreglo de punteros opacos, para lo cual requiere que se
 * le pase una función de comparación. Modifica el arreglo "in-place".
 * Notar que esta función NO es formalmente parte del TAD Heap.
 */
void heap_sort(void *elementos[], size_t cant, cmp_func_t cmp){
    heap_t* arreglo_heap = heap_crear(cmp);
    if (!arreglo_heap) return;
    int i;
    for (i = 0; i < cant; i++){
        heap_encolar(arreglo_heap, elementos[i]);
    } // Tenemos el heap del arreglo ordenado segun la prioridad dada.
    for (i = 0; i < cant; i++){
        elementos[i] = heap_desencolar(arreglo_heap);
    }
    heap_destruir(arreglo_heap, NULL);
}
