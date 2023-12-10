#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include "functions.h"
int main(int argc, char** argv){
	if(argc<2){
		printf("Error: improper command!\n");
		return -1;
	}
	if (strcmp(argv[1], "-b") == 0) {
		char* saufilename = "a.sau";
		bool oopt = (strcmp(argv[argc-2],"-o") == 0);
		int numfile = argc-2;
		
		if(oopt){
			saufilename = argv[argc-1];
			strcat(saufilename,".sau");
			numfile -= 2;
		} 
		if(numfile > 32){
			printf("Error: This archiver can only archive 32 files at a time. Please reduce the number of files.\n");
			return -1;
		}
		for(int i = 2; i<numfile+2;i++){
			if(isTextFile(argv[i]) == 0){
				printf("Error: %s input file format is	incompatible! \n",argv[i]);
				return -1;
			}
		}
		int totalSize = 0;
		for(int i = 2; i<numfile+2;i++){
			totalSize += getSize(argv[i]);
		}
		if(totalSize > (200*1024*1024)){
			printf("Error: The data that you want to archive is larger than 200MiB\n");
			return -1;
		}
		for(int i = 2; i<numfile+2;i++){
			createHeader(argv[i],saufilename); 
		}
		FILE *file = fopen(saufilename,"a+");fprintf(file,"\n");fclose(file); //spacing after header info

		for(int i = 2; i<numfile+2; i++){
			readAndWrite(argv[i],saufilename); //write data into .sau file
		}
    } else if (strcmp(argv[1], "-a") == 0) {
        
    } else {	
        printf("Error: There is no such an option in tarsau command! --> -b for archiving -a for extracting\n");
    	return -1;
    }
	return 0;
}
