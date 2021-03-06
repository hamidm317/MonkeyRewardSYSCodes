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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "picture.h"
#include <sdl2_text_support.h>
#ifdef WIN32
#include <w32_dialogs.h>
#endif

#include "SDL2/SDL_image.h"
#include "SDL2/SDL2_rotozoom.h"

DISPLAYINFO dispinfo; /* display information: size, colors, refresh rate */
/* Name for experiment: goes in task bar, and in EDF file */
char program_name[100] = "Eyelink Sample Experiment 2.0";
SDL_Surface  *screen = NULL;


FILE *fp;
SDL_Rect bound;
SDL_Rect bound1;
SDL_Rect bound2;
SDL_Rect bound3;
SDL_Rect Rect_Regions[10];

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface  *screen2 = NULL;
SDL_Window *window2 = NULL;
SDL_Renderer *renderer2 = NULL;
SDL_Texture *content2Texture = NULL;
SDL_Texture *content2TextureRect = NULL;
SDL_Texture *Texture_Textbar = NULL;
SDL_Texture *Texture_Fractal_Prep = NULL;
SDL_Texture *Texture_Fractal = NULL;
SDL_Texture *Texture_Fractal_Monkey = NULL;
/*The colors of the target and background for calibration and drift correct */
SDL_Color target_background_color ={0,0,0,255};
SDL_Color target_foreground_color ={128,0,128,255};
char *trackerip = NULL; /* use default IP address */

char trialIDstr[10];
int trialID;

SDL_Texture *Texture_Fixation;
SDL_Texture *Texture_Fixation_Window;
SDL_Texture *Texture_Gaze;
SDL_Texture *Texture_Grid;
SDL_Texture *Texture_Fixation_Monkey;
SDL_Surface *Surface_Fractals[24];
SDL_Texture *Texture_Fractals[24];
SDL_Texture *Texture_Fractals_Monkey[24];
SDL_Texture *Texture_Sensation_Window;
SDL_Texture *Texture_Photodiode_Monkey;
SDL_Color Color_Background;
SDL_Color Color_Fixation;
SDL_Color Color_Fixation_Search;
SDL_Color Color_Fixation_Search_Window;
SDL_Color Color_Fixation_Search_Monkey;
SDL_Color Color_Fixation_Window_Neutral;
SDL_Color Color_Fixation_Window_Hold;
SDL_Color Color_Fixation_Window_Error;
SDL_Color Color_Fixation_Window_Success;
SDL_Color Color_Gaze;
SDL_Color Color_Grid;
SDL_Color Color_Photodiode;

SDL_Rect Rect_Fixation;
SDL_Rect Rect_Fixation_Search;
SDL_Rect Rect_Fixation_Monkey;
SDL_Rect Rect_Fixation_Window;
SDL_Rect Rect_Fixation_Search_Monkey;
SDL_Rect Rect_Fixation_Search_Window;
SDL_Rect Rect_Gaze;
SDL_Rect Rect_Sensation;
SDL_Rect Rect_Photodiode_Monkey;

float renderer2ScaleX;
float renderer2ScaleY;

int Region_Last;

int Regions_Center_X;
int Regions_Center_Y;
int Regions_Inter_Ring_Distance;
double Regions_Offset;
int Regions_Width;
int Regions_Height;
int fd;
int numberOfOutOfRegionSamplesAllowed;
int currentNumberOfOutOfRegionSamples;

int randomSequenseNumbers[16];
int randomSequenseNumbersRegions[16];
int numberOfFractals;
int currentSequenceNumber;
int currentBatchSequenceNumber;
int currentTrialType;
int dispWidthmm;
int dispHeightmm;
int dispWidthpx;
int dispHeightpx;
int dispDistmm;
double disptheta;
double dispDegPerCm;
double dispDegPerPixel;
double dispPixelPerDeg;
//const double dispCmPerDeg;
//const double dispPixelPerDeg;
UINT32 currentRegionStartTime;
int isPhotodiodeCheckOn;

int setsToLoad[3];

int Region_Choice_Idx;

int goodFractalQueueIdx[36]; // indexes of good fractals
int badFractalQueueIdx[36]; // indexes of bad fractals
int currentGoodFractalQueuePos;
int currentBadFractalQueuePos;
int currentNumOfFractals;
int currentGoodFractalRegion;
int drawFractalsMonkey;

int Reward_Skip_Duration;
int Reward_Skip_Probability;

int trialTypePerm[numberOfBatchFractals]; // target present or absent
int fractalNumPerm[numberOfBatchFractals]; //3, 5, 7, 9 fractals on trial
//int goodFractRegionPerm[numberOfBatchFractals]; //what region to put the good fract
trialRunResultEnum trialRunResult;
trialRunResultEnum lastTrialRunResult;

int eye_used;
ALLF_DATA evt;			/* buffer to hold sample and event data*/
char our_file_name[260] = "Trial";
char our_file_name_final[260] = "Trial";

struct trialStateStruct trialStates[Trial_State_Len];
//struct searchCasesStruct searchCases[Search_Case_Len];
struct searchEventsStruct searchEvents[Search_Event_Len];

int exit_eyelink()
{
	/* CLEANUP*/
	close_expt_graphics();           /* tell EXPTSPPT to release window */
	close_eyelink_connection();      /* disconnect from tracker */
	return 0;
}

int end_expt(char * our_file_name)
{
	/*END: close, transfer EDF file */
	set_offline_mode(); /* set offline mode so we can transfer file */
	pump_delay(500);    /* delay so tracker is ready */
	/* close data file */
	eyecmd_printf("close_data_file");

	if(break_pressed())
	  return exit_eyelink(); /* don't get file if we aborted experiment */
	if(our_file_name[0]&& eyelink_is_connected())   /* make sure we created a file */
	{
		close_expt_graphics();           /* tell EXPTSPPT to release window */
		receive_data_file(our_file_name, "", 0);
	}
	/* transfer the file, ask for a local name */

	return exit_eyelink();
}


int get_tracker_sw_version(char* verstr)
{
	int ln = 0;
	int st =0;
	ln = strlen(verstr);
	while(ln>0 && verstr[ln -1]==' ')  
		verstr[--ln] = 0; // trim 

	// find the start of the version number
	st = ln;
	while(st>0 && verstr[st -1]!=' ')  st --; 
	return atoi(&verstr[st]);
	
}

int app_main(char * trackerip, DISPLAYINFO * disp)
{
	UINT16 i, j;
    char verstr[50];
    int eyelink_ver = 0;
    int tracker_software_ver = 0;
	char *vd = NULL;

	vd = getenv("SDL_VIDEODRIVER");
	if (vd)
#ifdef WIN32
		_putenv("SDL_VIDEODRIVER=");
#elif defined(UNIX)
		putenv("SDL_VIDEODRIVER=");
#endif


#ifdef WIN32
    edit_dialog(NULL,"Create EDF File", "Enter Tracker EDF file name:", our_file_name,260);
#endif
	if(trackerip)
		set_eyelink_address(trackerip);
	if(open_eyelink_connection(0))
	  return -1;       /* abort if we can't open link*/
	set_offline_mode();
	flush_getkey_queue();/* initialize getkey() system */
    eyelink_ver = eyelink_get_tracker_version(verstr);
    if (eyelink_ver == 3)
	  tracker_software_ver = get_tracker_sw_version(verstr);
///*
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
		printf("ErrorInit");
		return -1;
	}
		int displays = SDL_GetNumVideoDisplays();

	//SDL_Surface* screenSurface = NULL;

		
		SDL_GetDisplayBounds(0,&bound); 
		SDL_GetDisplayBounds(1,&bound1); 
		//printf("displays = %d\n,1: bound x = %d, bound y = %d\n, bound w = %d, bound h = %d\n", displays, bound1.x, bound1.y, bound1.w, bound1.h);
		

		//bound1.w = bound1.h;

		bound2.w = 1000;
		bound2.h = 1000;
		bound2.x = 2500;
		//bound2.x = bound.w - bound2.w - 320;
		bound2.y = bound.y;
		//bound2.w = bound1.w;
		//bound2.h = bound1.h;

		bound3.h = bound1.h;
		bound3.w = bound3.h;
		bound3.x = (bound1.w - bound3.w)/2;
		bound3.y = bound1.y;
		//printf("displays = %d\n,0: bound x = %d, bound y = %d\n, bound w = %d, bound h = %d\n", displays, bound.x, bound.y, bound.w, bound.h);
		//printf("displays = %d\n,1: bound x = %d, bound y = %d\n, bound w = %d, bound h = %d\n", displays, bound1.x, bound1.y, bound1.w, bound1.h);
		//printf("displays = %d\n,2: bound x = %d, bound y = %d\n, bound2 w = %d, bound2 h = sthbefore %d sthafter and hahaha", displays, bound2.x, bound2.y, bound2.w, bound2.h);
			//printf( "Before SDL window 1\n");
		//printf("disp: bound x = %d, bound y = %d\n, bound x2 = %d, bound y2 = %d\n",
		//		 dispinfo.left, dispinfo.top, dispinfo.right, dispinfo.bottom);
		//window = SDL_CreateWindow( "SDL Tutorial",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		//window = SDL_CreateWindow( "SDL Tutorial",bound1.x, bound1.y, bound1.w, bound1.h, SDL_WINDOW_FULLSCREEN_DESKTOP);
		window = SDL_CreateWindow( "Search Task",bound1.x, bound1.y, bound1.w, bound1.h, SDL_WINDOW_BORDERLESS);
			//printf( "After SDL window 1\n");
			//printf( "before rend\n");
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			//printf( "after rend\n");
		window2 = SDL_CreateWindow( "Search Task",bound2.x, bound2.y, bound2.w, bound2.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
			//printf( "After SDL window 2\n");
			renderer2 = SDL_CreateRenderer(window2, -1, SDL_RENDERER_PRESENTVSYNC);
			content2Texture = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound2.w, bound2.h);
			content2TextureRect = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound2.w, bound2.h);
			SDL_SetTextureBlendMode(content2TextureRect, SDL_BLENDMODE_BLEND);
			Texture_Fixation = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound2.w, bound2.h);
			SDL_SetTextureBlendMode(Texture_Fixation, SDL_BLENDMODE_BLEND);
			Texture_Fixation_Window = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound2.w, bound2.h);
			SDL_SetTextureBlendMode(Texture_Fixation_Window, SDL_BLENDMODE_BLEND);
			Texture_Gaze = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound2.w, bound2.h);
			SDL_SetTextureBlendMode(Texture_Gaze, SDL_BLENDMODE_BLEND);
			Texture_Grid = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound2.w, bound2.h);
			SDL_SetTextureBlendMode(Texture_Grid, SDL_BLENDMODE_BLEND);
			//printf( "after rend2\n");
			
			Texture_Fixation_Monkey = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound3.w, bound3.h);
			SDL_SetTextureBlendMode(Texture_Fixation_Monkey, SDL_BLENDMODE_BLEND);
			Texture_Fractal_Prep = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound3.w, bound3.h);
			SDL_SetTextureBlendMode(Texture_Fractal_Prep, SDL_BLENDMODE_BLEND);
			Texture_Fractal = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound3.w, bound3.h);
			SDL_SetTextureBlendMode(Texture_Fractal, SDL_BLENDMODE_BLEND);
			Texture_Fractal_Monkey = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound3.w, bound3.h);
			SDL_SetTextureBlendMode(Texture_Fractal_Monkey, SDL_BLENDMODE_BLEND);
			Texture_Sensation_Window = SDL_CreateTexture(renderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound2.w, bound2.h);
			SDL_SetTextureBlendMode(Texture_Sensation_Window, SDL_BLENDMODE_BLEND);
            Texture_Photodiode_Monkey = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bound1.w, bound1.h);
            SDL_SetTextureBlendMode(Texture_Photodiode_Monkey, SDL_BLENDMODE_BLEND);


			for (int i=0; i<3; i++)
				setsToLoad[i] = 121 + i;

			fractalLoaderFromSets(setsToLoad[i], setsToLoad[i+1], setsToLoad[i+2]);

			/*
			SDL_Surface *srf = IMG_Load("fracts/i1000.jpeg");
			for (int i=0; i<8; i++){
				char *str[100];
				char *idx[10];
				strcpy(str, "fracts/i100");
				sprintf(idx, "%d", i);
				strcat(str, idx);
				strcat(str, ".jpeg");
				Surface_Fractals[i] = IMG_Load(str);
				Texture_Fractals[i] = SDL_CreateTextureFromSurface(renderer2, Surface_Fractals[i]);
				SDL_SetTextureBlendMode(Texture_Fractals[i], SDL_BLENDMODE_BLEND);
				Texture_Fractals_Monkey[i] = SDL_CreateTextureFromSurface(renderer, Surface_Fractals[i]);
				SDL_SetTextureBlendMode(Texture_Fractals_Monkey[i], SDL_BLENDMODE_BLEND);
			}

*/
			//dispCmPerDeg; 
			//dispPixelPerDeg;

			//double pixelToDeg(double pixels);
			//double degToPixels(double degrees);

			//printf();
			/*	
			disptheta = atan((dispHeightmm/2.0)/dispDistmm) * 180.0 / M_PI;
			dispDegPerCm = (disptheta/(dispHeightmm/2.0)) * 10.0;
			dispDegPerPixel = disptheta / (dispHeightpx/2);

			printf("dispTheta = %f, dispDeg/Cm = %f, dispDeg/pixel = %f\n", disptheta, dispDegPerCm, dispDegPerPixel);
			printf("4 degrees is pixel %f/%f\n", degToPixels(4), bound1.h);
			*/
			//int randtemp[32];
			//randGen(32, randtemp, 1);
			//memcpy(randomSequenseNumbers, randtemp, 16*sizeof(int));
			//memcpy(randomSequenseNumbersRegions, (randtemp+(16)), 16*sizeof(int));
			//for (int i=0; i<16; i++){
			//	randomSequenseNumbers[i] %= 16;
			//	randomSequenseNumbersRegions[i] %= 16;
			//}
			randGen(numberOfBatchFractals, goodFractalQueueIdx, Random_Permutation_Type_Fract_Idx_Good);
			randGen(numberOfBatchFractals, badFractalQueueIdx, Random_Permutation_Type_Fract_Idx_Bad);
			randGen(numberOfBatchFractals, fractalNumPerm, Random_Permutation_Type_Fractal_Number); 
//			randGen(numberOfBatchFractals, goodFractRegionPerm, Random_Permutation_Type_Good_Fractal_Region);
			
			//for (int i=0; i<64; i++)
			//	printf("Rand[%d] = %d\n", i, randomSequenseNumbers[i]);
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		}
		else if( window2 == NULL )
		{
			printf( "Window2 could not be created! SDL_Error: %s\n", SDL_GetError() );
		}
		else
		{
			//printf( "Inside else\n");
			//Get window surface
			//screen = SDL_GetWindowSurface( window );

			int displays = SDL_GetNumVideoDisplays();
			if (!renderer2){
				printf("Failed to init r2");
			}
			//SDL_SetRenderDrawColor (renderer2, 0xFF,0xFF,0xFF,0xFF);


			//if (displays <= 1){
				//printf("displays = %d\n, bound x = %d, bound y = %d\n, bound w = %d, bound h = %d", displays, bound.x, bound.y, bound.w, bound.h);
			//	return -1;
			//}

			//Fill the surface white
			//SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ) );
			//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			//Update the surface
			//SDL_UpdateWindowSurface( window );
			//printf( "End of else\n");
		}

	


//*
	renderer2ScaleX = (float) bound1.w / (float) bound2.w;
	renderer2ScaleY = (float) bound1.h / (float) bound2.h;
	//printf("x , y Scale = %f, %f" , renderer2ScaleX, renderer2ScaleY);

	eye_used = -1;

	Color_Background.r = 0;
	Color_Background.g = 0;
	Color_Background.b = 0;
	Color_Background.a = 255;

	Color_Fixation.r = 128;
	Color_Fixation.g = 0;
	Color_Fixation.b = 128;
	Color_Fixation.a = 255;

	Color_Fixation_Search.r = 128;
	Color_Fixation_Search.g = 0;
	Color_Fixation_Search.b = 128;
	Color_Fixation_Search.a = 255;

	Color_Fixation_Search_Window.r = 64;
	Color_Fixation_Search_Window.g = 64;
	Color_Fixation_Search_Window.b = 64;
	Color_Fixation_Search_Window.a = 255;

	Color_Fixation_Search_Monkey.r = 128;
	Color_Fixation_Search_Monkey.g = 0;
	Color_Fixation_Search_Monkey.b = 128;
	Color_Fixation_Search_Monkey.a = 255;
	
	Color_Fixation_Window_Neutral.r = 255;
	Color_Fixation_Window_Neutral.g = 255;
	Color_Fixation_Window_Neutral.b = 255;
	Color_Fixation_Window_Neutral.a = 255;
	
	Color_Fixation_Window_Hold.r = 255;
	Color_Fixation_Window_Hold.g = 0;
	Color_Fixation_Window_Hold.b = 255;
	Color_Fixation_Window_Hold.a = 255;

	Color_Fixation_Window_Error.r = 255;
	Color_Fixation_Window_Error.g = 0;
	Color_Fixation_Window_Error.b = 0;
	Color_Fixation_Window_Error.a = 255;

	Color_Fixation_Window_Success.r = 0;
	Color_Fixation_Window_Success.g = 0;
	Color_Fixation_Window_Success.b = 255;
	Color_Fixation_Window_Success.a = 255;

	Color_Gaze.r = 255;
	Color_Gaze.g = 0;
	Color_Gaze.b = 0;
	Color_Gaze.a = 255;

	Color_Grid.r = 0;
	Color_Grid.g = 16;
	Color_Grid.b = 0;
	Color_Grid.a = 255;

    Color_Photodiode.r = 255;
    Color_Photodiode.g = 255;
    Color_Photodiode.b = 255;
    Color_Photodiode.a = 255;

	Rect_Fixation_Monkey.x = bound3.w/2 - Rect_Fixation_Monkey.w/2;
	Rect_Fixation_Monkey.y = bound3.h/2 - Rect_Fixation_Monkey.h/2;

	Rect_Fixation_Search_Monkey.x = bound3.w/2 - Rect_Fixation_Search_Monkey.w/2;
	Rect_Fixation_Search_Monkey.y = bound3.h/2 - Rect_Fixation_Search_Monkey.h/2;

	Rect_Fixation_Window.x = Rect_Fixation_Monkey.x - ((Rect_Fixation_Window.w - Rect_Fixation_Monkey.w)/2);
	Rect_Fixation_Window.y = Rect_Fixation_Monkey.y - ((Rect_Fixation_Window.h - Rect_Fixation_Monkey.h)/2);

	Rect_Fixation_Search_Window.x = Rect_Fixation_Search_Monkey.x - ((Rect_Fixation_Search_Window.w - Rect_Fixation_Search_Monkey.w)/2);
	Rect_Fixation_Search_Window.y = Rect_Fixation_Search_Monkey.y - ((Rect_Fixation_Search_Window.h - Rect_Fixation_Search_Monkey.h)/2);

    Rect_Photodiode_Monkey.x = 0;
    Rect_Photodiode_Monkey.y = bound1.h - Rect_Photodiode_Monkey.h;
	
	Rect_Fixation = getTransformedRect(Rect_Fixation_Monkey);
	Rect_Regions[0] = Rect_Fixation_Window;
	Rect_Fixation_Search = getTransformedRect(Rect_Fixation_Search_Monkey);
	Rect_Fixation_Window = getTransformedRect(Rect_Fixation_Window);
	Rect_Fixation_Search_Window = getTransformedRect(Rect_Fixation_Search_Window);

	Rect_Gaze = getTransformedRect(Rect_Gaze);
	Rect_Sensation = getTransformedRect(Rect_Sensation);

//typedef enum {Trial_State_ITI, Trial_State_Wait_For_Gaze, Trial_State_Gaze_Fix, Trial_State_Result, Trial_State_Len} trialStatesEnum;
	for (int i=0; i<Trial_State_Len; i++){
		trialStates[i].hasExpired = 0;
	}

	//searchCases[Search_Case_Reject_Before_Fixation_Window_Exit].expirationTime = 600;
	//searchCases[Search_Case_Reject_After_Or_Commit].expirationTime = 300;
	//searchTimeoutToBringFixationBack = 1000;
	for (int i=0; i<Search_Event_Len; i++){
		searchEvents[i].canBeExpired = 0;
		searchEvents[i].hasExpired = 0;
		searchEvents[i].hasActed = 0;
	}


    if(init_expt_window(&window, disp)) /* register window with EXPTSPPT*/
	{
		alert_printf("init_expt_graphics failed\n");
		return exit_eyelink();
	}

	if(!window)
	{
		alert_printf("invalid window\n");
		return exit_eyelink();
	}

	renderer = SDL_GetRenderer(window);
	renderer2 = SDL_GetRenderer(window2);
	if(!renderer)
	{
		alert_printf("invalid renderer\n");
		return exit_eyelink();
	}
	if(!renderer2)
	{
		alert_printf("invalid renderer2\n");
		return exit_eyelink();
	}

	get_display_information(&dispinfo); /* get window size, characteristics*/

	/* NOTE: Camera display does not support 16-color modes
	 NOTE: Picture display examples don't work well with 256-color modes
		   However, all other sample programs should work well.
	*/
	if(dispinfo.palsize==16)      /* 16-color modes not functional */
	{
	  alert_printf("This program cannot use 16-color displays");
	  return exit_eyelink();
	}

	/* 256-color modes: palettes not supported by this example*/
	if(dispinfo.palsize)
	  alert_printf("This program is not optimized for 256-color displays");



	//i = SCRWIDTH/60;        /* select best size for calibration target */
	//j = SCRWIDTH/300;       /* and focal spot in target  */
	i = SCRWIDTH/70;        /* select best size for calibration target */
	j = SCRWIDTH/300;       /* and focal spot in target  */
	if(j < 2) j = 2;
	set_target_size(i, j);  /* tell DLL the size of target features */
	set_calibration_colors(&target_foreground_color, &target_background_color); /* tell EXPTSPPT the colors*/

	set_cal_sounds("off", "off", "off");
	set_dcorr_sounds("off", "off", "off");


	clear_full_screen_window(target_background_color);    /* clear screen */
	get_new_font("Times Roman", SCRHEIGHT/32, 1);         /* select a font */
														/* Draw text */
	/*
	sdl2_printf(window, target_foreground_color, CENTER, SCRWIDTH/2, 1*SCRHEIGHT/30,
				 "EyeLink Demonstration Experiment: Sample Code");
	sdl2_printf(window, target_foreground_color, CENTER,SCRWIDTH/2, 2*SCRHEIGHT/30,
				 "Included with the Experiment Programming Kit for Windows");
	sdl2_printf(window, target_foreground_color, CENTER,SCRWIDTH/2, 3*SCRHEIGHT/30,
				 "All code is Copyright (c) 1997-2002 SR Research Ltd.");
	sdl2_printf(window, target_foreground_color, CENTER,SCRWIDTH/5, 4*SCRHEIGHT/30,
				 "Source code may be used as template for your experiments.");

	*/
	SDL_RenderPresent(renderer);
	SDL_RenderPresent(renderer2);
	pump_delay(500);

	if(our_file_name[0])    /* If file name set, open it */
	{
	  /* add extension */
	  if(!strstr(our_file_name, ".")) strcat(our_file_name, ".EDF");
	  i = open_data_file(our_file_name); /* open file */
	  if(i!=0)                           /* check for error */
		{
		  alert_printf("Cannot create EDF file '%s'", our_file_name);
		  return exit_eyelink();
		}                                /* add title to preamble */
	  eyecmd_printf("add_file_preamble_text 'RECORDED BY %s' ", program_name);
	}

	/* Now configure tracker for display resolution */
	/* Set display resolution */
	eyecmd_printf("screen_pixel_coords = %ld %ld %ld %ld",
				 dispinfo.left, dispinfo.top, dispinfo.right, dispinfo.bottom);
	/* Setup calibration type */
	eyecmd_printf("calibration_type = HV9");

	/* Add resolution to EDF file */
	eyemsg_printf("DISPLAY_COORDS %ld %ld %ld %ld",
				 dispinfo.left, dispinfo.top, dispinfo.right, dispinfo.bottom);
	if(dispinfo.refresh>40)
	eyemsg_printf("FRAMERATE %1.2f Hz.", dispinfo.refresh);

	/* SET UP TRACKER CONFIGURATION */
	/* set parser saccade thresholds (conservative settings) */
  if(eyelink_ver>=2)
    {
      eyecmd_printf("select_parser_configuration 0");  // 0 = standard sensitivity
	  if(eyelink_ver == 2) //turn off scenelink camera stuff
	  {
		eyecmd_printf("scene_camera_gazemap = NO");
	  }
    }
  else
    {
      eyecmd_printf("saccade_velocity_threshold = 35");
      eyecmd_printf("saccade_acceleration_threshold = 9500");
    }
  // NOTE: set contents before sending messages!
		     // set EDF file contents 
  eyecmd_printf("file_event_filter = LEFT,RIGHT,FIXATION,SACCADE,BLINK,MESSAGE,BUTTON,INPUT");
  eyecmd_printf("file_sample_data  = LEFT,RIGHT,GAZE,AREA,GAZERES,STATUS%s,INPUT",(tracker_software_ver>=4)?",HTARGET":"");
		      // set link data (used for gaze cursor) 
  eyecmd_printf("link_event_filter = LEFT,RIGHT,FIXATION,SACCADE,BLINK,BUTTON,INPUT");
  eyecmd_printf("link_sample_data  = LEFT,RIGHT,GAZE,GAZERES,AREA,STATUS%s,INPUT",(tracker_software_ver>=4)?",HTARGET":"");

	/* Program button #5 for use in drift correction */
	eyecmd_printf("button_function 5 'accept_target_fixation'");

	/* make sure we're still alive */
	if(!eyelink_is_connected() || break_pressed())
	  return end_expt(our_file_name);

	/*
	   RUN THE EXPERIMENTAL TRIALS (code depends on type of experiment)
	   Calling run_trials() performs a calibration followed by trials
	   This is equivalent to one block of an experiment
	   It will return ABORT_EXPT if the program should exit
	*/
	printf("launching trial");
//	printf("100 px and 200 px are %f and %f degs\n\n", pixelToDeg(100), pixelToDeg(200));
//	printf("\n\nUE of ecc, frsiz are %f %f\n", pixelToDeg(Regions_Inter_Ring_Distance), pixelToDeg(Regions_Width));
	reloadConf();
	i = run_trials();


	printf("Saving trialID = %d to file\n" , trialID);
        if(fp = fopen("count.txt", "w")){
                fprintf(fp,"%d" ,++trialID);
        }
        fclose(fp);

        strcat (our_file_name_final, "_ID_");
        strcat (our_file_name_final, trialIDstr);
        strcat (our_file_name_final, "_Search");
        strcat (our_file_name_final, ".EDF");




	return end_expt(our_file_name);
}


void clear_full_screen_window(SDL_Color c)
{
    // Set the color for drawing
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
	SDL_SetRenderDrawColor(renderer2, c.r, c.g, c.b, 255);

	// Clear the entire screen to selected color
    SDL_RenderClear(renderer);
    SDL_RenderClear(renderer2);

    // show window contents
    SDL_RenderPresent(renderer);
    SDL_RenderPresent(renderer2);
}

int parseArgs(int argc, char **argv, char **trackerip, DISPLAYINFO *disp )
{
	int i =0;
	int displayset =0;
	memset(disp,0,sizeof(DISPLAYINFO));
	for( i =1; i < argc; i++)
	{
		/*
		if(_stricmp(argv[i],"-numoftrials") ==0 && argv[i+1]){
			i++;
			numberOfBatchExperiments = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-set") ==0 && argv[i+1]){
			i++;
			fractSetToLoad = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-iti") ==0 && argv[i+1]){
			i++;
			trialStates[Trial_State_ITI].expirationTime = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-gazewait") ==0 && argv[i+1]){
			i++;
			trialStates[Trial_State_Wait_For_Gaze].expirationTime = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-fixationfix") ==0 && argv[i+1]){
			i++;
			trialStates[Trial_State_Fixation_Fix].expirationTime = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-overlap") ==0 && argv[i+1]){
			i++;
			trialStates[Trial_State_Fract_Overlap].expirationTime = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-sacwait") ==0 && argv[i+1]){
			i++;
			trialStates[Trial_State_Wait_For_Saccade].expirationTime = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-fractfix") ==0 && argv[i+1]){
			i++;
			trialStates[Trial_State_Fractal_Fix].expirationTime = atoi(argv[i]);
		}
		else if(_stricmp(argv[i],"-fixsize") ==0 && argv[i+1])
                {
                        i++;
                        Rect_Fixation_Monkey.w = atoi(argv[i]);
                        Rect_Fixation_Monkey.h = atoi(argv[i]);
                }
                else if(_stricmp(argv[i],"-fixwin") ==0 && argv[i+1])
                {
                        i++;
                        Rect_Fixation_Window.w = atoi(argv[i]);
                        Rect_Fixation_Window.h = atoi(argv[i]);
                }
		else if(_stricmp(argv[i],"-sensewin") ==0 && argv[i+1])
                {
                        i++;
                        Rect_Sensation.w = atoi(argv[i]);
                        Rect_Sensation.h = atoi(argv[i]);
                }
                else if(_stricmp(argv[i],"-radius") ==0 && argv[i+1])
                {
                        i++;
                        Regions_Inter_Ring_Distance = atoi(argv[i]);
                }
                else if(_stricmp(argv[i],"-fractsize") ==0 && argv[i+1])
                {
                        i++;
                        Regions_Width = atoi(argv[i]);
                        Regions_Height = atoi(argv[i]);
                }
                else if(_stricmp(argv[i],"-erramp") ==0 && argv[i+1])
                {
                        i++;
                        errorVolume = (float) atof(argv[i]);
                }
                else if(_stricmp(argv[i],"-sucamp") ==0 && argv[i+1])
                {
                        i++;
                        successVolume = (float) atof(argv[i]);
                }

	
		else{
                        printf("\t === General ===\n");
			printf("\t\t -numoftrials <number of force and choice trials to pack in a batch>. currently %d \n", numberOfBatchExperiments);
			printf("\t\t -set <fractal set>. currently %d \n", fractSetToLoad);
			printf("\t\t -conf <Load from config file> (Will override Timing settings below) . currently %d \n", loadFromConf);
                        printf("\t === Timing ===\n");
			printf("\t\t -iti <iti time>. currently %d \n", trialStates[Trial_State_ITI].expirationTime);
			printf("\t\t -gazewait <time to wait for gaze>. currently %d \n", trialStates[Trial_State_Wait_For_Gaze].expirationTime);
			printf("\t\t -fixationfix <time to fixate on fixation point>. currently %d \n", trialStates[Trial_State_Fixation_Fix].expirationTime);
			printf("\t\t -overlap <overlap time of fixation and fractal(s)>. currently %d \n", trialStates[Trial_State_Fract_Overlap].expirationTime);
			printf("\t\t -sacwait <time to wait for saccade>. currently %d \n", trialStates[Trial_State_Wait_For_Saccade].expirationTime);
			printf("\t\t -fractfix <time to fixate on fractal>. currently %d \n", trialStates[Trial_State_Fractal_Fix].expirationTime);
			printf("\t === Fractal ===\n");
                        printf("\t\t -fixsize <size of fixation in pixels>. currently %d \n", Rect_Fixation_Monkey.w);
                        printf("\t\t -fixwin <size of fixation window in pixels>. currently %d \n", Rect_Fixation_Window.w);
                        printf("\t\t -sensewin <size of sensation window in pixels>. currently %d \n", Rect_Sensation.w);
                        printf("\t\t -radius <radius to display fractals from center in pixels>. currently %d \n", Regions_Inter_Ring_Distance);
                        printf("\t\t -fractsize <size of fractal in pixels>. currently %d \n", Regions_Width);
                        printf("\t === Sound ===\n");
                        printf("\t\t -erramp <amplitude of error tone>. currently %f\n", errorVolume);
                        printf("\t\t -sucamp <amplitude of success tone>. currently %f\n", successVolume);
			return 1;
		}
		*/
	}	


	if(displayset && !disp->width && !disp->height)
		return 1;
	return 0;
}
#if  defined(WIN32) && !defined(_CONSOLE)
/* WinMain - Windows calls this to execute application */
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	app_main(NULL,NULL);/* call our real program */
	return 0;
}
#else
/* non windows application or win32 console application. */
int main(int argc, char ** argv)
{
srand(time(NULL));
	DISPLAYINFO disp;
	char *trackerip = NULL;

	dispWidthmm = 365;
	dispHeightmm = 270;
	dispWidthpx = 1280;
	dispHeightpx = 960;
	dispDistmm = 570;

	disptheta = atan((dispHeightmm/2.0)/dispDistmm) * 180.0 / M_PI;
	dispDegPerCm = (disptheta/(dispHeightmm/2.0)) * 10.0;
	dispDegPerPixel = disptheta / (dispHeightpx/2);
	dispPixelPerDeg = (dispHeightpx/2) / disptheta;
	/*
	printf("dispPixelPerDeg = %f\n", dispPixelPerDeg);
	printf("dispTheta = %f, dispDeg/Cm = %f, dispDeg/pixel = %f\n", disptheta, dispDegPerCm, dispDegPerPixel);
	printf("4 degrees is pixel %f/%f\n", degToPixel(4.0), bound1.h);
	*/

	eyemsg_printf(" -dispWidthmm  %d", dispWidthmm);
	eyemsg_printf(" -dispHeightmm %d", dispHeightmm);
	eyemsg_printf(" -dispWidthpx  %d", dispWidthpx);
	eyemsg_printf(" -dispHeightpx  %d", dispHeightpx);
	eyemsg_printf(" -dispDistmm  %d", dispDistmm);
	eyemsg_printf(" -disptheta  %f", disptheta);
	eyemsg_printf(" -dispDegPerCm  %f", dispDegPerCm);
	eyemsg_printf(" -dispDegPerPixel  %f", dispDegPerPixel );
	eyemsg_printf(" -dispPixelPerDeg  %f", dispPixelPerDeg);


	// === Defaults === //

	Regions_Inter_Ring_Distance = degToPixel(9.0); // paper 15 deg
	Regions_Offset = 0;
	Regions_Width = degToPixel(4.0);
	Regions_Height = degToPixel(4.0);

	Rect_Fixation_Monkey.w = (int)degToPixel(0.5);
	Rect_Fixation_Monkey.h = (int)degToPixel(0.5);
	Rect_Fixation_Search_Monkey.w = (int)degToPixel(0.5);
	Rect_Fixation_Search_Monkey.h = (int)degToPixel(0.5);
	Rect_Fixation_Window.w = (int)degToPixel(5.5);
	Rect_Fixation_Window.h = (int)degToPixel(5.5);
	Rect_Sensation.w = (int)degToPixel(5.0);
	Rect_Sensation.h = (int)degToPixel(5.0);
	Rect_Fixation_Search_Window.w = (int)degToPixel(5.5); // paper 5 deg
	Rect_Fixation_Search_Window.h = (int)degToPixel(5.5); // paper 5 deg
    Rect_Photodiode_Monkey.w= (int)degToPixel(1.0);
    Rect_Photodiode_Monkey.h= (int)degToPixel(1.0);
	Rect_Gaze.w = (int)degToPixel(0.25);
	Rect_Gaze.h = (int)degToPixel(0.25);
	

	trialStates[Trial_State_ITI].expirationTime = 3000; // paper: 1-1.5 ms
	//Rand()
	trialStates[Trial_State_Wait_For_Gaze].expirationTime = 2000; // couldn't find anything on paper
	trialStates[Trial_State_Fixation_Fix].expirationTime = 400; // paper 400ms
	trialStates[Trial_State_Search].expirationTime = 3000; //paper 3s
	trialStates[Trial_State_Fractal_Fix].expirationTime = 100; // paper 100ms
	trialStates[Trial_State_Result].expirationTime = 0; // --

	searchEvents[Search_Event_Reject_Before_Exiting_Fixation].expirationTime = 600; //paper : 600ms
	searchEvents[Search_Event_Reject_After_Exiting_Fixation].expirationTime = 400; //From paper, paper: 300ms
	searchEvents[Search_Event_Bring_Back_Fixation].expirationTime = 200; //couldn't find anything on paper
	searchEvents[Search_Event_Commit].expirationTime = 400; //paper 400ms


	int rv = parseArgs(argc,argv, &trackerip, &disp);

        if(fp = fopen("count.txt", "r")){
          fgets(trialIDstr, 10, fp);
          //fgets(succTrialIDstr, 10, fp);
        }
        fclose(fp);
        int trialIDParser = atoi (trialIDstr);
        trialID = trialIDParser;

	int numberOfOutOfRegionSamplesAllowed = 2;

	set_blocking (fd, 0);   
	set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	system("stty sane");
	char *portname = "/dev/ttyACM0";

        fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0)
        {
                portname = "/dev/ttyACM1";
                fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
                if (fd < 0)
                 {
                        printf ("error %d opening %s: %s", errno, portname, strerror (errno));
                        return;
                 }
        }

	system("export AUDIODRIVER=alsa");

	if(rv) return rv;

	printf("launching appmain");
	if(disp.width)
		app_main(trackerip, &disp);/* call our real program */
	else
		app_main(trackerip, NULL);/* call our real program - no display parameters set*/

	rename(our_file_name, our_file_name_final);
        char cmdstr[100];
        strcpy(cmdstr, "mv ./");
        strcat(cmdstr, our_file_name_final);
        strcat(cmdstr, " ./recorded_data/");
        system(cmdstr);
        printf("Saved: %s\n\n", our_file_name_final);

	return 0;
}
#endif

void fractalLoaderFromSet(int set){
        //101 , 102
        //set -= 100;
        int fractSetIdx = 1000 + (8 * (set - 100));
        char temp[100];
        char idxToStr[100];

        for (int i=0; i<8; i++){
                strcpy(temp, "/home/lab/Fractals/i");
                sprintf(idxToStr, "%d", fractSetIdx + i);
                strcat(temp, idxToStr);
                strcat(temp, ".jpeg");
		Surface_Fractals[i] = IMG_Load(temp);
		Texture_Fractals[i] = SDL_CreateTextureFromSurface(renderer2, Surface_Fractals[i]);
		SDL_SetTextureBlendMode(Texture_Fractals[i], SDL_BLENDMODE_BLEND);
		Texture_Fractals_Monkey[i] = SDL_CreateTextureFromSurface(renderer, Surface_Fractals[i]);
		SDL_SetTextureBlendMode(Texture_Fractals_Monkey[i], SDL_BLENDMODE_BLEND);

        }
}


void fractalLoaderFromSets(int set1, int set2, int set3){
        //101 , 102
        //set -= 100;
        int fractSet1Idx = 1000 + (8 * (set1 - 100));
        int fractSet2Idx = 1000 + (8 * (set2 - 100));
        int fractSet3Idx = 1000 + (8 * (set3 - 100));
        char temp[100];
        char idxToStr[100];

        for (int i=0; i<24; i++){
                strcpy(temp, "/Fractals/i");
		if (i<8){
                	sprintf(idxToStr, "%d", fractSet1Idx + i);
		}
		else if ( i>=8 && i<16){
                	sprintf(idxToStr, "%d", fractSet2Idx + i);
		}
		else if ( i>=16 && i<24){
                	sprintf(idxToStr, "%d", fractSet3Idx + i);
		}
                strcat(temp, idxToStr);
                strcat(temp, ".jpeg");
		Surface_Fractals[i] = IMG_Load(temp);
		printf("Loading Fractal #%d : %s.jpeg\n", i, idxToStr);
		Texture_Fractals[i] = SDL_CreateTextureFromSurface(renderer2, Surface_Fractals[i]);
		SDL_SetTextureBlendMode(Texture_Fractals[i], SDL_BLENDMODE_BLEND);
		Texture_Fractals_Monkey[i] = SDL_CreateTextureFromSurface(renderer, Surface_Fractals[i]);
		SDL_SetTextureBlendMode(Texture_Fractals_Monkey[i], SDL_BLENDMODE_BLEND);
        }
}


