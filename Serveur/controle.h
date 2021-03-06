#define N 16






/* Convertir un nombre entier en chaine de caractères représentant son binaire
 * Entrée : x - l'entier en question
 *	  : n - taille de la chaine
 * Sortie : une chaine de caractères de taille n
 */
char *convert_vers_bin(int x, int n);

/* Convertir une chaine de caractères représentant un binaire vers un nombre entier
 * Entrée : v - la chaine de caractères à transformer
 *	  : n - taille de la chaine
 * Sortie : un entier 
 */
int convert_vers_dec(char v[], int n);


/* Calcule le checksum d'un fichier à partir de son nom
 * Entrée : filename - le fichier à évaluer
 * Sortie : un entier représentant le checksum du fichier
 */
int checksum(char filename[25]);


/* Transforme un fichier ppm en fichier pgm
 * Entrée : filename - le fichier à transformer
 * Pas de sortie. Le fichier est transformé directement.
 */
void ppmtopgm(char filename[25]);

/* structure définissant un pixel
 * bleue : valeur de l'intensité du bleu
 * verte : valeur de l'intensité du vert
 * rouge : valeur de l'intensité du rouge
 */
struct pixel {
    	int bleue,verte,rouge;
 	};
