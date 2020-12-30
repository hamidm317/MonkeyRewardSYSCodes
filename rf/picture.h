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
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>

extern int fd;

extern int eye_used;
extern ALLF_DATA evt;			/* buffer to hold sample and event data*/

extern DISPLAYINFO dispinfo;  /* display information: size, colors, refresh rate*/
extern SDL_Surface *screen;   /* SDL surface for drawing */
extern SDL_Renderer *renderer;   /* SDL renderer for drawing */
extern SDL_Window *window;   /* SDL window for drawing */
extern SDL_Color target_background_color;   /* SDL color for the background */
extern SDL_Color target_foreground_color;   /* SDL color for the foreground drawing (text, calibration target, etc)*/

extern int dispWidthmm;
extern int dispHeightmm;
extern int dispWidthpx;
extern int dispHeightpx;
extern int dispDistmm;
extern double disptheta;
extern double dispDegPerCm;
extern double dispDegPerPixel;
extern double dispPixelPerDeg;

extern SDL_Surface  *screen2;
extern SDL_Window *window2;
extern SDL_Renderer *renderer2;

extern SDL_Surface *Surface_Fractals[8];
extern SDL_Texture *Texture_Fixation;
extern SDL_Texture *Texture_Fixation_Window;
extern SDL_Texture *Texture_Gaze;
extern SDL_Texture *Texture_Grid;
extern SDL_Texture *Texture_Fractal;
extern SDL_Texture *Texture_Textbar;
extern SDL_Texture *Texture_Fixation_Monkey;
extern SDL_Texture *Texture_Fractals[8];
extern SDL_Texture *Texture_Fractals_Monkey[8];
extern SDL_Texture *Texture_Fractal;
extern SDL_Texture *Texture_Fractal_Monkey;
extern SDL_Texture *Texture_Sensation_Window;
extern SDL_Texture *Texture_Edit_Overlay;
extern SDL_Texture *Texture_Photodiode_Monkey;
extern SDL_Rect bound;
extern SDL_Rect bound1;
extern SDL_Rect bound2;
extern SDL_Rect bound3;
extern SDL_Rect Rect_Regions[33];
extern SDL_Rect Rect_Sensation;
extern SDL_Rect Rect_Photodiode_Monkey;
//extern SDL_Point Point_Bounds_Circle_Sector[8];

extern int Regions_Center_X;
extern int Regions_Center_Y;
extern int Regions_Offset;
extern int Regions_Inter_Ring_Distance;
extern int Regions_Width;
extern int Regions_Height;

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
extern SDL_Color Color_Background;
extern SDL_Color Color_Fixation;
extern SDL_Color Color_Fixation_Window_Neutral;
extern SDL_Color Color_Fixation_Window_Hold;
extern SDL_Color Color_Fixation_Window_Error;
extern SDL_Color Color_Fixation_Window_Success;
extern SDL_Color Color_Gaze;
extern SDL_Color Color_Grid;
extern SDL_Color Color_Edit_Overlay;
extern SDL_Color Color_Photodiode;


extern int randomSequenseNumbers[264];
extern int numberOfFractals;
extern int currentSequenceNumber;
extern int numberOfBatchFractals;
extern int currentBatchSequenceNumber;

extern float renderer2ScaleX_inv;
extern float renderer2ScaleY_inv;
extern int totalRun;

extern int loadFromConf;
extern int reloadedConf;
extern int isPhotodiodeCheckOn;

extern int defaultFractalSet;

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

typedef enum {DidntGaze, Gazed } gazeWaitResultEnum;
typedef enum {Gaze_Fix_Result_Undetermined, Gaze_Fix_Result_Broke, Gaze_Fix_Result_KeptGaze} gazeFixResultEnum;
typedef enum {Monkey_Gaze_State_HasLookedInsideFixationWindow, Monkey_Gaze_State_HasNotLookedInsideFixationWindow} monkeyGazeStateEnum;
typedef enum {Experiment_Mode_Run, Experiment_Mode_Edit} experimentModeEnum;

typedef enum {UndeterminedRunResult, SuccessfulRun, Failed} runResultEnum;

typedef struct trialStateStruct{
	UINT32 expirationTime;
};

//typedef enum {Trial_State_ITI, Trial_State_Wait_For_Gaze, Trial_State_Gaze_Fix, Trial_State_Result, Trial_State_Len} trialStatesEnum;
typedef enum {Trial_State_ITI, Trial_State_Wait_For_Gaze, Trial_State_Fract_Off, Trial_State_Fract_On, Trial_State_Result, Trial_State_Len} trialStatesEnum;

extern struct trialStateStruct trialStates[Trial_State_Len];

int run_trials(void);   /* This code sequences trials within a block. */
void clear_full_screen_window(SDL_Color c);  /* Clear the window with a specific color */

/* Run a single trial, recording to EDF file only */
int simple_recording_trial();

extern experimentStateEnum experimentState;
extern trialStatesEnum trialState;
extern statePhaseEnum statePhase;
extern gazeWaitResultEnum gazeWaitResult;
extern gazeFixResultEnum gazeFixResult;
extern monkeyGazeStateEnum monkeyGazeState;
extern runResultEnum runResult;

extern int fractSetToLoad;
extern UINT32 trialStateStartTime;
extern float errorVolume;
extern float successVolume;

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

void drawFixation();
void eraseFixation();
void drawFixationWindow(SDL_Color color);
void eraseFixationWindow();
void drawSensationWindow();
void eraseSensationWindow();
void drawGaze();
void eraseGaze();
void drawPhotodiodeMonkey();
void erasePhotodiodeMonkey();
void setGaze();
void updateFrame();
int timedOut();
/*
void gotoState(experimentStateType state);
void writeToBlackrock(char *msg);
void writeToEyelink(char *msg);
void playError();
void playSuccess();
*/
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
SDL_Rect getUntransformedRect(SDL_Rect i);

double getUntransformedEdge(int in);
double pixelToDeg(double pixels);

void getPhotodiodeAck();
void sendEventToNeuralData(char *msg);
void initgetPhotodiodeAck();
void stopPollingForPhotodiodeAck();

