#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "handlers.h"
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include "controle.h"

int main(int argc, char *argv[]){


	/* ****************** SIGNAUX ****************** */
	
	struct sigaction a;
	a.sa_handler = finfils;
	a.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &a, NULL);


	/* ****************** SERVEUR ****************** */

	/* initialisation de la liste des blacklistés */
	struct filelist{
		char filename[25];	// le nom du fichier
		int checksum;		// le checksum correspondant
	};

	// liste des images blacklistées à partir du fichier blacklist.txt (par défaut)
	struct filelist* blackList = (struct filelist *)malloc(256 * sizeof(struct filelist) );
	
	FILE * fichierBlackList = fopen("blacklist.txt", "r");
	if(fichierBlackList == NULL){
		printf("Problème de lecture du fichier BlackList \n");
	}

	int iter = 0;
	// lecture dans le fichier
	while(	fscanf(fichierBlackList, "%s ", blackList[iter].filename)!=-1 &&
		fscanf(fichierBlackList, "%d\n", &blackList[iter].checksum)!=-1){
		iter++;
	}

	// fermeture du fichier
	fclose(fichierBlackList);

	struct sockaddr_in domaineAF_INET;

	domaineAF_INET.sin_family=AF_INET;
	domaineAF_INET.sin_port= htons(atoi(argv[1]));		// Choix arbitraire du port (>5000 car déjà utilisés)
	domaineAF_INET.sin_addr.s_addr=htons(INADDR_ANY);	// Reçoit n'importe quelle adresse IP

	/* Création de socket */
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);	

	if(sock != -1)
	{
		//puts("Une socket a été crée avec succès\n");
	}
	else
	{
		/* Cas erreur création */
		perror("Erreur de création de la socket");
		exit(-1);
	}

	/* Attachement de la socket */
	if(bind(sock, (struct sockaddr *)&domaineAF_INET, sizeof(domaineAF_INET)) != -1) 
	{
		puts("Bind réalisé \n");
	}
	else
	{
		/* Cas erreur attachement */
		perror("Erreur bind");
		exit(-1);
	}

	/* Ouverture du service */
	if (listen(sock, 20)!=-1) // 20 = choix arbitraire
	{
		puts("En attente de connexion ... \n");
	}
	else
	{
		perror("Echec du passage en mode écoute");
		exit(-1);
	}


	/* Acceptation d'une connexion */
	int c;
	int socketClient;
	c = sizeof(struct sockaddr_in);
	
	while(1){	// Boucle d'acceptation

		// création de la socket de transfert
		socketClient = accept(sock, (struct sockaddr *)&domaineAF_INET, (socklen_t*)&c);
		if(socketClient != -1)
		{
			puts("Accept effectué\n");
		}
		else
		{
			puts("Erreur lors du accept\n");
			exit(-1);
		}

		/* Création d'un processus fils pour prendre le relai */
		pid_t ecoute = fork();
		if(ecoute == 0){

				// buffers d'entrée/sortie
				char message_envoi[1500] = "Bonjour vous êtes connectés";
				char message_recu[1500];


				while(strcmp(message_recu, "fin")!=0) {
				printf("envoi du message d'accueil... \n");
			
				if(write(socketClient, message_envoi, 1500) == -1){		// envoi message d'accueil
					perror("erreur d'envoi du message d'accueil... \n");
				}
				
				printf("attente client...\n");
				read(socketClient, message_recu,1500);
				printf("reponse client : %s \n",message_recu);

				// 1 = Lister les éléments sur Serveur
				if(strcmp(message_recu,"1")==0)	
				{
					
					strcpy(message_envoi, "Liste des images sur le serveur :\n");
					if(write(socketClient, message_envoi, 1500)==-1)
					{
						perror("erreur d'envoi de la liste... \n");
					}

					
					strcpy(message_envoi,"");	//vider pour ensuite concatener
					struct dirent *lecture;
					DIR *rep;
					rep = opendir("." );		// ouverture du répertoire

					// lecture des fichiers du répertoire
					while ((lecture = readdir(rep))) {
						if(strstr(lecture->d_name, ".pgm") != NULL
						 ||strstr(lecture->d_name, ".ppm") != NULL
						 ||strstr(lecture->d_name, ".pbm") != NULL)
						 {
							// on envoie la liste des fichiers dans un seul message
							strcat(message_envoi, lecture->d_name);
						 	strcat(message_envoi, "\n");
					  	 }					
					}
					
					write(socketClient, message_envoi, 1500);
					strcpy(message_envoi,"#stop#");
					sleep(1);	// silence inter trame
					write(socketClient, message_envoi, 1500);
					closedir(rep);	// fermeture du répertoire
				}


				// 2 = Télécharger un fichier Serveur -> Client
				else if (strcmp(message_recu,"2")==0) 
				{
					//read(socketClient, message_recu, 1500);
			
					printf("attends la reponse du client....\n");
					read(socketClient, message_recu, 1500);		//récéption nom du fichier

					// ouverture du fichier (lecture/ecriture)
					int f =open(message_recu,O_RDONLY, S_IWUSR | S_IRUSR);

					int nblu=1; // nombre de caractères lus 
					printf("filename saisi par le client %s \n",message_recu);
					if(f==-1)
					{
						// erreur ouverture du fichier
						perror("erreur ouverture fichier \n");
						exit(-1);
					}

					// on envoie le contenu du fichier
					nblu= read(f,message_envoi,1500);
					if(nblu==-1)
					{
						perror("erreur lecture fichier \n");
						exit(-1);
					}
					while(nblu!=0)
					{

						if(write(socketClient,message_envoi,nblu) == -1)
						{
							perror("erreur d'envoi du message... \n");
						}

						
						nblu= read(f,message_envoi,1500);
						if(nblu==-1)
						{
							perror("erreur lecture fichier \n");
							exit(-1);
						}
						
					}
					close(f);	// fermeture du fichier
					sleep(1);	// silence inter trame

					strcpy(message_envoi, "");		//envoie fin fichier
					write(socketClient,message_envoi,1);	

				}

				// 3 = Uploader un fichier Client -> Serveur
				else if(strcmp(message_recu,"3")==0)
				{
					int sum;
					read(socketClient, message_recu, 1500); // NOM DU FICHIER
					read(socketClient, &sum, sizeof(int)); // CHECKSUM

					printf("CHECKSUM : %d \n", sum);

					int kk;	// itérateur
					int is_ban = 0;// booleen (le fichier est-il blacklisté ?)
					for(kk =0; kk < iter; kk++){

						// comparaison avec le nom du fichier ET avec le checksum
						if(strcmp(blackList[kk].filename, message_recu)==0
						 ||blackList[kk].checksum==sum) {
							is_ban = 1;
						}
					}

					if(!is_ban){	// si le fichier est accepté
						char* rep="ok";
						int r = send(socketClient, rep, 2,0);	// confirmation ok
						char * filename = message_recu;

						// creation du fichier (lecture/écriture)
						int f = creat(filename,S_IWUSR | S_IRUSR);
						f = open(filename,O_RDWR); //ouverture du fichier (lecture)

						if(f==-1)
						{
							// erreur ouverture du fichier
							perror("Erreur creation du fichier...");
							exit(-1);
						}

						printf("Debut du telechargement...\n");

						// Lecture du fichier envoyé
						int nb= read(socketClient, message_recu, 1500);
						while(nb !=0 && strcmp(message_recu,"")!=0)
						{
							write(f,message_recu,nb);
							strcpy(message_recu,"");
							nb = read(socketClient, message_recu, 1500);
						}

						close(f);	// fermeture du fichier
						printf("Le téléchargement est fini. %s\n", filename);
					}
					else{	// si le fichier n'est pas accepté
						char *rep="ko";
						int r = write(socketClient, rep, 2);	// confirmation pas bon
					}

				}
				else{}
				strcpy(message_envoi, "");
				strcpy(message_recu, "");
				}
				close(sock);
			}
	}
	return 0;
}
