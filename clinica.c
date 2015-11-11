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

int comp(const void* clave_vieja, const void* clave_nueva) {
	// Si las claves son iguales, devuelve 0
	unsigned long long vieja = ((paciente_t*) clave_vieja)->total_contribuciones;
	unsigned long long nueva = ((paciente_t*) clave_nueva)->total_contribuciones;
	if (vieja == nueva) return 0;
	// Si clave nueva es mayor a clave vieja, return -1
	else if (nueva > vieja) return (-1);
	// Si clave nueva es menor a clave vieja, return 1
	return 1;
}

cmp_func_t cmp = &comp;

/***********************************
 *        FUNCIONES AUXILIARES     *
 ***********************************/

doctor_t* doctor_crear(char* nombre, char* especialidad) {
	doctor_t* doctor = malloc(sizeof(doctor_t));
	if (!doctor) return NULL;
	doctor->nombre = strcpy(malloc(strlen(nombre) + 1), nombre);
	doctor->especialidad = strcpy(malloc(strlen(especialidad) + 1), especialidad);
	doctor->cant_atendidos = 0;
	return doctor;
}

paciente_t* paciente_crear(char* nombre, char* total_contribuciones) {
	paciente_t* paciente = malloc(sizeof(paciente_t));
	if (!paciente) return NULL;
	paciente->nombre = strcpy(malloc(strlen(nombre) + 1), nombre);
	char* ptrEnd;
	paciente->total_contribuciones = strtoull(total_contribuciones, &ptrEnd, 10);
	return paciente;
}

especialidad_t* especialidad_crear(char* nombre) {
	especialidad_t* especialidad = malloc(sizeof(especialidad_t));
	if (!especialidad) return NULL;
	especialidad->nombre = strcpy(malloc(strlen(nombre) + 1), nombre);
	especialidad->nombre[strlen(especialidad->nombre) - 1] = '\0'; // Lo uso para remover el caracter de nueva línea \n
	heap_t* lista_de_espera = heap_crear(cmp);
	especialidad->lista_de_espera = lista_de_espera;
	return especialidad;
}

parametros_t* obtener_parametros() {
	parametros_t* parametros = malloc(sizeof(parametros_t));
	if (!parametros) return NULL;
	parametros->comando = NULL;
	parametros->param1 = NULL;
	parametros->param2 = NULL;
	
	csv_t cmd = {.delim = ':'};
	csv_siguiente(&cmd, stdin);
	parametros->comando = strcpy(malloc(strlen(cmd.primero) + 1), cmd.primero);
	parametros->param1 = strcpy(malloc(strlen(cmd.segundo) + 1), cmd.segundo);
	csv_terminar(&cmd);
	split(',', parametros->param1, &parametros->param2);
	return parametros;
}
 
/***********************************
 *       FUNCIONES PRINCIPALES     *
 ***********************************/

hash_t* generar_hash_doctores(char* archivo_doctores) {
	// Confirmo el archivo existe
	FILE *csv_doctores = fopen(archivo_doctores, "r");
	if (!csv_doctores) return NULL;
	
	// Proceso archivo de doctores
	csv_t linea = {.delim = ','};
	hash_t* hash_doctores = hash_crear(NULL);
	while (csv_siguiente(&linea, csv_doctores)) {
		doctor_t* doctor = doctor_crear(linea.primero, linea.segundo);
		hash_guardar(hash_doctores, doctor->nombre, doctor);
	}
	fclose(csv_doctores);
	csv_terminar(&linea);
	
	return hash_doctores;
}

hash_t* generar_hash_pacientes(char* archivo_pacientes) {
	// Confirmo que el archivo existe
	FILE *csv_pacientes = fopen(archivo_pacientes, "r");
	if (!csv_pacientes) return NULL;
	
	// Proceso archivo de pacientes
	csv_t linea = {.delim = ','};
	hash_t* hash_pacientes = hash_crear(NULL);
	while (csv_siguiente(&linea, csv_pacientes)) {
		paciente_t* paciente = paciente_crear(linea.primero, linea.segundo);
		hash_guardar(hash_pacientes, paciente->nombre, paciente);
	}
	fclose(csv_pacientes);
	csv_terminar(&linea);
	
	return hash_pacientes;
}

hash_t* generar_hash_especialidades(hash_t* hash_doctores) {
	hash_iter_t* iter = hash_iter_crear(hash_doctores);
	if (!iter) return NULL;
	
	hash_t* hash_especialidades = hash_crear(NULL);
	if (!hash_especialidades) return NULL;
	
	while (!hash_iter_al_final(iter)) {
		const char *clave = hash_iter_ver_actual(iter);
		doctor_t* doctor = hash_obtener(hash_doctores, clave);
		especialidad_t* especialidad = especialidad_crear(doctor->especialidad);
		if (hash_pertenece(hash_especialidades, especialidad->nombre)) free(especialidad);
		else hash_guardar(hash_especialidades, especialidad->nombre, especialidad);
		hash_iter_avanzar(iter);
	}
	hash_iter_destruir(iter);
	
	return hash_especialidades;
}

void pedir_turno(parametros_t* parametros, hash_t* hash_pacientes, hash_t* hash_especialidades) {
	if (!hash_pertenece(hash_pacientes, parametros->param1)) {
		printf(ENOENT_PACIENTE, parametros->param1);
		return;
	}
	if (!hash_pertenece(hash_especialidades, parametros->param2)) {
		printf(ENOENT_ESPECIALIDAD, parametros->param2);
		return;
	}
	paciente_t* paciente = hash_obtener(hash_pacientes, parametros->param1);
	especialidad_t* especialidad = hash_obtener(hash_especialidades, parametros->param2);
	if (heap_encolar(especialidad->lista_de_espera, paciente)) {
		printf(PACIENTE_ENCOLADO, parametros->param1);
		size_t cant_en_espera = heap_cantidad(especialidad->lista_de_espera);
		printf(NUM_PACIENTES_ESPERAN, cant_en_espera, parametros->param2);
	}
	return;
}

void atender_siguiente(parametros_t* parametros) {
	printf("Entre a atender_siguiente\n");
	return;
}

void mostrar_informe(parametros_t* parametros) {
	printf("Entre a mostrar_informe\n");
	return;
}

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
		if (!parametros->comando) fin = true;
			else if (strcmp(parametros->comando, "PEDIR_TURNO") == 0) pedir_turno(parametros, hash_pacientes, hash_especialidades);
				else if (strcmp(parametros->comando, "ATENDER_SIGUIENTE") == 0) atender_siguiente(parametros);
					else if (strcmp(parametros->comando, "INFORME") == 0) mostrar_informe(parametros);
						else {
							printf(ENOENT_CMD, parametros->comando, parametros->param1);
						}
		free(parametros);
	} while (!fin);
	return 0;
}