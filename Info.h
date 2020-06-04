#define MAXSIZE 100
#define MAXGRUPOS 20

/**
 * @brief 
 * Estructura que guarda la informacion de un mensaje para el manager, 
 * guarda el ID del talker que envia el mensaje el PID para que le puedan 
 * enviar señales y ademas el mensaje que quiere enviar
 * 
 */
typedef struct msgToSer
{
	int id; //ID del talker que envía el mensaje
	int pid;
	char mensaje[MAXSIZE];
} msgToServer;

/**
 * @brief 
 * Es la estructura que guarda un talker con toda su informacion y su estado
 * para saber si esta conectado o creado y la cantidad de amigos que tiene y ademas
 * tambien el arreglo de amigos y su PID para enviarle señales
 */
typedef struct Talk
{
	int id;
	int creado;	//Indica si el talker si ha conectado por primera vez
	int conectado; //Indica si el talker se encuentra conectado
	int cantAmigos;
	int *amigos;
	int pid;
} Talker;

/**
 * @brief 
 * Es la estructura que guarda el Id del grupo o GID y la lista de sus miembros
 */
typedef struct group
{
	char gid[10];
	int *miembros;
} Grupo;
