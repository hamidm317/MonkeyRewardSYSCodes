#include "picture.h"
#include <time.h>
#include <math.h>

void randGen(int len, int *arr, int isRand){
int *p = arr;

int tempNums[len];
                for (int i=0; i<len; i++)
                    tempNums[i] = i;
		if (isRand){
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
		for (int i=0; i<len; i++){
		    *(p++) = tempNums[i];
		}
}