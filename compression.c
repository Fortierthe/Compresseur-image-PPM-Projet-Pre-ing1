#include "ppm_lib.h"

int verification_tableau(unsigned char tab1[], unsigned char tab2[]){		// Check if 2 pixels have identical RGB values
	if((tab1[0] == tab2[0]) &&
	(tab1[1] == tab2[1]) &&
	(tab1[2] == tab2[2])){	
		return 1;
	}
	return 0;
}

void same_tab(unsigned char tab1[], unsigned char tab2[]){	// Assigns to a pixel the RGB values of another pixel
	tab1[0] = tab2[0];
	tab1[1] = tab2[1];
	tab1[2] = tab2[2];	
}

void block_RGB(unsigned char pixel[], FILE * fichier){
	unsigned char valeur = 128+64+32+16+8+4+2;
	fwrite(&valeur,sizeof(unsigned char),1,fichier);	// Write first byte of the block EVA_BLK_RGB		
	fwrite(pixel,sizeof(unsigned char),3,fichier);		// Write the other 3 bytes of the block EVA_BLK_RGB
}

int block_diff_or_luma(unsigned char pixel[],unsigned char pixelPrecedent[], unsigned char difference[]){
	unsigned char diffr = pixel[0] - pixelPrecedent[0];	// calculates the red difference between previous pixel and current pixel
	unsigned char diffg = pixel[1] - pixelPrecedent[1];	// calculates the green difference between previous pixel and current pixel
	unsigned char diffb = pixel[2] - pixelPrecedent[2];	// calculates the blue difference between previous pixel and current pixel
	unsigned char diffrg = diffr - diffg;		// calculating the difference between red and green
	unsigned char diffbg = diffb - diffg;		// calculating the difference between blue and green
	if( ((diffr >= -2) && (diffr <= 1)) && 		// check if the RGB differences are in the range [-2;1]
	  ((diffg >= -2) && (diffg <= 1)) &&
	  ((diffb >= -2) && (diffb <= 1)) ){ 
		// ------------- DIFF ---------------------------
		difference[0] = diffr;
		difference[1] = diffg;
		difference[2] = diffb;
		return 1;
	}
	else if((diffg >= -32) && (diffg <= 31) &&	// Verification if green difference between [-32;31]
	(diffrg >= -8) && (diffrg <= 7) && 		// Verification if difference between red and green included in [-8;7]
	(diffbg >= -8) && (diffbg <= 7)){		// Verification if difference between blue and green included in [-8;7]
		// ------------- LUMA ---------------------------	
		difference[0] = diffrg;
		difference[1] = diffg;
		difference[2] = diffbg;
		return 2;		
	}
	return 0;
}



void compress(){	//----------START OF COMPRESSION----------------
	PPM_IMG* ppm;	
	FILE * fichier;
	unsigned char cache[64][3] =  {0};	// Creation of the cache of 64 pixel values stored as (0,0,0)
	int x,y,xMax,yMax, lu;
	unsigned char valeur, index, compteur;
	unsigned char pixel[3],pixelPrecedent[3], difference[3];
	int pixelOK;

    	char * nom, *nomPPM;
	
	
	do{
		printf("Quel est le nom de votre fichier .ppm à décompresser < nom > = ");
		nom = malloc(500 * sizeof(char));
		nomPPM = malloc(500 * sizeof(char));
		scanf("%s", nom);
		strcat(nomPPM,nom);
		strcat(nomPPM,".ppm");
		ppm = ppmOpen(nomPPM);
		yMax =  ppmGetHeight(ppm);	// yMax= image height
		xMax = ppmGetWidth (ppm);	// xMax= image width
		if((yMax == 0) || (xMax == 0)){
			free(nom);
			free(nomPPM);
		}
	}while((yMax == 0) || (xMax == 0));
	
	strcat(nom,"_compress.txt");
	fichier = fopen(nom, "wb+");
	if (fichier == NULL){
		printf("erreur %d",  errno  );
		printf("%s",  strerror(errno)  );
		exit(1);
	}	
	clock_t begin = clock();

	fwrite(&yMax,sizeof(int),1,fichier);
	fwrite(&xMax,sizeof(int),1,fichier);
	
	compteur = 0;
	for(y=0;y< yMax;y++){
		for(x=0;x<xMax;x++){
				
			lu = ppmRead(ppm,x,y);
			pixel[0]= red(lu);	// Reading and storing in the cache the red component of the first pixel
			pixel[1]= green(lu);	// Reading and storing in the cache the green component of the first pixel
			pixel[2]= blue(lu);	// Reading and storing in the cache the blue component of the first pixel
			index = (3*pixel[0] + 5*pixel[1] +7*pixel[2])%64;			

			pixelOK = 0;

			if((x==0) && (y==0)){
				// ------------- FIRST ---------------------------
				block_RGB(pixel, fichier);	// Writing in the cache of the first pixel in the form (R,G,B)
				
				pixelOK = 1;
			}			
			else if((verification_tableau(pixelPrecedent,pixel)) && (compteur<=61)){	// Case where previous pixel is identical to current pixel and number of identical pixel <=62
				// ------------- INC SAME counter ---------------------------
				compteur++;
				pixelOK = 1;
			}
			else if(compteur != 0){		// Case where previous pixel is different from current pixel
				// ------------- WRITE SAME ---------------------------

				compteur += 128 + 64 -1;
				fwrite(&compteur,sizeof(unsigned char),1,fichier);	// Writing the EVA_BLK_SAME block
				compteur = 0;

				// ok we have processed the previous SAME block : now process the current pixel
			}
		


			if(pixelOK == 1){
				// do nothing or refactor the code noob !!!
			}
			else if	(verification_tableau(cache[index], pixel)){	// Check if the current pixel has already been seen in the cache at location index
				// ------------- INDEX ---------------------------	// If yes
		
		
				fwrite(&index,sizeof(unsigned char),1,fichier);		// Writing of the EVA_BLK_INDEX block
			}	
			else{ 	// If no
				int diff_or_luma = block_diff_or_luma(pixel,pixelPrecedent, difference);
				if( diff_or_luma == 1){ 	//If BLOC DIFF
					// ------------- DIFF ---------------------------
			
					valeur = 64 + (16*(difference[0]+2)) + (4*(difference[1]+2)) + difference[2]+2;		// Value of the EVA_BLK_DIFF block
					fwrite(&valeur,sizeof(unsigned char),1,fichier);	// Write block EVA_BLK_DIFF
				}
				else if(diff_or_luma == 2){	//If BLOC LUMA
					// ------------- LUMA ---------------------------

					
					valeur = 128 +difference[1]+32;
					fwrite(&valeur,sizeof(unsigned char),1,fichier);	// Writing the first LUMA block of 1 byte 
					valeur = (difference[0]+8)*16 + difference[2]+8;
					fwrite(&valeur,sizeof(unsigned char),1,fichier);	// Writing the second LUMA block of 1 byte
					
				}
				else{
					// ------------- RGB ---------------------------
					block_RGB(pixel, fichier);	// Write BLOCK RGB
			
				}
			}
			same_tab(cache[index],pixel);
			same_tab(pixelPrecedent, pixel);	// Allows to give to previous pixel the values of the current pixel
		}
	}
	if(compteur != 0){
		compteur += 128 + 64 -1;
		fwrite(&compteur,sizeof(unsigned char),1,fichier);
	}	
	clock_t end = clock();
   	printf(" Compilation terminée en  %ld ms. \n", (end -  begin) * 1000 / CLOCKS_PER_SEC);	// Compression time 
	fclose(fichier);
	ppmClose(ppm);
	taux_compression(nomPPM, nom);
	free(nom);
	free(nomPPM);			// COMPRESSION END
}

