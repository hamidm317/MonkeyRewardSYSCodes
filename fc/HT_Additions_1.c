#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "picture.h"
#ifdef WIN32
#include <w32_dialogs.h>
#endif

int GOOD_REWARD = 100;
int BAD_REWARD = 0;

void HT_OSRU_fractalLoaderFromSet(int set, int numberOfFractPerTrial){
        //101 , 102
        //set -= 100;
        int fractSetIdx = 1000 + (numberOfFractPerTrial * (set - 100));
        char temp[100];
        char idxToStr[100];

        for (int i = 0; i < numberOfFractPerTrial; i++){
                strcpy(temp, "/home/lab/Fractals/i");
                sprintf(idxToStr, "%d", fractSetIdx + i);
                strcat(temp, idxToStr);
                strcat(temp, ".jpeg");
				printf("loaded fractal = %s\n", temp);
                Surface_Fractals[i] = IMG_Load(temp);
                Texture_Fractals[i] = SDL_CreateTextureFromSurface(renderer2, Surface_Fractals[i]);
                SDL_SetTextureBlendMode(Texture_Fractals[i], SDL_BLENDMODE_BLEND);
                Texture_Fractals_Monkey[i] = SDL_CreateTextureFromSurface(renderer, Surface_Fractals[i]);
                SDL_SetTextureBlendMode(Texture_Fractals_Monkey[i], SDL_BLENDMODE_BLEND);

        }
}

int * fractalOptLabeller(int TypeTempVar, int numberOfFractPerTrial)
{
    
    double FractalLabels[50]; // First element contains number of fractal sets, second element is type of labeling, and the last non Null element is -1 (so maximum number of sets is 47).

    FractalLabels[0] = numberOfFractPerTrial;
    FractalLabels[1] = TypeTempVar;
    FractalLabels[numberOfFractPerTrial] = -1;

    switch (TypeTempVar)
    {
        case 1: //Type is Binary Labeling and we set "Good" to first half and "Bad" to the others.
            for(int i = 0; i < numberOfFractPerTrial ; i++)
            {
                FractalLabels[i + 2] = (i < numberOfFractPerTrial / 2) ? 100 : 0;
            }
            break;

        case 2: //Type is amount labeling and we set a range of 0 to 100 for this labels.
            for(int i = 0; i < numberOfFractPerTrial ; i++)
            {
                FractalLabels[i + 2] = i / numberOfFractPerTrial * 100;
            }
            break;

        case 3: //Type is probable labeling and we set a range of 0 to 100 for this labels. (Difference of this part and previous part is in rewarding part)
            for(int i = 0; i < numberOfFractPerTrial ; i++)
            {
                FractalLabels[i + 2] = i / numberOfFractPerTrial * 100;
            }
            break;
        default:;
            break;
    }
    
    return FractalLabels;
}

void Rewarder(double FractLabel, int Type)
{
    
    int RewardAmount = 0;
    switch (Type)
    {
        case 1: //Type is Binary Labeling
            RewardAmount = FractLabel;
            break;

        case 2: //Type is amount labeling
            RewardAmount = FractLabel;
            break;

        case 3: //Type is probable labeling
            
            LuckNumber = RandGenerator()
            if (LuckNumber > FractLabel)
            {
                RewardAmount = GOOD_REWARD;
            }
            else
            {
                RewardAmount = BAD_REWARD;
            }
                
            break;
        default:;
            break;
    }

    RewardSender(RewardAmount);

}

void RewardSender(RewardAmount) // a message for eyelink should be sent and a message for Arduino
{

    

}

double RandGenerator()
{

    srand(time(NULL))
    return (double)rand() / (double)RAND_MAX;

}

// strcpy(Type1, "Binary_Labeling")
// strcpy(Type2, "Amount_Labeling")
// strcpy(Type3, "Probable_Labeling")

int ProbablisticModeSTPoint = 300;
int ProbablisticModeLen = 100;

int AmountModeSTPoint = 200;
int AmountModeLen = 100;

int BinaryModeSTPoint = 120;
int BinaryModeLen = 80;

void labelRanging(int set)
{
    if (set > BinaryModeSTPoint && set =< BinaryModeSTPoint + BinaryModeLen)
    {
        strcpy(LabelingType, "Binary_Labeling")
    }
    else
    {
        if (set > AmountModeSTPoint && set =< AmountModeSTPoint + AmountModeLen)
        {
            strcpy(LabelingType, "Amount_Labeling")
        }
        else
        {
            if (set > ProbablisticModeSTPoint && set =< ProbablisticModeSTPoint + ProbablisticModeLen)
            {
                strcpy(LabelingType, "Probable_Labeling")
            }
        }
        
    }
    
}