#include "Util.h"

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
void comunicar(Talker *talkers, int id, char *msg)
{
    int fdSC;
    char pipeTalker[10]; //Nombre del pipe que comunica al manager con el talker
    sprintf(pipeTalker, "pipe%d", id);
    //Se abre pipe servidor-cliente
    if ((fdSC = open(pipeTalker, O_WRONLY | O_NONBLOCK)) == -1)
    {
        perror("Abriendo pipe servidor-cliente: ");
        exit(0);
    }
    //printf("Server va a eviar el mensaje: %s\n", msgToClient);
    // Se envía señal con el mensaje de respuesta
    write(fdSC, msg, MAXSIZE);
    //Se envía señal para que el cliente lea el mensaje de respuesta
    if (kill(talkers[id - 1].pid, SIGUSR1) == -1)
    {
        perror("Enviando signal: ");
        exit(0);
    }
}

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
void comunicarNew(msgToServer msg, char *mesg)
{
    int fdSC;
    char pipeTalker[10]; //Nombre del pipe que comunica al manager con el talker
    sprintf(pipeTalker, "pipe%d", msg.id);
    //Se abre pipe servidor-cliente
    if ((fdSC = open(pipeTalker, O_WRONLY | O_NONBLOCK)) == -1)
    {
        perror("Abriendo pipe servidor-cliente: ");
        exit(0);
    }
    //printf("Server va a eviar el mensaje: %s\n", msgToClient);
    // Se envía señal con el mensaje de respuesta
    write(fdSC, mesg, MAXSIZE);
    //Se envía señal para que el cliente lea el mensaje de respuesta
    if (kill(msg.pid, SIGUSR1) == -1)
    {
        perror("Enviando signal: ");
        exit(0);
    }
}

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
char *List(Talker *talkers, int n)
{
    char id[5] = "";
    static char msgToClient[MAXSIZE + 1] = "";
    strcpy(msgToClient, "Los usuarios actualmente en el sistema son: ");
    for (int i = 0; i < n; i++)
    {
        if (talkers[i].conectado == 1)
        {
            sprintf(id, "%d, ", i + 1);
            strcat(msgToClient, id);
        }
    }
    strcat(msgToClient, "\n");
    return msgToClient;
}

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
char *listFriends(Talker *talkers, msgToServer msg)
{
    char id[5] = "";
    static char msgToClient[MAXSIZE + 1] = "";
    int bandera = 0, i = 0;
    printf("Talker %d solicita sus amigos.\n", msg.id);
    strcpy(msgToClient, "Amigos: ");
    while (bandera == 0)
    {
        if (talkers[msg.id - 1].amigos[i] == 0)
        {
            bandera = 1;
        }
        else
        {
            sprintf(id, "%d, ", talkers[msg.id - 1].amigos[i]);
            strcat(msgToClient, id);
        }
        i++;
    }
    return msgToClient;
}

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
char *New(Talker *talkers, msgToServer msg, int n, int *salida)
{
    static char msgToClient[MAXSIZE + 1] = "";
    if (msg.id > 0 && msg.id <= n)
    {
        if (talkers[msg.id - 1].conectado == 1)
        {
            strcpy(msgToClient, "Error: El talker con el ID ingresado ya esta conectado.");
        }
        else
        {
            talkers[msg.id - 1].creado = 1;
            talkers[msg.id - 1].conectado = 1;
            talkers[msg.id - 1].id = msg.id;
            talkers[msg.id - 1].pid = msg.pid;
            talkers[msg.id - 1].cantAmigos = 0;
            talkers[msg.id - 1].amigos = (int *)malloc(n * sizeof(int));
            sprintf(msgToClient, "Se inicializa un talker con ID = %d\n", msg.id);
            *salida += 1;
        }
    }
    else
    {
        sprintf(msgToClient, "Error: El ID ingresado no es valido.");
    }
    return msgToClient;
}

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
char *listGroup(Grupo *grupos, char *cadena, int cantGrupos)
{
    static char msgToClient[MAXSIZE + 1] = "";
    char id[5] = "";
    int entro = 0, bandera = 0, existe = 0;
    for (int i = 0; i < cantGrupos; i++)
    {
        if (strcmp(grupos[i].gid, cadena) == 0)
        {
            entro = 1;
            int a = 0;
            sprintf(msgToClient, "El grupo %s tiene como miembros: ", cadena);
            while (bandera == 0)
            {
                if (grupos[i].miembros[a] == 0)
                {
                    bandera = 1;
                }
                else
                {
                    sprintf(id, "%d, ", grupos[i].miembros[a]);
                    strcat(msgToClient, id);
                }
                a++;
            }
        }
    }
    if (entro == 0)
    {
        printf("El grupo %s no existe.\n", cadena);
        sprintf(msgToClient, "Error: El grupo %s no existe: ", cadena);
    }
    return msgToClient;
}

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
char *Rel(Talker *talkers, msgToServer msg, int n, char *cadena)
{
    static char msgToClient[MAXSIZE + 1] = "";
    int relID;
    if ((relID = atoi(cadena)) != 0)
    {
        printf("Talker %d desea establecer relacion con Talker %d. ", msg.id, relID);
        if (relID > 0 && relID <= n)
        {
            if (talkers[relID - 1].conectado == 1)
            {
                //Se forma la relacion
                talkers[relID - 1].amigos[talkers[relID - 1].cantAmigos] = msg.id;
                talkers[relID - 1].cantAmigos++;
                talkers[msg.id - 1].amigos[talkers[msg.id - 1].cantAmigos] = relID;
                talkers[msg.id - 1].cantAmigos++;
                //Se envia mensajes a ambos
                char msgRel[MAXSIZE + 1];
                sprintf(msgRel, "Se establece una relación entre los Talkers %d y %d ", msg.id, relID);
                strcpy(msgToClient, msgRel);
                printf("Relacion establecida en el manager.\n");
                comunicar(talkers, relID, msgRel);
            }
            else
            {
                printf("Talker %d no conectado. Se devuelve error.\n", relID);
                sprintf(msgToClient, "Error: el Talker %d no esta conectado", relID);
            }
        }
        else
        {
            printf("El ID ingresado no es valido.\n");
            sprintf(msgToClient, "Error: el ID tiene que ser un numero del 1 al %d", n);
        }
    }
    else
    {
        sprintf(msgToClient, "Error: en ID tiene que ser un numero del 1 al %d", n);
    }
    return msgToClient;
}

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
char *group(Talker *talkers, Grupo *grupos, int n, int *cantGrupos, msgToServer msg, char *cadena)
{
    static char msgToClient[MAXSIZE + 1] = "";
    int indi = 0;
    int esAmigo = 0;
    int groupError = 0;
    int *miembros;
    miembros = (int *)malloc(n * sizeof(int));
    printf("Talker %d solicita creacion de grupo con amigos %s\n", msg.id, cadena);
    char *idsGroup = strtok(cadena, ", ");
    char msgRel[MAXSIZE + 1];
    while (idsGroup != NULL)
    {
        miembros[indi] = atoi(idsGroup);
        //printf("Evaluando miembro %d\n", miembros[indi]);
        esAmigo = 0;
        for (int i = 0; i < n; i++)
        {
            if (miembros[indi] == talkers[msg.id - 1].amigos[i])
            {
                esAmigo = 1;
            }
        }
        //printf("Es amigo: %d\n", esAmigo);
        //printf("Creado: %d\n", talkers[miembros[indi] - 1].creado);
        if (talkers[miembros[indi] - 1].creado == 0 || esAmigo == 0)
        {
            groupError = 1;
            break;
        }
        idsGroup = strtok(NULL, ", ");
        indi += 1;
    }
    if (groupError == 0)
    {
        sprintf(grupos[*cantGrupos].gid, "G%d", *cantGrupos + 1);
        sprintf(msgRel, "Hace parte del grupo %s", grupos[*cantGrupos].gid);
        sprintf(msgToClient, "Se forma el grupo %s", grupos[*cantGrupos].gid);
        printf("Se crea el grupo %s\n", grupos[*cantGrupos].gid);
        grupos[*cantGrupos].miembros = (int *)malloc(n * sizeof(int));
        for (int i = 0; i < indi; i++)
        {
            grupos[*cantGrupos].miembros[i] = miembros[i];
            //printf("SE AGREGO: %d", grupos[*cantGrupos].miembros[i]);
            if (talkers[miembros[i] - 1].conectado == 1)
            {
                comunicar(talkers, miembros[i], msgRel);
            }
        }
        for (int i = indi; i < n; i++)
        {
            grupos[*cantGrupos].miembros[i] = 0;
        }
        grupos[*cantGrupos].miembros[indi] = msg.id;
        *(cantGrupos) += 1;
    }
    else
    {
        printf("Error en la creacion del grupo\n");
        strcpy(msgToClient, "Error en la creacion del grupo\n");
    }
    return msgToClient;
}

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
char *sent(Talker *talkers, Grupo *grupos, int n, int cantGrupos, char *cadena, msgToServer msg)
{
    static char msgToClient[MAXSIZE + 1] = "";
    char enviar[MAXSIZE + 1];
    //sprintf(enviar, "Talker %d envia: \n %s", msg.id, cadena);
    strcpy(enviar, cadena);
    cadena = strtok(NULL, " "); //cadena ahora tiene el GroupID o el IDi
    int error = 0;
    int bandera = 0, amigos = 0, con = 0, miembro = 0;
    if (cadena[0] == 'G')
    {
        for (int i = 0; i < cantGrupos; i++)
        {
            if (strcmp(grupos[i].gid, cadena) == 0)
            {
                Grupo g = grupos[i];
                while (bandera == 0)
                {
                    if (g.miembros[con] == 0)
                    {
                        bandera = 1;
                    }
                    else
                    {
                        if (g.miembros[con] == msg.id)
                        {
                            miembro = 1;
                        }
                    }
                    con++;
                }
                if (miembro == 1)
                {
                    bandera = 0;
                    con = 0;
                    char enviarGrupo[MAXSIZE + 1];
                    sprintf(enviarGrupo, "%s \n viene de %s", enviar, cadena);
                    printf("Talker %d envia \"%s\" a %s\n", msg.id, enviar, cadena);
                    amigos = 1;

                    while (bandera == 0)
                    {
                        if (g.miembros[con] == 0)
                        {
                            bandera = 1;
                        }
                        else
                        {
                            if (talkers[g.miembros[con] - 1].conectado == 1)
                            {
                                comunicar(talkers, g.miembros[con], enviarGrupo);
                            }
                        }
                        con++;
                    }
                }
                else
                {
                    strcpy(msgToClient, "Lo sentimos pero usted no es miembro de este grupo");
                }
            }
        }
        if (amigos == 0)
        {
            sprintf(msgToClient, "El grupo %s no existe", cadena);
        }
        else
        {
            strcpy(msgToClient, "");
        }
    }
    else
    {
        int destino = atoi(cadena);
        error = destino;
        if (destino < 0 || destino > n)
        {
            sprintf(msgToClient, "El ID %d esta por fuera de los limites", destino);
        }
        else if (talkers[destino - 1].conectado == 1)
        {
            while (bandera == 0)
            {
                if (talkers[msg.id - 1].amigos[con] == 0)
                {
                    bandera = 1;
                }
                else
                {
                    if (talkers[msg.id - 1].amigos[con] == destino)
                    {
                        amigos = 1;
                    }
                }
                con++;
            }
            if (amigos == 1)
            {
                char enviarTalker[MAXSIZE + 1];
                sprintf(enviarTalker, "Talker %d envia: \n %s", msg.id, enviar);
                comunicar(talkers, destino, enviarTalker);
                strcpy(msgToClient, " ");
            }
            else
            {
                sprintf(msgToClient, "Usted y el usuario %d no son amigos", error);
            }
        }
        else
        {
            sprintf(msgToClient, "El usuario %d esta desconectado", error);
        }
    }
    return msgToClient;
}
