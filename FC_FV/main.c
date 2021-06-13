#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

/* Rand tester
int main(){
	srand(time(0));
	for (int i=0; i<6; i++){
		int fractIdxs[4];
		int regionType;
		FVPerm(4,&fractIdxs,&regionType);
		printf("Batch[%d]:\n",i);
		for (int j=0; j<4;j++){
			printf("Fract ID = %d, Region Type = %d\n",
			fractIdxs[j], regionType);
		}
	}
	return 0;
}
*/

int main(){

	reloadConf();
	return 0;
}


int reloadConf(){
	FILE *fp;
	char str[200];
	static int currentDelimitedPosFC = 0;
	static int currentDelimitedPosFV = 0;

	fp = fopen("FVConf.txt", "r");
	if (!fp){
		printf("Failed to open file");
		return -1;
	}
	while (fgets(str, 200, fp)){
		if (strstr(str, "FCSequence")!= NULL){
			char parsedStr[200];
			strcpy(parsedStr,strstr(str, "=") + 1);
			//int parsedInt;
			//parsedInt = atoi(strstr(str, "=") + 1);
			printf("parsed str = %s\n", parsedStr);
			char *pt;
			pt = strtok(parsedStr, " ,");
			printf("parsed token = %s\n", pt);
			while (pt != NULL){
				pt = strtok(NULL, ", ");
				printf("parsed token = %s\n", pt);
			}
		}
		/*
		if (strstr(str, "FVSequence")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			printf("iti: %d\n", parsedInt);
		}*/

	}
	fclose(fp);
	//reloadedConf = 1;
	return 1;
}


