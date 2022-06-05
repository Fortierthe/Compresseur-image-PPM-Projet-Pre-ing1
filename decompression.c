#include "ppm_lib.h"

void compteur(int *x, int* y, int largeur){
	(*x) = (*x) +1;
	
	if((*x)>=largeur){
		(*x) = 0;
		(*y) = (*y) + 1;	
	}
}

int new_block_RGB(PPM_IMG* ppm,int x,int y,FILE * fichier, int cache[]){
	unsigned char rouge,vert,bleu, index;
	int pixel;
	fread(&rouge,sizeof(unsigned char),1,fichier);
	fread(&vert,sizeof(unsigned char),1,fichier);
	fread(&bleu,sizeof(unsigned char),1,fichier);
	
	ppmWrite(ppm, x, y, pixel(rouge,vert,bleu));		// Writing the output pixel according to its RGB components
	pixel = pixel(rouge,vert,bleu);
	index = (3*rouge + 5*vert +7*bleu)%64;
	cache[index] = pixel;
	return pixel;
	
	
}

int new_block_DIFF(PPM_IMG* ppm, unsigned char oct, int cache[], int x,int y,FILE * fichier, int pixelPrecedent){ 	
	int pixel, index;
	oct = oct -64;				
	int diffr = (oct/16)-2;		// Red difference recovery on bits 4 and 5 of the DIFF block
	oct = oct%16;
	int diffg = (oct/4) - 2;	// Recovery of green difference on bits 2 and 3 of the DIFF block
	oct = oct%4;
	int diffb = oct - 2;		// Recovery of blue difference on bits 0 and 1 of the DIFF block
	pixel = pixel(red(pixelPrecedent)+diffr,green(pixelPrecedent)+diffg,blue(pixelPrecedent)+diffb);	// Adding color differences to pixel components
	index = (3*red(pixel) + 5*green(pixel) +7*blue(pixel))%64;
	cache[index] = pixel;
	ppmWrite(ppm, x, y, pixel);
	return pixel;
}

int new_block_LUMA(PPM_IMG* ppm, unsigned char oct, int cache[], int x,int y,FILE * fichier, int pixelPrecedent){ 
	int pixel, index;	
	int diffg = oct - 128 - 32;		// Recovery on the bits from 0 to 5 of the first LUMA block the difference of green by removing the offset to +32
	fread(&oct,sizeof(unsigned char),1,fichier); 

	int diffr = (oct/16)-8+diffg;		// Recovery on bits 4 to 7 of the second LUMA block the difference of red by removing the offset at +8
	oct = oct%16;
	int diffb = oct-8+diffg;	// Recovery on bits 0 to 3 of the second LUMA block the difference of blue by removing the offset at +8

	pixel = pixel(red(pixelPrecedent)+diffr,green(pixelPrecedent)+diffg,blue(pixelPrecedent)+diffb);	// Adding RGB differences to the RGB components of the pixel
	index = (3*red(pixel) + 5*green(pixel) +7*blue(pixel))%64;
	cache[index] = pixel;
	ppmWrite(ppm, x, y, pixel);
	return pixel;
}
void decompress(){ 		// ---------------START DECOMPRESSION---------------

	PPM_IMG* ppm;	
	FILE * fichier;
	int cache[64] = {0};
	int x,y,xMax,yMax, pixelPrecedent, pixel;
	unsigned char oct,index;
	char * nom, *nomPPM;
	
	
	do{		// Loop to request compressed image name
		printf("Quel est le nom de votre fichier : <name>_compress.txt à décompresser < nom > = ");
		nom = calloc(500,sizeof(char));
		nomPPM = calloc(500,sizeof(char));
		scanf("%s", nomPPM);
		strcat(nom,nomPPM);
		strcat(nom,"_compress.txt");
		fichier = fopen(nom, "rb");
		if(fichier == NULL){
			free(nom);
			free(nomPPM);
		}
				
	}while(fichier == NULL);
	clock_t begin = clock();
	yMax =  0;
	xMax = 0;
	pixelPrecedent = 0;
	fread(&yMax,sizeof(int),1,fichier);	// Image height retrieval
	fread(&xMax,sizeof(int),1,fichier);	// Image width retrieval
	x = 0;
	y = 0;
	ppm = ppmNew(xMax,yMax, 255, 3);
	
	do{	
			pixel = 0;
			fread(&oct,sizeof(unsigned char),1,fichier); 
			
			if(oct == 254){		// If the byte = 11111110 -> detection of the FIRST BLOCK EVA_BLK_RGB
				// ------------- RGB ---------------------------    
				pixel = new_block_RGB(ppm, x,y, fichier, cache);	// Write current pixel
				compteur(&x, &y, xMax);
				
			}
			else if((oct/64) == 3){		// If the 2 strong point bits = 11 -> detection BLOC EVA_BLK_SAME
				// ------------- WRITE SAME ---------------------------  
				pixel = oct - (128 + 64) +1;
				for(int i = 0; i < pixel; i++){		// Store in output the value of the previous pixel "pixel" times
					ppmWrite(ppm, x, y,pixelPrecedent);
					compteur(&x, &y, xMax);
					
				}
				pixel = pixelPrecedent;
			}
			else if((oct/64) == 0){		// If the 2 high point bits = 00 -> detection BLOC EVA_BLK_INDEX
				// ------------- INDEX ---------------------------   
		
				pixel = cache[oct];		// We retrieve the value of the pixel in the cache
				ppmWrite(ppm, x, y, pixel);	// Output storage of the pixel value
				compteur(&x, &y, xMax);	
				
			}
			else if((oct/64) == 1){		// If the 2 strong point bits = 01 -> detection BLOC EVA_BLK_DIFF
				// ------------- DIFF ---------------------------   

				pixel = new_block_DIFF(ppm, oct, cache, x, y, fichier, pixelPrecedent);		// Get red, green, blue difference from current pixel and add these values to previous pixel
				compteur(&x, &y, xMax);
				
			}
			else if((oct/64) == 2){		// If the 2 bits of strong points = 10 -> detection BLOC EVA_BLK_LUMA
				// ------------- LUMA   ---------------------------  

				pixel = new_block_LUMA(ppm, oct, cache, x, y, fichier, pixelPrecedent);		// Output storage of new pixel value
				compteur(&x, &y, xMax);
				
			}
			pixelPrecedent = pixel;		// Previous pixel takes the value of the current pixel before moving to the next pixel
		
	}while((y <=yMax-1) && ( x <=xMax-1) );
			// ---------------END DECOMPRESSION---------------
	
	fclose(fichier);
	strcat(nomPPM,".ppm");
	ppmSave(ppm,nomPPM);
	clock_t end = clock();
   	printf("Décompilation terminée en %ld ms. \n", (end -  begin) * 1000 / CLOCKS_PER_SEC);		// Duration of decompression
	ppmClose(ppm);
	taux_compression(nomPPM, nom);
	free(nom);
	free(nomPPM);
}
