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
	if (paciente->total_contribuciones == 0) return NULL;
	return paciente;
}

especialidad_t* especialidad_crear(char* nombre) {
	especialidad_t* especialidad = malloc(sizeof(especialidad_t));
	if (!especialidad) return NULL;
	especialidad->nombre = strcpy(malloc(strlen(nombre) + 1), nombre);
	especialidad->nombre[strlen(especialidad->nombre)] = '\0'; // Dado a que no hay fin de linea en los archivos csv
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
	if (strcmp(cmd.segundo, "") == 0) {
		printf(EINVAL_CMD);
		csv_terminar(&cmd);
		return NULL;
	}
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

int comparar_string(const void* clave_a, const void* clave_b) {
	return strcmp((char*) clave_b, (char*) clave_a);
}

cmp_func_t cmp_string = &comparar_string;


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
			else if (strcmp(parametros->comando, "PEDIR_TURNO") == 0) pedir_turno(parametros, hash_pacientes, hash_especialidades);
				else if (strcmp(parametros->comando, "ATENDER_SIGUIENTE") == 0) atender_siguiente(parametros, hash_doctores, hash_especialidades);
					else if (strcmp(parametros->comando, "INFORME") == 0) mostrar_informe(hash_doctores);
						else {
							printf(ENOENT_CMD, parametros->comando, parametros->param1);
						}
		free(parametros);
	} while (!fin);
	return 0;
}
