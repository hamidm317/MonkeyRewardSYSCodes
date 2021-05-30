#include "picture.h"
#include <time.h>
#include <math.h>
int goodFractRegionPerm[numberOfBatchFractals];

void randGen(int len, int *arr, randomPermutationTypeEnum randType){
	int *p = arr;
	int tempNums[len];
	//Stage 1 : Fill The array
	if (randType == Random_Permutation_Type_Sorted ||
	    randType == Random_Permutation_Type_Shuffle){
      		for (int i=0; i<len; i++){
			tempNums[i] = i;
		}
	}
	else if (randType == Random_Permutation_Type_Fract_Idx_Good){
		int j=0;
		for (int i=0; i<2*len; i++){
			if (i%8 >= 4)
				continue;
			else 
				tempNums[j++] = i%24;
		}
	}
	else if (randType == Random_Permutation_Type_Fract_Idx_Bad){
		int j=0;
		for (int i=0; i<2*len; i++){
			if (i%8 < 4)
				continue;
			else 
				tempNums[j++] = i%24;
		}
	}
	else if (randType == Random_Permutation_Type_Trial_Type_Targert_Wise){
      		for (int i=0; i<len; i++){
			tempNums[i] = i%2;
		}
	}
	else if (randType == Random_Permutation_Type_Fractal_Number){
      		for (int i=0; i<len; i++){
			tempNums[i] = 2*(i%4)+3;
		}
	}
	/*
	else if (randType == Random_Permutation_Type_Good_Fractal_Region){
      		for (int i=0; i<len; i++){
			tempNums[i] = (rand()%fractalNumPerm[i]) + 1;
		}
	}*/

	//Stage 2: Do the suffling
	if (randType != Random_Permutation_Type_Sorted){
		for (int i = 0; i < len - 1; i++)
                {
                    int j = i + rand() % (len - i);
		    {
			int temp = tempNums[i];
			tempNums[i] = tempNums[j];
			tempNums[j] = temp;
		    }
                }
	}
	
	//Stage 3: Output the array
	for (int i=0; i<len; i++){
		if (randType == Random_Permutation_Type_Fract_Idx_Bad){
			//printf("badFractIdx[%d] = %d (%8 = %d)\n", i, tempNums[i], tempNums[i]%8);
		}
		/*
		if (randType == Random_Permutation_Type_Good_Fractal_Region){
			printf("goodfractregion[%d] = %d\n", i, tempNums[i]);
		}
		if (randType == Random_Permutation_Type_Fractal_Number){
			printf("fractnum[%d] = %d\n", i, tempNums[i]);
		}*/
	    *(p++) = tempNums[i];
	}
}
