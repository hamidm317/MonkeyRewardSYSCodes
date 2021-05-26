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
			
			/*FILE * fL;
			char logger[20];

			sprintf(logger, "%d\n", tempNums[i]);

    			fL = fopen("RandomLogger.txt","a");
	
    			fprintf (fL, logger);
		
			fclose (fL);*/
                 }
		}
		
	/*FILE * fL;
	char logger[20];
	
	fL = fopen("RandomLogger.txt","a");
	
    	fprintf (fL, "\n\n\n\n\n");
		
	fclose (fL);*/

	/* for (int i = 0; i < len; i++)
	{

		tempNums[i] = rand() % 5;

	} */

	for (int i=0; i<len; i++)
	{
		*(p++) = tempNums[i];
	}
	

}

int RandGenerator()
{

	srand(time(NULL));
	int temp;
	temp = rand() % 4;
	printf("\n Rand mod 4 is: %d \n", temp);		
	return temp + 1;

}
