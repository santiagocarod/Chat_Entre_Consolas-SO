#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "Info.h"

int fdSC;				   //Descriptor de archivo para el pipe servidor-cliente
char mensaje[MAXSIZE + 1]; //Mensaje que lee el talker en el signalHandler

/**
 * @brief 
 * Es la funcion encargada de manejar las señales enviadas por el manager,
 * se encarga de leer el mensaje que fue dejado en su respectivo pipe y mostrarlo al usuario
 * ademas de despues de esto volver a mostrar el menu
 * @param signal 
 */
void signalHandler(int signal)
{
	//printf("Signal Handler\n");
	//Se lee el mensaje enviado por el servidor
	read(fdSC, mensaje, MAXSIZE);
	//Se imprime el mensaje enviado por el servidor
	printf("%s \n", mensaje);
	printf("Ingrese la operación a realizar: \n");
}

int main(int argc, char **argv)
{
	int fdCS;				   //Descriptor de archivo para el pipe cliente-servidor
	int id;					   //Descriptor de archivo para el pipe cliente-servidor
	int abierto = 0;		   //Bandera que indica si el pipe cliente-servidor ha sido abierto
	char nombreSC[10];		   //Cadena de caracteres con el nombre del pipe servidor-cliente
	char entrada[MAXSIZE + 1]; //Cadena de caracteres con la entrada del usuario
	msgToServer msg;		   //Mensaje que se le va a enviar al servidor

	mode_t fifo_mode = S_IRUSR | S_IWUSR;

	//Se instala el manejador de la señal
	signal(SIGUSR1, signalHandler);
	id = atoi(argv[1]);

	if (argc != 3)
	{
		/*Comprobacion de los argumentos del programa*/
		printf("./talker ID pipeNom\n");
		exit(-1);
	}

	do
	{
		//Se abre pipe cliente-servidor
		fdCS = open(argv[2], O_WRONLY);
		if (fdCS == -1)
		{
			perror("pipe");
			printf(" Se volvera a intentar despues\n");
			sleep(20);
		}
		else
		{
			abierto = 1;
		}
	} while (abierto == 0);

	//Se utiliza el id del talker para nombrar el pipe servidor-cliente
	sprintf(nombreSC, "pipe%d", id);

	//Se crea pipe servidor-cliente

	unlink(nombreSC);
	if (mkfifo(nombreSC, fifo_mode) == -1)
	{
		perror("Talker creando pipe servidor-cliente:");
		exit(1);
	}

	//Se abre pipe servidor-cliente

	if ((fdSC = open(nombreSC, O_RDONLY | O_NONBLOCK)) == -1)
	{
		perror("Talker abriendo pipe servidor-cliente:");
		exit(0);
	}

	//El talker se registra con el manager
	strcpy(msg.mensaje, "New");
	msg.id = id;
	msg.pid = getpid();
	//Se envía mensaje usando pipe cliente-servidor
	write(fdCS, &msg, sizeof(msg));
	//Se espera a recibir la señal para leer el mensaje de respuesta
	pause();
	if (strcmp(mensaje, "Error: El talker con el ID ingresado ya esta conectado.") == 0 || strcmp(mensaje, "Error: El ID ingresado no es valido.") == 0)
	{
		sleep(0.5);
		close(fdSC);
		close(fdCS);
		exit(0); //Termina
	}
	while (1) //Ciclo para solicitar operaciones.
	{
		//Se solicita operación

		setbuf(stdin, NULL);
		scanf("%[^\n]s", entrada);
		//Escribe mensaje en el pipe cliente-servidor
		strcpy(msg.mensaje, entrada);
		msg.id = id;
		msg.pid = getpid();
		write(fdCS, &msg, sizeof(msg));
		//Si es salir
		if (strcmp(entrada, "Salir") == 0)
		{
			pause();
			sleep(0.5);
			//Se cierran los pipes que habían sido abiertos
			close(fdSC);
			close(fdCS);
			exit(0); //Termina
		}
		// el cliente espera a que llegue la señal para leer el mensaje de respuesta
		pause();
	}
}
