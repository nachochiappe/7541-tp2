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
	long long total_contribuciones;
};

struct parametros {
	char* comando;
	char* param1;
	char* param2;
};

int comp(const void* clave_vieja, const void* clave_nueva) {
	// Si las claves son iguales, devuelve 0
	long long vieja = ((paciente_t*) clave_vieja)->total_contribuciones;
	long long nueva = ((paciente_t*) clave_nueva)->total_contribuciones;
	if (vieja == nueva) return 0;
	// Si clave nueva es mayor a clave vieja, return -1
	else if (nueva > vieja) return (-1);
	// Si clave nueva es menor a clave vieja, return 1
	return 1;
}

cmp_func_t cmp = &comp;

// Usar un heap como cola de prioridad (por especialidad) según total contribuido por cada paciente
// Usar un hash para la lista de doctores
// Usar un hash para la lista de pacientes

/***********************************
 *        FUNCIONES AUXILIARES     *
 ***********************************/

parametros_t* obtener_parametros() {
	parametros_t* parametros = malloc(sizeof(parametros_t));
	if (!parametros) return NULL;
	parametros->comando = NULL;
	parametros->param1 = NULL;
	parametros->param2 = NULL;
	
	csv_t cmd = {.delim = ':'};
	csv_siguiente(&cmd, stdin);
	parametros->comando = cmd.primero;
	parametros->param1 = cmd.segundo;
	split(':', parametros->param1, &parametros->param2);
	csv_terminar(&cmd);
	//csv_t args = {.delim = ','};
	//csv_siguiente(&args, cmd.segundo);
	//parametros->param1 = args.primero;
	//parametros->param2 = args.segundo;
	//csv_terminar(&args);
	
	return parametros;
}
 
/***********************************
 *       FUNCIONES PRINCIPALES     *
 ***********************************/

void pedir_turno(parametros_t* parametros) {
	return;
}

void atender_siguiente(parametros_t* parametros) {
	return;
}

void mostrar_informe(parametros_t* parametros) {
	return;
}

char abrir_sistema(char* archivo_doctores, char* archivo_pacientes) {
	// Confirmo que ambos archivos existen
	FILE *csv_doctores = fopen(archivo_doctores, "r");
	if (!csv_doctores) return 1;
	FILE *csv_pacientes = fopen(archivo_pacientes, "r");
	if (!csv_pacientes) return 1;
	
	// Proceso archivo de doctores
	csv_t linea = {.delim = ','};
	while (csv_siguiente(&linea, csv_doctores)) {
		printf("Primer campo: %s\n", linea.primero);
		printf("Segundo campo: %s\n", linea.segundo);
	}
	fclose(csv_doctores);
	
	// Proceso archivo de pacientes
	while (csv_siguiente(&linea, csv_pacientes)) {
		printf("Primer campo: %s\n", linea.primero);
		printf("Segundo campo: %s\n", linea.segundo);
	}
	fclose(csv_pacientes);
	
	csv_terminar(&linea);
	return 0;
}

int main(int argc, char *argv[]) {
	// Si no se recibieron exactamente dos argumentos por la línea de comandos
	if (argc != 3) {
		return 1;
	}
	if (abrir_sistema(argv[1], argv[2]) != 0) return 1;
	bool fin = false;
	do {
		parametros_t* parametros = obtener_parametros();
		if (!parametros->comando) fin = true;
			else if (strcmp(parametros->comando, "PEDIR_TURNO") == 0) pedir_turno(parametros);
				else if (strcmp(parametros->comando, "ATENDER_SIGUIENTE") == 0) atender_siguiente(parametros);
					else if (strcmp(parametros->comando, "INFORME") == 0) mostrar_informe(parametros);
						else {
							printf(ENOENT_CMD, parametros->comando, parametros->param1);
						}
	} while (!fin);
	return 0;
}