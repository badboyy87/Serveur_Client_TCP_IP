#include <stdlib.h>
#include <error.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


/*
 * définit le comportement d'un processus lorsque l'un de ses fils se termine.
 * on préfère un comportement non bloquant pour gérer les processus en arrière-plan
 */
 void finfils(int sig);
