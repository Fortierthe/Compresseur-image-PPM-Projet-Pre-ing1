#include "ppm_lib.h"

void taux_compression(char* nom_ppm, char* nom_txt) {
    struct stat sb;
    char *filename = NULL;
    double ratio, ppm,txt;

    filename = nom_ppm; 	// PPM Image Data Recovery

	if (stat(filename, &sb) == -1) {
		    perror("stat");
		}
    ppm = sb.st_size;		// Retrieving the number of bytes in the file
	
    
    filename = nom_txt;		// Compressed image data recovery

    if (stat(filename, &sb) == -1) {
        perror("stat");
    }
    txt = sb.st_size;		// Retrieving the number of bytes in the file

	ratio = txt/ppm  ;	// calculation of the compression ratio

	printf("Taux de compression = %lf \n", ratio );		// compression ratio display
}

