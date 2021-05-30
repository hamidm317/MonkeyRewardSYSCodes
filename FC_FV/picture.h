/*****************************************************************************
 * Copyright (c) 1997 - 2003 by SR Research Ltd., All Rights Reserved        *
 *                                                                           *
 * This software is provided as is without warranty of any kind.  The entire *
 * risk as to the results and performance of this software is assumed by the *
 * user. SR Research Ltd. disclaims all warranties, either express or implied*
 * ,including but not limited, the implied warranties of merchantability,    *
 * fitness for a particular purpose, title and noninfringement, with respect *
 * to this software.                                                         *
 *                                                                           *
 *                                                                           *
 * For non-commercial use by Eyelink licencees only                          *
 *                                                                           *
 * Windows 95/98/NT/2000/XP sample experiment in C                           *
 * For use with Version 2.0 of EyeLink Windows API                           *
 *****************************************************************************/

#ifndef __SR_RESEARCH__PICTURE_H__
#define __SR_RESEARCH__PICTURE_H__

#ifdef MACOSX
#include <eyelink_core_graphics/sdl_expt.h>
#else
#include <sdl2_expt.h>
#endif

#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define numberOfBatchFractals 36

extern int fd;

extern int dispWidthmm;
extern int dispHeightmm;
extern int dispWidthpx;
extern int dispHeightpx;
extern int dispDistmm;
extern double disptheta;
extern double dispDegPerCm;
extern double dispDegPerPixel;
extern double dispPixelPerDeg;

extern int eye_used;
extern ALLF_DATA evt;			/* buffer to hold sample and event data*/

extern DISPLAYINFO dispinfo;  /* display information: size, colors, refresh rate*/
extern SDL_Surface *screen;   /* SDL surface for drawing */
extern SDL_Renderer *renderer;   /* SDL renderer for drawing */
extern SDL_Window *window;   /* SDL window for drawing */
extern SDL_Color target_background_color;   /* SDL color for the background */
extern SDL_Color target_foreground_color;   /* SDL color for the foreground drawing (text, calibration target, etc)*/

extern SDL_Surface  *screen2;
extern SDL_Window *window2;
extern SDL_Renderer *renderer2;

extern int setsToLoad[8];

extern SDL_Surface *Surface_Fractals[64];
extern SDL_Texture *Texture_Fixation;
extern SDL_Texture *Texture_Fixation_Window;
extern SDL_Texture *Texture_Gaze;
extern SDL_Texture *Texture_Grid;
extern SDL_Texture *Texture_Fractal_Prep;
extern SDL_Texture *Texture_Fractal;
extern SDL_Texture *Texture_Textbar;
extern SDL_Texture *Texture_Fixation_Monkey;
extern SDL_Texture *Texture_Fractals[64];
extern SDL_Texture *Texture_Fractals_Monkey[64];
extern SDL_Texture *Texture_Fractal;
extern SDL_Texture *Texture_Sensation_Window;
extern SDL_Texture *Texture_Fractal_Monkey;
extern SDL_Texture *Texture_Photodiode_Monkey;
extern SDL_Texture *Texture_Stats;
extern SDL_Rect bound;
extern SDL_Rect bound1;
extern SDL_Rect bound2;
extern SDL_Rect bound3;
extern SDL_Rect Rect_Regions[10];
extern SDL_Rect Rect_Sensation;
extern SDL_Rect Rect_Photodiode_Monkey;
extern int Region_Last;
extern int numberOfOutOfRegionSamplesAllowed;
extern int currentNumberOfOutOfRegionSamples;

extern int currentNumberOfTrials;
extern int currentNumberOfErroneousTrials;
extern int currentNumberOfTargetPresentTrials;
extern int currentNumberOfTargetAbsentTrials;
extern int currentNumberOfSkipsInTargetAbsentTrials;
extern int currentNumberOfSkipsInTargetPresentTrials;
extern int currentNumberOfCorrectChoicesInTargetPresentTrials;

extern int Regions_Center_X;
extern int Regions_Center_Y;
extern double Regions_Offset;
extern int Regions_Inter_Ring_Distance;
extern int Regions_Width;
extern int Regions_Height;

extern int Region_Choice_Idx;

extern int Regions_Center_X_Monkey;
extern int Regions_Center_Y_Monkey;
extern int Regions_Inter_Ring_Distance_Monkey;
extern int Regions_Offset_Monkey;
extern int Regions_Width_Monkey;
extern int Regions_Height_Monkey;

extern SDL_Rect Rect_Fixation;
extern SDL_Rect Rect_Fixation_Monkey;
extern SDL_Rect Rect_Fixation_Window;
extern SDL_Rect Rect_Gaze;
extern SDL_Rect Rect_Fixation_Search;
extern SDL_Rect Rect_Fixation_Search_Monkey;
extern SDL_Rect Rect_Fixation_Search_Window;
extern SDL_Color Color_Background;
extern SDL_Color Color_Fixation;
extern SDL_Color Color_Fixation_Search;
extern SDL_Color Color_Fixation_Search_Monkey;
extern SDL_Color Color_Fixation_Search_Window;
extern SDL_Color Color_Fixation_Window_Neutral;
extern SDL_Color Color_Fixation_Window_Hold;
extern SDL_Color Color_Fixation_Window_Error;
extern SDL_Color Color_Fixation_Window_Success;
extern SDL_Color Color_Gaze;
extern SDL_Color Color_Grid;
extern SDL_Color Color_Photodiode;
extern SDL_Color Color_Stats;

extern int Reward_Skip_Duration;
extern int Reward_Skip_Probability;

extern int goodFractalQueueIdx[36]; // indexes of good fractals
extern int badFractalQueueIdx[36]; // indexes of bad fractals
extern int trialTypePerm[numberOfBatchFractals]; // target present or absent
extern int fractalNumPerm[numberOfBatchFractals]; //3, 5, 7, 9 fractals on trial
extern int goodFractRegionPerm[numberOfBatchFractals]; //what region to put the good fract
extern int numberOfFractals;
extern int currentSequenceNumber;
extern int currentBatchSequenceNumber;
extern int currentTrialType;
extern int currentGoodFractalQueuePos;
extern int currentBadFractalQueuePos;
extern int currentNumOfFractals;
extern int currentGoodFractalRegion;

extern float fixationBoundaryDegrees;
extern int drawFractalsMonkey;

extern int reloadedConf;
extern int isPhotodiodeCheckOn;

extern float renderer2ScaleX;
extern float renderer2ScaleY;

//extern int isNextFramePreparedMonkey;

int run_trials(void);   /* This code sequences trials within a block. */
void clear_full_screen_window(SDL_Color c);  /* Clear the window with a specific color */

/* Run a single trial, recording to EDF file only */
//int simple_recording_trial(char *text, UINT32 time_limit);
typedef enum {FixationOn, FixationOff} fixationStateEnum;
typedef enum {NoGaze, WaitGaze, GazeFix} gazeStateEnum;
//typedef enum {UndeterminedFixation, BrokenFixation, WaitGazeTimeout, SuccessfulFixation} fixationRunResultEnum;
typedef enum {Experiment_State_Init, Experiment_State_Prep, Experiment_State_Run, Experiment_State_Exit} experimentStateEnum;
typedef enum {State_Phase_Enter, State_Phase_Inside, State_Phase_Exit} statePhaseEnum;

typedef enum {DidntGaze, Gazed} gazeWaitResultEnum;
typedef enum {Gaze_Fix_Result_Undetermined, Gaze_Fix_Result_Broke, Gaze_Fix_Result_KeptGaze} gazeFixResultEnum;
typedef enum {Gaze_Fract_Fix_Result_Undetermined, Gaze_Fract_Fix_Result_Broke, Gaze_Fract_Fix_Result_KeptGaze} gazeFractFixResultEnum;
typedef enum {Monkey_Gaze_State_HasLookedInsideFixationWindow, Monkey_Gaze_State_HasNotLookedInsideFixationWindow} monkeyGazeStateEnum;
typedef enum {Monkey_Fract_Gaze_State_Hasnt_Left_Fixation_Window, Monkey_Fract_Gaze_State_Has_Left_Fixation_Window } monkeyFractGazeStateEnum;
typedef enum {Monkey_Fract_Choice_Undetermined, Monkey_Fract_Choice_Good, Monkey_Fract_Choice_Bad, Monkey_Fract_Choice_Reject} monkeyFractChoiceEnum;
typedef enum {Trial_Run_Result_Undetermined ,Trial_Run_Result_Timeout, Trial_Run_Result_Reject_Before_Exitting_Fixation_Window, Trial_Run_Result_Reject_After_Exitting_Fixation_Window, Trial_Run_Result_Commit_On_Good, Trial_Run_Result_Commit_On_Bad, Trial_Run_Result_Fixed_On_Good, Trial_Run_Result_Fixed_On_Bad, Trial_Run_Result_Len} trialRunResultEnum;

extern trialRunResultEnum trialRunResult;
extern trialRunResultEnum lastTrialRunResult;
typedef enum {UndeterminedRunResult, SuccessfulRun, Failed} runResultEnum;

typedef struct trialStateStruct{
	UINT32 expirationTime;
	int hasExpired;
};

//typedef enum {Trial_State_ITI, Trial_State_Wait_For_Gaze, Trial_State_Gaze_Fix, Trial_State_Result, Trial_State_Len} trialStatesEnum;
typedef enum {Trial_State_ITI, Trial_State_Wait_For_Gaze, Trial_State_Fixation_Fix, Trial_State_Search, Trial_State_Fractal_Fix , Trial_State_Result, Trial_State_Len} trialStatesEnum;
typedef enum {Trial_Result_Committed_Good, Trial_Result_Committed_Bad, Trial_Result_Skipped, Trial_Result_Timeout, Trial_Result_Broke, Trial_Result_Len} trialResultsEnum;
typedef enum {Random_Permutation_Type_Sorted, Random_Permutation_Type_Shuffle, Random_Permutation_Type_Fract_Idx_Good, Random_Permutation_Type_Fract_Idx_Bad, Random_Permutation_Type_Trial_Type_Targert_Wise, Random_Permutation_Type_Fractal_Number} randomPermutationTypeEnum;
//typedef enum {Search_Case_Reject_Before_Fixation_Window_Exit, Search_Case_Reject_After_Or_Commit, Search_Case_Len} searchCasesEnum;
typedef enum {Search_Event_Bring_Back_Fixation, Search_Event_Reject_Before_Exiting_Fixation, Search_Event_Reject_After_Exiting_Fixation, Search_Event_Commit, Search_Event_Len} searchEventsEnum;

typedef struct searchEventsStruct{
	UINT32 startTime;
	UINT32 expirationTime;
	int hasExpired;
	int canBeExpired;
	int hasActed;
};
extern struct searchEventsStruct searchEvents[Search_Event_Len];

extern struct trialStateStruct trialStates[Trial_State_Len];
//extern struct searchCasesStruct searchCases[Search_Case_Len];

int run_trials(void);   /* This code sequences trials within a block. */
void clear_full_screen_window(SDL_Color c);  /* Clear the window with a specific color */

/* Run a single trial, recording to EDF file only */
int simple_recording_trial();

extern experimentStateEnum experimentState;
extern trialStatesEnum trialState;
extern statePhaseEnum statePhase;
extern gazeWaitResultEnum gazeWaitResult;
extern gazeFixResultEnum gazeFixResult;
extern gazeFixResultEnum gazeFractFixResult;
extern monkeyGazeStateEnum monkeyGazeState;
extern monkeyFractGazeStateEnum monkeyFractGazeState;
extern monkeyFractChoiceEnum monkeyFractChoice;
extern runResultEnum runResult;
extern trialResultsEnum trialResult;
extern trialResultsEnum previousTrialResult;

extern UINT32 trialStateStartTime;
extern UINT32 currentRegionStartTime;
//extern runResultEnum lastRunResult;
//extern SDL_Surface *hbm;

/* Run a single bitmap trial, recording to EDF file only */
//int bitmap_recording_trial(SDL_Surface *gbm, UINT32 time_limit);

/* convenient macros */
#define SETCOLOR(x,red,green,blue) x.r =red; x.g = green; x.b =blue; 


#ifndef WIN32
#define _stricmp strcasecmp
#endif
#endif

/*
void drawFixation();
void eraseFixation();
void drawFixationWindow(SDL_Color color);
void eraseFixationWindow();
void drawGaze();
void eraseGaze();
void setGaze();
void updateFrame();
int timedOut();
void gotoExperimentState(experimentStateEnum state);
void gotoState(trialStatesEnum state);
void gotoStatePhase (statePhaseEnum state);


void prepareNextTrial();
int isInsideFixationWindow();
void processGaze();
int hasLookedIntoFixationWindow();
int hasBrokenFixation();
int hasKeptFixation();
void setFixationSuccess();

void randGen(int len, int *arr, int isRand);

SDL_Rect getSensationRect();
SDL_Rect getTransformedRect(SDL_Rect i);
*/	


void drawFixation();
void drawFixationMonkey();
void drawNextFrameMonkey(trialStatesEnum state);
void eraseFixation();
void eraseFixationMonkey();
void drawFixationWindow(SDL_Color color);
void drawSensationWindow();
void eraseSensationWindow();
void drawFract();
void eraseFract();
void drawFractMonkey();
void eraseFractMonkey();
void drawFract();
void eraseFixationWindow();
void drawGaze();
void eraseGaze();
void drawPhotodiodeMonkey();
void erasePhotodiodeMonkey();
void setGaze();
void updateFrame();
void prepareFrameMonkey();
void updateFrameMonkey();
void writeToBlackrock(char *msg);
void writeToEyelink(char *msg);
void playError();
void playSuccess();
void gotoExperimentState(experimentStateEnum state);
void gotoState(trialStatesEnum state);
void gotoStatePhase (statePhaseEnum state);
int timedOut();
void prepareNextTrial();
int isInsideFixationWindow();
int isInsideSensationWindow();
void processGaze();
int hasLookedIntoFixationWindow();
int hasBrokenFixation();
int hasKeptFixation();
void setFixationSuccess();
int hasLookedIntoSensationWindow();
int hasBrokenFractFixation();
int hasKeptFractFixaton();
void setFractFixationSuccess();
void setNextFrameMonkeyUnprepared();
void setNextFrameMonkeyUndrawn();
SDL_Rect scaleRenderer(SDL_Rect i);
int getRegion(int n);
int getFractal(int n);
int getRegion(int n);
int getFractal(int n);
SDL_Rect getSensationRect();
SDL_Rect getTransformedRect(SDL_Rect i);
int getTransformedEdge(int in);
double getUntransformedEdge(int in);
void randGen(int len, int *arr, randomPermutationTypeEnum type);


void drawSensationWindowGood();
void drawSensationWindowBad();

SDL_Rect getSensationRectGood();
SDL_Rect getSensationRectBad();
	
double pixelToDeg(int pixels);
double degToPixels(double degrees);

void getPhotodiodeAck();
void sendEventToNeuralData(char *msg);
void initgetPhotodiodeAck();
void stopPollingForPhotodiodeAck();

SDL_Rect getUntransformedRect(SDL_Rect i);
