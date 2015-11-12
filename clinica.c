#include "clinica.h"

/***********************************
 *       ESTRUCTURAS DE DATOS      *
 ***********************************/

struct doctor {
	char* nombre;
	char* especialidad;
	int cant_atendidos;
};

struct paciente {
	char* nombre;
	unsigned long long total_contribuciones;
};

struct especialidad {
	char* nombre;
	heap_t* lista_de_espera;
};

struct parametros {
	char* comando;
	char* param1;
	char* param2;
};

int comp(const void* clave_a, const void* clave_b) {
	unsigned long long a = ((paciente_t*) clave_a)->total_contribuciones;
	unsigned long long b = ((paciente_t*) clave_b)->total_contribuciones;
	if (a == b) return 0;
	else if (a < b) return (-1);
	return 1;
}

cmp_func_t cmp = &comp;

/***********************************
 *        FUNCIONES AUXILIARES     *
 ***********************************/

// Función auxiliar para crear un doctor según un nombre y una especialidad, pasados como parámetro.
// Pre: Ninguna.
// Post: Se devuelve el doctor creado, NULL si no se pudo crear.
doctor_t* doctor_crear(char* nombre, char* especialidad) {
	doctor_t* doctor = malloc(sizeof(doctor_t));
	if (!doctor) return NULL;
	doctor->nombre = strcpy(malloc(strlen(nombre) + 1), nombre);
	doctor->especialidad = strcpy(malloc(strlen(especialidad) + 1), especialidad);
	doctor->cant_atendidos = 0;
	return doctor;
}

// Función auxiliar para crear un paciente según un nombre y total contribuído, pasados como parámetro.
// Pre: Ninguna.
// Post: Se devuelve el paciente creado, NULL si no se pudo crear o si el total contribuído no es un número.
paciente_t* paciente_crear(char* nombre, char* total_contribuciones) {
	paciente_t* paciente = malloc(sizeof(paciente_t));
	if (!paciente) return NULL;
	paciente->nombre = strcpy(malloc(strlen(nombre) + 1), nombre);
	char* ptrEnd;
	paciente->total_contribuciones = strtoull(total_contribuciones, &ptrEnd, 10);
	if (paciente->total_contribuciones == 0) return NULL;
	return paciente;
}

// Función auxiliar para crear un doctor según un nombre y una especialidad, pasados como parámetro.
// Pre: Ninguna.
// Post: Se devuelve la especialidad creada, NULL si no se pudo crear.
especialidad_t* especialidad_crear(char* nombre) {
	especialidad_t* especialidad = malloc(sizeof(especialidad_t));
	if (!especialidad) return NULL;
	especialidad->nombre = strcpy(malloc(strlen(nombre) + 1), nombre);
	especialidad->nombre[strlen(especialidad->nombre)] = '\0'; // Dado a que no hay fin de linea en los archivos csv
	heap_t* lista_de_espera = heap_crear(cmp);
	especialidad->lista_de_espera = lista_de_espera;
	return especialidad;
}

// Función auxiliar para parsear un texto ingresado por teclado.
// Pre: Ninguna.
// Post: Se devuelve el comando, y el/los parámetro(s) en una estructura, NULL si no se pudo crear la misma, o si no se ingresó un comando.
parametros_t* obtener_parametros() {
	parametros_t* parametros = malloc(sizeof(parametros_t));
	if (!parametros) return NULL;
	parametros->comando = NULL;
	parametros->param1 = NULL;
	parametros->param2 = NULL;
	
	csv_t cmd = {.delim = ':'};
	csv_siguiente(&cmd, stdin);
	if (cmd.primero) {
		if (strcmp(cmd.segundo, "") == 0) {
			if (strcmp(cmd.primero, "") != 0) printf(EINVAL_CMD);
			else {
				csv_terminar(&cmd);
				return NULL;
			}
		}
		else {
			parametros->comando = strcpy(malloc(strlen(cmd.primero) + 1), cmd.primero);
			parametros->param1 = strcpy(malloc(strlen(cmd.segundo) + 1), cmd.segundo);
			split(',', parametros->param1, &parametros->param2);
		}
		csv_terminar(&cmd);
		return parametros;
	}
	return NULL;
}
 
/***********************************
 *       FUNCIONES PRINCIPALES     *
 ***********************************/

// Función que genera un hash de doctores a partir de un archivo CSV (cuya ruta es el parámetro de la función).
// Pre: Ninguna.
// Post: Devuelve el hash de doctores si se pudo procesar el archivo, NULL si no pudo realizarse por algún motivo.
hash_t* generar_hash_doctores(char* archivo_doctores) {
	// Confirmo el archivo existe
	FILE *csv_doctores = fopen(archivo_doctores, "r");
	if (!csv_doctores) return NULL;
	
	// Proceso archivo de doctores
	csv_t linea = {.delim = ','};
	hash_t* hash_doctores = hash_crear(NULL);
	while (csv_siguiente(&linea, csv_doctores)) {
		if (strcmp(linea.segundo, "") != 0) {
			doctor_t* doctor = doctor_crear(linea.primero, linea.segundo);
			hash_guardar(hash_doctores, doctor->nombre, doctor);
		}
		else break;
	}
	fclose(csv_doctores);
	csv_terminar(&linea);
	
	return hash_doctores;
}

// Función que genera un hash de pacientes a partir de un archivo CSV (cuya ruta es el parámetro de la función).
// Pre: Ninguna.
// Post: Devuelve el hash de pacientes si se pudo procesar el archivo, NULL si no pudo realizarse por algún motivo.
hash_t* generar_hash_pacientes(char* archivo_pacientes) {
	// Confirmo que el archivo existe
	FILE *csv_pacientes = fopen(archivo_pacientes, "r");
	if (!csv_pacientes) return NULL;
	
	// Proceso archivo de pacientes
	csv_t linea = {.delim = ','};
	hash_t* hash_pacientes = hash_crear(NULL);
	while (csv_siguiente(&linea, csv_pacientes)) {
		if (strcmp(linea.segundo, "") != 0) {
			paciente_t* paciente = paciente_crear(linea.primero, linea.segundo);
			if (!paciente) return NULL;
			hash_guardar(hash_pacientes, paciente->nombre, paciente);
		}
		else break;
	}
	fclose(csv_pacientes);
	csv_terminar(&linea);
	
	return hash_pacientes;
}

// Función que genera un hash de especialidades a partir del hash de doctores previamente creado.
// Pre: El hash de doctores existe.
// Post: Devuelve el hash de especialidades, NULL si hubo algún error.
hash_t* generar_hash_especialidades(hash_t* hash_doctores) {
	hash_iter_t* iter = hash_iter_crear(hash_doctores);
	if (!iter) return NULL;
	
	hash_t* hash_especialidades = hash_crear(NULL);
	if (!hash_especialidades) return NULL;
	
	while (!hash_iter_al_final(iter)) {
		const char *clave = hash_iter_ver_actual(iter);
		doctor_t* doctor = hash_obtener(hash_doctores, clave);
		if (!hash_pertenece(hash_especialidades, doctor->especialidad)){
			especialidad_t* especialidad = especialidad_crear(doctor->especialidad);
			hash_guardar(hash_especialidades, especialidad->nombre, especialidad);
		}
		hash_iter_avanzar(iter);
	}
	hash_iter_destruir(iter);
	
	return hash_especialidades;
}

// Función que permite solicitar un turno para un paciente para una determinada especialidad.
// Pre: Los hashes de pacientes y especialidades existen.
// Post: Se encola un paciente en el heap de la especialidad ingresada por teclado, según su total contribuído.
//
// Salida por pantalla:
//
// Paciente NOMBRE_PACIENTE encolado
// N paciente(s) en espera para NOMBRE_ESPECIALIDAD
void pedir_turno(parametros_t* parametros, hash_t* hash_pacientes, hash_t* hash_especialidades) {
	paciente_t* paciente = hash_obtener(hash_pacientes, parametros->param1);
	if (!paciente) {
		printf(ENOENT_PACIENTE, parametros->param1);
		return;
	}
	especialidad_t* especialidad = hash_obtener(hash_especialidades, parametros->param2);
	if (!especialidad) {
		printf(ENOENT_ESPECIALIDAD, parametros->param2);
		return;
	}
	if (heap_encolar(especialidad->lista_de_espera, paciente)) {
		printf(PACIENTE_ENCOLADO, parametros->param1);
		printf(NUM_PACIENTES_ESPERAN, heap_cantidad(especialidad->lista_de_espera), especialidad->nombre);
	}
	return;
}

// Función que permite atender a un médico atender al paciente que esté primero en la cola de prioridad de su especialidad.
// Pre: Los hashes de doctores y especialidades existen.
// Post: El paciente es desencolado
//
// Salida por pantalla:
//
// Se atiende a NOMBRE_PACIENTE
// N paciente(s) en espera para NOMBRE_ESPECIALIDAD
void atender_siguiente(parametros_t* parametros, hash_t* hash_doctores, hash_t* hash_especialidades) {
	doctor_t* doctor = hash_obtener(hash_doctores, parametros->param1);
	if (!doctor) {
		printf(ENOENT_DOCTOR, parametros->param1);
		return;
	}
	especialidad_t* especialidad = hash_obtener(hash_especialidades, doctor->especialidad);
	if (!heap_cantidad(especialidad->lista_de_espera)){
		printf(CERO_PACIENTES_ESPERAN);
		return;
	}
	paciente_t* paciente = heap_desencolar(especialidad->lista_de_espera);
	doctor->cant_atendidos++;
	printf(PACIENTE_ATENDIDO, paciente->nombre);
	printf(NUM_PACIENTES_ESPERAN, heap_cantidad(especialidad->lista_de_espera), especialidad->nombre);
	return;
}

// Función que compara dos strings para determinar su orden alfabético.
int comparar_string(const void* clave_a, const void* clave_b) {
	return strcmp((char*) clave_b, (char*) clave_a);
}

cmp_func_t cmp_string = &comparar_string;

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
void mostrar_informe(hash_t* hash_doctores) {
	printf(NUM_DOCTORES, hash_cantidad(hash_doctores));	
	heap_t* doctores_orden = heap_crear(cmp_string);
	if (!doctores_orden) return;
	hash_iter_t* iter = hash_iter_crear(hash_doctores);
	if (!iter) return;
	while (!hash_iter_al_final(iter)) {
		const char *clave = hash_iter_ver_actual(iter);
		doctor_t* doctor = hash_obtener(hash_doctores, clave);
		heap_encolar(doctores_orden, doctor->nombre);
		hash_iter_avanzar(iter);
	}
	hash_iter_destruir(iter);
	unsigned int i = 1;
	while (!heap_esta_vacio(doctores_orden)){
		doctor_t* doctor = hash_obtener(hash_doctores, heap_desencolar(doctores_orden));
		printf(INFORME_DOCTOR, i, doctor->nombre, doctor->especialidad, doctor->cant_atendidos);
		i++;
	}
	return;
}

// Función principal del programa. Se le deben pasar por parámetro los nombres de los dos archivos CSV, sino aborta.
int main(int argc, char *argv[]) {
	// Si no se recibieron exactamente dos argumentos por la línea de comandos
	if (argc != 3) {
		return 1;
	}
	
	hash_t* hash_doctores = generar_hash_doctores(argv[1]);
	if (!hash_doctores) return 1;
	
	hash_t* hash_pacientes = generar_hash_pacientes(argv[2]);
	if (!hash_pacientes) return 1;
	
	hash_t* hash_especialidades = generar_hash_especialidades(hash_doctores);
	if (!hash_especialidades) return 1;
	
	bool fin = false;
	do {
		parametros_t* parametros = obtener_parametros();
		if (!parametros) fin = true;
			else if (parametros->comando) {
				if (strcmp(parametros->comando, "PEDIR_TURNO") == 0) pedir_turno(parametros, hash_pacientes, hash_especialidades);
				else if (strcmp(parametros->comando, "ATENDER_SIGUIENTE") == 0) atender_siguiente(parametros, hash_doctores, hash_especialidades);
				else if (strcmp(parametros->comando, "INFORME") == 0) mostrar_informe(hash_doctores);
				else printf(ENOENT_CMD, parametros->comando, parametros->param1);
			}
		free(parametros);
	} while (!fin);
	return 0;
}
