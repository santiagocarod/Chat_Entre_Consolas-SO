#include "Info.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief 
 * Esta funcion permite enviar un mensaje por medio de un pipe a alguno de los talkers que se
 * identifican por el id, abriendo el pipe escribiendo en el y despues enviando una señal en la 
 * que le avisa al talker que le tiene que leer el pipe ya que hay un mensaje para él
 * @param talkers 
 * Es el arreglo de talker de donde se puede extrar el PID para porder enviar la señal
 * @param id 
 * Es el ID del talker al cual se le va a enviar el mensaje y la señal
 * @param msg 
 * El mensaje que se escribira en el pipe
 */
void comunicar(Talker *talkers, int id, char *msg);

/**
 * @brief 
 * Esta funcion envia por medio de un pipe un mensaje de error en caso de que no se pueda
 * crear de forma exitosa una nueva conexion
 * 
 * @param msg 
 * El mensaje que fue recibido por el manager para poder saber a quien responderle ya que
 * de esta variable se saca el pid a quien se le va a enviar la señal
 * @param mesg 
 * El mensaje que va a ser escrito en el pipe
 */
void comunicarNew(msgToServer msg, char *mesg);

/**
 * @brief 
 * Esta funcion devuelve todos los usuarios que estan conectados al momento que algun talker la llame
 * para saber con quien se puede comuncar
 * @param talkers 
 * El arreglo de talkers del cual va a extraer la informacion para poder saber quien esta conectado
 * @param n 
 * La cantidad de talkers que pueden estar conectados, esta informacion llega por parametro cuando se corre el manager
 * @return char* 
 * Retorna una cadena de caracteres informando los talkers que estan conectados. Informacion que se envia al cliente. 
 */
char *List(Talker *talkers, int n);

/**
 * @brief 
 * Esta función se encarga de informar al talker que la invoque cual es su lista de amigos, por
 * medio de una cadena de caracteres la cual le provee esta información
 * @param talkers 
 * Es el arreglo de talkers para buscar la lista de amigos del talker que invoca la función.
 * @param msg 
 * Es el mensaje recibido por el manager para poder saber quien fue el talker que envio la
 * peticion y asi extraer su lista de amigos
 * @return char* 
 * La lista de amigos en forma de cadena de caracteres.
 */
char *listFriends(Talker *talkers, msgToServer msg);

/**
 * @brief 
 * Esta funcion se encarga de ingresar un nuevo talker al sistema asignadole su id y guardando su
 * pid para mas tarde poder enviarle señales, ademas asignando estados de creado y conectado
 * @param talkers 
 * La lista de talkers en donde se va a agregar este nuevo talker y se va a 
 * guardar la información correspondiente con el.
 * @param msg 
 * Es el mensaje recibido por el manager para poder saber quien fue el talker que envio la
 * peticion y asi extraer informacion del talker como el PID y su ID
 * @param n 
 * La cantidad maxima de talkers que puede haber
 * @param salida 
 * Esta es la variable que lleva la cuenta de cuantos usuarios hay activos en el sistema,
 * en caso de ser exitosa la conexion se aumenta en 1.
 * @return char* 
 * Retorna un mensaje de exito en caso de ser exitosa la operacion de ingresar el nuevo
 * talker y un mensaje de Error por lo contrario
 */
char *New(Talker *talkers, msgToServer msg, int n, int *salida);

/**
 * @brief 
 * Esta funcion permite al talker saber quienes estan registrados como parte de un grupo
 * @param grupos 
 * Es el arreglo de grupos para poder extraer la informacion solicitada por el talker
 * @param cadena 
 * En esta cadena viene el GID el cual va a ser comparado
 * con los que estan guardados en el arreglo y asi poder sacar la información
 * @param cantGrupos 
 * La cantidad de grupos que hasta al momento han sido registrados.
 * @return char* 
 * Retorna la cadena informando los ID's de los talkers que se encuentran dentro del grupo
 * y en caso de que el grupo no exista devuelve un mensaje de error. 
 */
char *listGroup(Grupo *grupos, char *cadena, int cantGrupos);

/**
 * @brief 
 * Esta funcion permite agregar una relacion entre dos talkers, buscando la lista de amigos de 
 * ambos y agregandose mutuamente a la lista de amigos, enviando un mensaje de aviso a 
 * ambos talkers para que sepan que ahora son amigos.
 * @param talkers 
 * Es el arreglo de talkers en donde se buscaran a ambos talkers para poder agregar
 * esta nueva relacion
 * @param msg 
 * En esta mensaje viene la informacion del talker quien quiere hacer la relacion entre
 * ambos
 * @param n 
 * Es el tamaño maximo del arreglo de talkers 
 * @param cadena 
 * En esta cadena viene el ID del segundo talker al cual se le notificara de la nueva
 * relacion
 * @return char* 
 * Retorna el mensaje de exito o de error para el Talker quien esta tratando de crear esta 
 * nueva relacion.
 */
char *Rel(Talker *talkers, msgToServer msg, int n, char *cadena);

/**
 * @brief 
 * Esta función permite crear un grupo,Primero se tieneque revisar que todos los talkers 
 * existan en el arreglo de talkers para saber si pueden hacer parte del grupo
 * despues se tiene que revisar que todos los IDs del Talker
 * que esta tratando de crear el grupo sean amigos del mismo, ya que si alguno no es amigo
 * no se puede hacer un grupo con este talker. Una vez verificado todo esto se crea el grupo
 * y se le asigna un unico GID el cual identifica el grupo y sus integrantes
 * 
 * @param talkers 
 * Es el arreglo de talkers en donde se va a buscar si el talker que va a hacer parte del grupo
 * existe y si esta conectado se le envia una señal diciendole que ahora hace parte de un grupo
 * @param grupos 
 * Es el arreglo de grupos en donde se va a guardar la nueva informacion de este grupo, tanto
 * el GID como los miembros del grupo
 * @param n 
 * La cantidad maxima de talkers que pueden haber en el sistema
 * @param cantGrupos 
 * La cantidad de grupos que han sido creados, este se recibe por referencia ya que dentro de si
 * se va a cambiar y se va a aumentar ya que agrega un nuevo grupo
 * @param msg
 * Dentro de este mensaje esta la informacion del talker que quiere crear el grupo y tambien es
 * agregado al grupo
 * @param cadena 
 * Dentro de esta cadena estan los ID's de los miembros del nuevo grupo los cuales van a ser 
 * revisados uno por uno que cumplan todas los requerimientos
 * @return char* 
 * Retorna un mensaje de exito para el creador en dado caso que se pueda crear el grupo
 * y un mensaje de error en caso de que haya algun problema al momento de crear este nuevo
 * grupo.
 */
char *group(Talker *talkers, Grupo *grupos, int n, int *cantGrupos, msgToServer msg, char *cadena);

/**
 * @brief 
 * Esta funcion permite enviar un mensaje de parte de un talker ya sea a un grupo o a otro talker,
 * en caso de que sea a un grupo tiene que verificar que el talker si haga parte del grupo al 
 * que va a mandar el mensaje y en caso de que si lo sea manda a cada uno de los talkers
 * que esten conectados el mensaje que envio el talker.
 * En caso de que sea un mensaje para otro talker en especifico entonces tiene que revisar que este
 * talker sea parte de la lista de sus amigos ya que si no son amigos no se pueden enviar mensajes.
 * En caso de que el primer caracter del ID sea una 'G' se sabe que el mensaje va a un grupo
 * @param talkers 
 * Es el arreglo de talkers en donde se va a buscar la informacion si son amigos.
 * @param grupos 
 * Es el arreglo de grupos en donde cada grupo tiene sus miembros en donde se va a buscar si el 
 * GID en donde se esta tratando de mandar un mensaje si exista y ademas el talker que trata de 
 * enviar el mensaje haga parte de de los miembros
 * @param cantGrupos 
 * Es la cantidad de grupos creados hasta ahora.
 * @param cadena 
 * En esta cadena esta tanto el mensaje que se quiere enviar como el destinatario del respectivo mensaje
 * ya sea un grupo o otro talker.
 * @param msg 
 * En este mensaje esta la informacion del talker que envia el mensaje para saber si es parte del
 * grupo o avisarle al destinatario de quien es el mensaje
 * @return char* 
 * Retorna un Mensaje de error en caso de que el mensaje no pueda ser enviado al destinatario.
 */
char *sent(Talker *talkers, Grupo *grupos, int n, int cantGrupos, char *cadena, msgToServer msg);
