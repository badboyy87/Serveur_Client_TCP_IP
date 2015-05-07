#include <stdio.h>
#include <stdlib.h>
#include "controle.h"
#include <string.h>
#include <time.h>
#include <math.h>


/* Convertir un nombre entier en chaine de caractères représentant son binaire
 * Entrée : x - l'entier en question
 *	  : n - taille de la chaine
 * Sortie : une chaine de caractères de taille n
 */
char* convert_vers_bin(int x, int n)
{
    char * tab;
    int i;
    tab = (char*)malloc(n * sizeof(char));
    for(i= n - 1; i >= 0; i--)
    {
    
        if (x%2==0) 
        {
        	tab[i]='0';
        } 
        else 
        { 
            tab[i] ='1';
            x = x - 1;
        }
    x = x / 2;
    }
    return tab;
}


/* Convertir une chaine de caractères représentant un binaire vers un nombre entier
 * Entrée : v - la chaine de caractères à transformer
 *	  : n - taille de la chaine
 * Sortie : un entier 
 */
int convert_vers_dec(char v[24], int n)
{ 
	int i =0;
	int pred =0;	

	for(i = 0; i<n; i++)
	{
		pred += v[i]-48 << (n-i-1);
	} 
	return pred;
}


/* Calcule le checksum d'un fichier à partir de son nom
 * Entrée : filename - le fichier à évaluer
 * Sortie : un entier représentant le checksum du fichier
 */
int checksum(char filename[25]){

	int largeur,hauteur,max_valeur;
	char type[2];

	int i=0,j=0;
	int b,v,r;
	char c;

	int sum = 0; 

	// lecture du fichier
	FILE * fl = fopen(filename, "r");
	fscanf(fl,"%s",type);
 	fscanf(fl,"%d",&largeur);
 	fscanf(fl,"%d",&hauteur);
 	fscanf(fl,"%d",&max_valeur);

	// remplissage du tableau
	struct pixel pixels[largeur][hauteur];


	for(i=0;i<hauteur;i++){

		for(j=0;j<largeur;j++) {

	 		fscanf(fl,"%d",&b);
			fscanf(fl,"%d",&v);
			fscanf(fl,"%d",&r);  

			pixels[i][j].bleue=b;
			pixels[i][j].verte=v;
			pixels[i][j].rouge=r;	

		}
  	}
	
	// traitement du checksum
	srand(0);	// initialisation (changer germe)
	int seed;	// element de la suite pseudo-aléatoire


	// table qui représente une couleur de pixel (8 bits par couleur x 3 couleurs)
	char* tab = (char *)malloc(24 * sizeof(char));
	int it;
	for(it = 0; it<100; it++){
		seed = rand() % (largeur*hauteur);
		

		i = seed%largeur;
		j = (int)(seed/largeur);

		tab = convert_vers_bin(pixels[i][j].rouge, 8);
		strcat(tab, convert_vers_bin(pixels[i][j].verte, 8));
		strcat(tab, convert_vers_bin(pixels[i][j].bleue, 8));

		sum += convert_vers_dec(tab, 24);

	}
	return sum;
}

/* Transforme un fichier ppm en fichier pgm
 * Entrée : filename - le fichier à transformer
 * Pas de sortie. Le fichier est transformé directement.
 */
void ppmtopgm(char filename[25])
{
	
	int largeur,hauteur,max_valeur;
	char type[2];

	int i=0,j=0;
	int b,v,r;
	char c;

	// lecture du fichier
	FILE * fl = fopen(filename, "r");
	fscanf(fl,"%s",type);
 	fscanf(fl,"%d",&largeur);
 	fscanf(fl,"%d",&hauteur);
 	fscanf(fl,"%d",&max_valeur);

	struct pixel pixels[largeur][hauteur];

	for(i=0;i<hauteur;i++)
		for(j=0;j<largeur;j++) {
	 		fscanf(fl,"%d",&b);
			fscanf(fl,"%d",&v);
			fscanf(fl,"%d",&r);  
    
			pixels[i][j].bleue=b;
			pixels[i][j].verte=v;
			pixels[i][j].rouge=r;	
  	} 

	// Conversion et écriture du fichier
	filename[strlen(filename)-2] = 'g';

	FILE *flw  = fopen(filename,"w"); 
	fprintf(flw,"%s\n","P2");
	fprintf(flw,"%d %d\n",largeur,hauteur);
	fprintf(flw,"%d \n",max_valeur);
	for(i=0;i<hauteur;i++) {
		for(j=0;j<largeur;j++) { 
			// traintement et conversion de la valeur des trois couleurs 
			int p=0.299*pixels[i][j].bleue+0.587*pixels[i][j].verte+0.114*pixels[i][j].rouge; 
			fprintf(flw,"%d ",p);
	      	}
		fprintf(flw,"\n");
	}

	// fermeture des fichiers
	fclose(fl);
	fclose(flw);

}
