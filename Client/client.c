#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include "controle.h"


int main (int argc, char *argv[])
{
	/*******************CLIENT****************/

	char nomMachine[] = "localhost";
	
	// déclaration de la socket serveur
	struct sockaddr_in serveur = { 0 };

	serveur.sin_family=AF_INET;
	serveur.sin_port= htons(atoi(argv[1]));
	
	// déclaration de la socket client
	struct sockaddr_in domaineAF_INET;
	
	domaineAF_INET.sin_family=AF_INET;
	domaineAF_INET.sin_port= htons(0); // car client
	domaineAF_INET.sin_addr.s_addr=htons(INADDR_ANY); // recoit n'importe quelle adresse ip
	int i;
	for(i = 0; i< 8; i++){
		domaineAF_INET.sin_zero[i] = 0;	// remplissage du champs de 0
	}
	
	// creation soket
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock==-1)	//echec creation de la socket
	{
		perror("erreur de creation de la socket");
		exit(-1);
	}
	else //creation de la socket reussie 
	{
		printf("Socket client creee \n");
	}
	
	
	//attachement de la socket 
	int b = bind(sock, (struct sockaddr *)&domaineAF_INET, sizeof(domaineAF_INET));

	if(b==-1)//cas erreur
	{
		perror("erreur de creation du bind");
		exit(-1);
	}
	

	//construction de l'adresse du serveur	
	struct hostent* host = gethostbyname(nomMachine);
	
	gethostname(nomMachine, 4);
	

	if(host==NULL)//erreur avec le serveur
	{
		perror("erreur construction de l'adresse du serveur");
		exit(-1);
	}
	
	// copie de l'adresse serveur
	memcpy(&serveur.sin_addr.s_addr, host->h_addr, host->h_length);
	
	
	//demande de connexion
	int c = connect(sock,(struct sockaddr *)&serveur, sizeof(domaineAF_INET));
	if(c==-1)	//cas erreur
	{
		perror("erreur de connexion");
		exit(-1);
	}
	printf ("Attente du serveur \n");

	// buffers d'entrée/sortie
	char message_recu[1500];
	char message_envoi[1500];

	if( read(sock, message_recu, 256) == -1 ){	// reception du message d'accueil
		printf("recv 1 : %s\n", message_recu);
		perror("Erreur de lecture du message d'accueil");
		exit(-1);
	}
	printf ("Message Recu : %s \n", message_recu);	// affichage du message d'accueil

	while(strcmp(message_recu, "fin") != 0){	
	
		// affichage du menu
		printf("1 : Lister les fichiers \n");
		printf("2 : Recuperer un fichier (Download) \n");
		printf("3 : Charger un fichier (Upload) \n");
		printf("fin : Fermer la connexion \n");
		scanf("%s", message_envoi);
		write(sock, message_envoi, 1500);	// lecture de la commande choisie
		

		// 1 = Lister les éléments sur Serveur
		if(strcmp(message_envoi,"1")==0){	
			printf("serveur envoie...\n");
			read(sock, message_recu, 1500);	//recoit debut liste
			while(strcmp(message_recu ,"#stop#")!=0){
	
				if(strcmp(message_recu ,"")!=0 && strcmp(message_recu ,"#stop#")!=0){
					printf("%s\n", message_recu);
				}
				read(sock, message_recu, 1500);
			
			}
		}

		// 2 = Télécharger un fichier Serveur -> Client
		else if(strcmp(message_envoi,"2")==0)	
		{
			read(sock, message_recu, 1500);
			printf("Saisir le nom du fichier \n");	// -> saisir le nom du fichier
			scanf("%s",message_envoi);
			write(sock,message_envoi,1500);		// envoi du nom du fichier

			char * filename = message_envoi;
			int f = creat(filename,S_IWUSR | S_IRUSR); // création du fichier (droits lecture/écriture)
			f = open(filename,O_RDWR);		   // ouverture du fichier (lecture)

			// cas erreur
			if(f==-1)
			{
				perror("Erreur creation du fichier...");
				exit(-1);
			}

			printf("Debut du telechargement...\n");

			int nb= read(sock, message_recu, 1500);	// lecture du fichier sur serveur
		
			while(nb !=0 && strcmp(message_recu,"")!=0)
			{
				write(f,message_recu,nb);		// écriture des données sur le fichier client
				strcpy(message_recu,"");		// vidage du buffer
				nb = read(sock, message_recu, 1500);
			}
			close(f);					// fermeture du fichier
			printf("Le téléchargement est fini. %s\n", filename);
		}

		// 3 = Uploader un fichier Client -> Serveur
		else if(strcmp(message_envoi,"3")==0)
		{

			printf("Donnez le nom de fichier : \n");
			scanf("%s", message_envoi);		// -> saisir le nom du fichier
			int sum = checksum(message_envoi);	// calcul du checksum (vérification)

			write(sock,message_envoi,1500);		// envoi du nom du fichier
			write(sock, &sum, sizeof(int));	// envoi du checksum
			char* confirmation = (char*)malloc(3*sizeof(char)); 

			int r = read(sock, confirmation, 2);	// lecture de la confirmation (ok ou ko)
			
			//while(strcmp(confirmation, "") ==0){
			//	r = recv(sock, confirmation, 2,0);
			//}
			//printf("BORDEL ! : #%s# \n ET : %d \n", confirmation, r);
			
			if(strcmp(confirmation, "ko") == 0){	// fichier non admis (ko)
				printf("Le fichier que vous avez envoyé n'est pas admissible. \n");
			}
			else{					// fichier admis (ok)
				printf("Le fichier est admis \n");

				// ouverture du fichier correspondant (lecture/écriture)
				int f =open(message_envoi,O_RDONLY, S_IWUSR | S_IRUSR);

				int nblu=1;	// nombre de caractères lus
				if(f==-1)
				{
					// cas erreur ouverture fichier
					perror("erreur ouverture fichier \n");
					exit(-1);
				}
				nblu= read(f,message_envoi,1500);	// on lit le fichier sur client
				if(nblu==-1)
				{
					// problème de lecture
					perror("erreur lecture fichier \n");
					exit(-1);
				}
				while(nblu!=0) 
				{
					// écriture du fichier sur serveur
					if(write(sock,message_envoi,nblu) == -1)
					{
						// cas erreur écriture
						perror("erreur d'envoi du message... \n");
					}
					nblu= read(f,message_envoi,1500);
					if(nblu==-1)
					{
						// cas erreur lecture fichier
						perror("erreur lecture fichier \n");
						exit(-1);
					}
				}
				close(f);			// fermeture du fichier	
				sleep(1);			// silence inter trame
				strcpy(message_envoi, "");	// vidage du buffer
				write(sock,message_envoi,1);	//envoie fin fichier
			}
			free(confirmation);	// libération de l'espace occupé par la chaine de confirmation
		}

		// fin = Fermer la connexion 
		else if(strcmp(message_envoi,"fin")==0){
			printf("fermeture de la connexion\n\n");
			exit(0);
		}

		// vidages des buffers
		strcpy(message_envoi, "");
		strcpy(message_recu, "");
	}
	// fermeture de la connexion
	close (sock);

	return 0;

}
