#include "handlers.h"

/*
 * Définit le comportement d'un processus lorsque l'un de ses fils se termine.
 * On préfère un comportement non bloquant pour gérer les processus en arrière-plan
 */
 void finfils(int sig){ 
	sigset_t p_ens, p_anc;
	sigfillset(&p_ens); 
	sigprocmask(SIG_SETMASK, &p_ens, &p_anc); //Masquage de tout les signaux 
	int statut;
	pid_t pid;

	// on attend tous les fils de manière non bloquante (WNOHANG) 
	while((pid = waitpid(-1,&statut,WNOHANG))>0){}
	sigprocmask(SIG_SETMASK, &p_anc, &p_ens); //Demasquage des signaux
	return;
 }


