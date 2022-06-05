all : exec

main.o : main.c ppm_lib.h
	gcc -c main.c -o main.o 

stats.o : stats.c ppm_lib.h
	gcc -c stats.c -o stats.o
	
compression.o : compression.c ppm_lib.h
	gcc -c compression.c -o compression.o 
	
decompression.o : decompression.c ppm_lib.h
	gcc -c decompression.c -o decompression.o	
	
ppm_lib.o : ppm_lib.c ppm_lib.h
	gcc -c ppm_lib.c -o ppm_lib.o 	

exec : main.o ppm_lib.o compression.o decompression.o stats.o
	gcc main.o ppm_lib.o compression.o decompression.o stats.o -o exec
	
clean : 
	rm -f *.o
	rm exec
	
	

