#include "ppm_lib.h"

int main(){
	int choix;
	do{
		printf("Voulez-vous compresser ou décompresser une image ppm ? \nTapez 1 pour compresser et 2 pour décompresser \nSi vous souhaitez sortir du programme tapez un autre chiffre.\nChoix = ");
		scanf("%d", &choix);
		if(choix == 1){
			compress();	
		}
		else if (choix == 2){
			decompress();
		}
	}while((choix == 1) && (choix == 2) );
	
	return 0;
	
}


















































