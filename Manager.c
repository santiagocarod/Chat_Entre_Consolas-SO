#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "Util.h" //Dentro de esta libreria viene incluida Info.h

int main(int argc, char **argv)
{
	int fdCS;							//Descriptor de archivo del pipe cliente-servidor
	int fdSC;							//Descriptor de archivo del pipe servidor-cliente
	int n;								//Número máximo de talkers
	int cuantos;						//Caracteres leidos
	int rProcesada;						//Bandera que indica si la operacion ha sido procesada
	msgToServer msg;					//Variable donde se guarda el mensaje enviado por un talker
	char *cadena;						//Varible auxiliar usada para separar en tokens los mensajes de los talkers
	char msgToClient[MAXSIZE + 1] = ""; //Variable donde se almacena el mensaje que se le va a enviar al cliente
	Talker *talkers;					//Arreglo de talkers
	Grupo grupos[MAXGRUPOS];			//Arreglo de grupos
	int cantGrupos = 0;					//Cantidad de grupos
	int salida = 0;						//Cuenta cuantos usuarios hay activos en el sistema para saber cuando salir.

	if (argc != 3)
	{
		/*Comprobacion de los argumentos del programa*/
		printf("./manager n pipeNom\n");
		exit(-1);
	}

	mode_t fifo_mode = S_IRUSR | S_IWUSR;

	n = atoi(argv[1]);
	printf("Manager iniciado. El sistema podra tener como maximo %d usuarios.\n", n);
	//Se crea arreglo de N talkers
	talkers = (Talker *)malloc(n * sizeof(Talker));
	//Se inicializan los talkers
	for (int i = 0; i < n; i++)
	{
		talkers[i].id = i + 1;
		talkers[i].creado = 0;
		talkers[i].conectado = 0;
		talkers[i].amigos = (int *)malloc(n * sizeof(int));
		for (int j = 0; j < n; j++)
		{
			talkers[i].amigos[j] = 0;
		}
	}

	unlink(argv[2]);
	//Crea pipe cliente-servidor
	if (mkfifo(argv[2], fifo_mode) == -1)
	{
		perror("Creando pipe cliente-servidor: ");
		exit(1);
	}

	//Abre pipe cliente-servidor
	fdCS = open(argv[2], O_RDONLY);

	//Se ejecuta ciclo para leer mensajes en el pipe cliente-servidor
	do
	{
		cuantos = read(fdCS, &msg, sizeof(msgToServer));
		if (cuantos == -1)
		{
			perror("Leyendo del pipe cliente-servidor: ");
			exit(1);
		}

		//Se procesa la solicitud
		//Se identifica la operación deseada y se ejecutan las instrucciones correspondientes
		rProcesada = 0;
		if (strcmp(msg.mensaje, "List") == 0)
		{
			printf("Talker %d solicita los usuarios conectados.\n", msg.id);
			rProcesada = 1;
			strcpy(msgToClient, List(talkers, n));
		}
		else if (strcmp(msg.mensaje, "List friends") == 0)
		{
			rProcesada = 1;
			strcpy(msgToClient, listFriends(talkers, msg));
		}
		else if (strcmp(msg.mensaje, "New") == 0)
		{
			rProcesada = 1;
			strcpy(msgToClient, New(talkers, msg, n, &salida));
		}
		else if (strcmp(msg.mensaje, "Salir") == 0)
		{
			rProcesada = 1;
			talkers[msg.id - 1].conectado = 0;
			sprintf(msgToClient, "Se ha cerrado la sesion del talker con ID = %d\n", msg.id);
			salida -= 1;
		}
		if (rProcesada == 0)
		{
			cadena = strtok(msg.mensaje, " ");
			if (strcmp(cadena, "List") == 0)
			{

				rProcesada = 1;
				cadena = strtok(NULL, " ");
				printf("Talker %d solicita los miembros del grupo %s.\n", msg.id, cadena);
				strcpy(msgToClient, listGroup(grupos, cadena, cantGrupos));
			}
			else if (strcmp(cadena, "Rel") == 0)
			{
				rProcesada = 1;
				cadena = strtok(NULL, " ");
				strcpy(msgToClient, Rel(talkers, msg, n, cadena));
			}
			else if (strcmp(cadena, "Group") == 0)
			{
				rProcesada = 1;
				cadena = strtok(NULL, " ");
				strcpy(msgToClient, group(talkers, grupos, n, &cantGrupos, msg, cadena));
			}
			else if (strcmp(cadena, "Sent") == 0)
			{
				rProcesada = 1;
				cadena = strtok(NULL, "\""); // Cadena tiene el mensaje a enviar
				strcpy(msgToClient, sent(talkers, grupos, n, cantGrupos, cadena, msg));
			}
		}
		if (rProcesada == 0)
		{
			strcpy(msgToClient, "Operacion no valida\n");
		}
		if (strcmp(msgToClient, "Error: El talker con el ID ingresado ya esta conectado.") == 0 || strcmp(msgToClient, "Error: El ID ingresado no es valido.") == 0)
		{
			comunicarNew(msg, msgToClient);
		}
		else
		{
			comunicar(talkers, msg.id, msgToClient);
		}
		sleep(1);
		close(fdSC);
	} while (salida > 0); //Se sale del ciclo cuando el pipe cliente-servidor haya sido cerrado por todos los clientes
	close(fdCS);
	return (0);
}
