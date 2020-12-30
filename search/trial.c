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


#include "picture.h"
#include <stdio.h>
#include "../coding.h"

SDL_Texture *content2Texture;
SDL_Texture *content2TextureRect;
SDL_Rect gazeRect[100];
float SDL_LastRefresh = 0;
float RefreshInterval = 1000/60.0;

UINT32 trialStateStartTime;
experimentStateEnum experimentState;
trialStatesEnum trialState;
statePhaseEnum statePhase;
monkeyGazeStateEnum monkeyGazeState;
monkeyFractGazeStateEnum monkeyFractGazeState;
gazeFixResultEnum gazeFixResult;
gazeFixResultEnum gazeFractFixResult;
monkeyFractChoiceEnum monkeyFractChoice;

int currentNumberOfTrials;
int currentNumberOfErroneousTrials;
int currentNumberOfTargetPresentTrials;
int currentNumberOfTargetAbsentTrials;
int currentNumberOfSkipsInTargetAbsentTrials;
int currentNumberOfSkipsInTargetPresentTrials;
int currentNumberOfCorrectChoicesInTargetPresentTrials;
int currentDispSize3;
int currentDispSize5;
int currentDispSize7;
int currentDispSize9;
int currentDispSize3Correct;
int currentDispSize5Correct;
int currentDispSize7Correct;
int currentDispSize9Correct;

double ANGLE;

int photodiodeAck = 0;
int initedPhotodiodeAck = 0;
int sentEventToNeuralData = 0;
int isPhotodiodeCheckOn = 1;
int isNextFramePreparedMonkey;
int isNextFrameDrawnMonkey;
int reloadedConf;
int isPaused;
int pauseTime;
int mycounter;
int gazeXOffset = 0;
int gazeYOffset = 0;
/***************************** PERFORM AN EXPERIMENTAL TRIAL  ***************/

/* End recording: adds 100 msec of data to catch final events */

UINT32 getTime(){
	if (isPaused)
		return pauseTime;
	return current_time();
}

void pauseExperiment(){
	isPaused = 1;
	pauseTime = current_time();	
	printf("\n\nPAUSED!\n\n");
}

void unpauseExperiment(){
	isPaused = 0;
	printf("\n\nUNPAUSED!\n\n");
}

static void end_trial(void)
{
	clear_full_screen_window(target_background_color);    /* hide display */
	end_realtime_mode();   /* NEW: ensure we release realtime lock */
	pump_delay(100);       /* CHANGED: allow Windows to clean up   */
						   /* while we record additional 100 msec of data */
	stop_recording();
}
int simple_recording_trial()
{
	UINT32 trial_start;	/* trial start time (for timeout)  */
	UINT32 display_time;  /* retrace-to-draw delay */
	int button;		/* the button pressed (0 if timeout)  */
	int error;            /* trial result code */
	SDL_Event e;
	SDL_Texture *bit;
	SDL_Texture *bit2;
	SDL_Surface *hbm = SDL_CreateRGBSurface(SDL_SWSURFACE,SCRWIDTH, SCRHEIGHT, dispinfo.bits,0,0,0,0);
		int frameCount = 0;
		int contentUpdateCountre = 0;
		int contentUpdateCountreRect = 0;
	unsigned short currGazeIdx = 0;
	currentSequenceNumber = 0;
	currentBatchSequenceNumber = 0;
/*
	SDL_Rect testRect;
	testRect.x = 20;
	testRect.y = 20;
	testRect.w = 30;
	testRect.h = 30;
*/
	eye_used = -1;		/* indicates which eye's data to display*/
	float x,y;

	/* 
		NOTE: TRIALID AND TITLE MUST HAVE BEEN SET BEFORE DRIFT CORRECTION!
		FAILURE TO INCLUDE THESE MAY CAUSE INCOMPATIBILITIES WITH ANALYSIS 
		SOFTWARE!
	 */

/*
	eyecmd_printf("link_event_filter = LEFT,RIGHT,FIXUPDATE");
	eyecmd_printf("fixation_update_interval = %d", DWELL_FIXATION_UPDATE_INTERVAL);
	eyecmd_printf("fixation_update_accumulate = %d", DWELL_FIXATION_UPDATE_ACCUMULATE);
*/
	while(1)
	{              /*Check link often so we can exit if tracker stopped */
	  if(!eyelink_is_connected()) return ABORT_EXPT;
	   /* 
			We let do_drift_correct() draw target in this example
			3rd argument would be 0 if we already drew the fixation target
		*/
	  error = do_drift_correct((UINT16)(SCRWIDTH/2), (UINT16)(SCRHEIGHT/2), 1, 1);
		   /* repeat if ESC was pressed to access Setup menu  */
	  if(error!=27) break;
	}


  set_offline_mode();
  pump_delay(50);

	error = start_recording(1,1,1,1);
	if(error != 0) return error;/* ERROR: couldn't start recording*/

	if(!eyelink_wait_for_block_start(100, 0, 1))  /* wait for link event data*/
	{
	  end_trial();
	  alert_printf("ERROR: No link events received!");
	  return TRIAL_ERROR;
	}
	eye_used = eyelink_eye_available(); /* determine which eye(s) are available*/ 
	switch(eye_used)		      /* select eye, add annotation to EDF file	*/
	{			
	  case RIGHT_EYE:
		eyemsg_printf("EYE_USED 1 RIGHT");
		break;
	  case BINOCULAR:           /* both eye's data present: use left eye only*/
		eye_used = LEFT_EYE;
	  case LEFT_EYE:
		eyemsg_printf("EYE_USED 0 LEFT");
		break;
	}
	   /* 
		  DO PRE-TRIAL DRIFT CORRECTION 
		  We repeat if ESC key pressed to do setup. 
		*/
	/* make sure display is blank */
	clear_full_screen_window(target_background_color);    /* hide display */

	  //ensure the eye tracker has enough time to switch modes (to start recording).

	/* 
		Start data recording to EDF file, BEFORE DISPLAYING STIMULUS 
		You should always start recording 50-100 msec before required
		otherwise you may lose a few msec of data 
	 */
	/*record samples and events to EDF file only*/

	/* record for 100 msec before displaying stimulus  */
	/* Windows 2000/XP: no interruptions till display start marked */
	begin_realtime_mode(100);   

	/*	DISPLAY OUR IMAGE TO SUBJECT  by copying bitmap to display
		Because of faster drawing speeds and good refresh locking,
		we now place the stimulus onset message just after display refresh 
		and before drawing the stimulus.  This is accurate and will allow 
		drawing a new stimulus each display refresh.
		However, we do NOT send the message after the retrace--this may take
		too long instead, we add a number to the message that represents 
		the delay  from the event to the message in msec
	*/
	SDL_RenderSetScale(renderer2, 1/renderer2ScaleX, 1/renderer2ScaleY); //Parameterize
	trial_start = current_msec();
	pump_delay(100);
	//if(screen->flags & SDL_HWSURFACE) // obsolete
	//	SDL_BlitSurface(gbm, NULL, screen, NULL);  /* to the background. for hwsurfaces */
	 display_time = current_msec() - trial_start;
	 /* message marks zero-plot time for EDFVIEW */
	 eyemsg_printf("%d DISPLAY ON", display_time);  /* message for RT recording in analysis */
	 eyemsg_printf("SYNCTIME %d", display_time);  

  
	trial_start = current_msec();

	/* we would stay in realtime mode if timing is critical   
	   for example, if a dynamic (changing) stimulus was used
	   or if display duration accuracy of 1 video refresh. was needed
	   we don't care as much about time now, allow keyboard to work
	 */
	end_realtime_mode();  
	
	/* Now get ready for trial loop */
	eyelink_flush_keybuttons(0);   /* reset keys and buttons from tracker */

	gotoExperimentState(Experiment_State_Init);
	monkeyGazeState = Monkey_Gaze_State_HasNotLookedInsideFixationWindow;
	monkeyFractGazeState = Monkey_Fract_Gaze_State_Hasnt_Left_Fixation_Window;
	gazeFixResult = Gaze_Fix_Result_Undetermined;
	gazeFractFixResult = Gaze_Fract_Fix_Result_Undetermined;
	monkeyFractChoice = Monkey_Fract_Choice_Undetermined;
	SDL_SetRenderTarget(renderer,NULL);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 255);
	SDL_RenderClear(renderer);
	
	isNextFramePreparedMonkey = 0;
	isNextFrameDrawnMonkey = 0;
	currentNumberOfOutOfRegionSamples = 0;
	reloadedConf = 0;
	Region_Last = -1;
	currentRegionStartTime = getTime();
	currentGoodFractalQueuePos = 0;
	currentBadFractalQueuePos = 0;
	drawFractalsMonkey = 0;
	mycounter = 0;
	isPaused = 0;
	Region_Choice_Idx = -1;
	lastTrialRunResult = Trial_Run_Result_Undetermined;
	trialRunResult = Trial_Run_Result_Undetermined;
	currentNumberOfTrials = 0;
	currentNumberOfErroneousTrials = 0;
	currentNumberOfTargetPresentTrials = 0;
	currentNumberOfTargetAbsentTrials = 0;
	currentNumberOfSkipsInTargetAbsentTrials = 0;
	currentNumberOfSkipsInTargetPresentTrials = 0;
	currentNumberOfCorrectChoicesInTargetPresentTrials = 0;
	currentDispSize3 = 0;
	currentDispSize5 = 0;
	currentDispSize7 = 0;
	currentDispSize9 = 0;
	currentDispSize3Correct = 0;
	currentDispSize5Correct = 0;
	currentDispSize7Correct = 0;
	currentDispSize9Correct = 0;

	
	//drawGrid();

   /* 
	  we don't use getkey() especially in a time-critical trial
      as operating system may interrupt us and cause an unpredicatable delay
      so we would use buttons or tracker keys only  
	*/
	//}

	 /* Trial loop: till timeout or response  */
	while(1)   
	{ 
	  /* First, check if recording aborted  */
	  if((error=check_recording())!=0) return error; 
	  /* Check if trial time limit expired */
	  /*
	  if(getTime() > trial_start+20000L)
		{
			eyemsg_printf("TIMEOUT");// message to log the timeout 
			end_trial();             // local function to stop recording
			button = 0;              // trial result message is 0 if timeout 
			break;                   // exit trial loop
		}
	*/
	  /*check for program termination or ALT-F4 or CTRL-C keys*/
	  /*
	  if (currentSequenceNumber >= numberOfFractals - 1){
		end_trial();
		break;
	  }*/	  

	  if(break_pressed())
		{
			end_trial();         /* local function to stop recording */
			return ABORT_EXPT;   /* return this code to terminate experiment */
		}
	  if (currentNumberOfTrials >= 240){
			end_trial();
			return TRIAL_OK;
	  }
	  /* check for local ESC key to abort trial (useful in debugging)   */
	  if(escape_pressed())    
		{
			end_trial();         /* local function to stop recording */
			return SKIP_TRIAL;   /* return this code if trial terminated */
		}
	  if (eyelink_newest_float_sample(NULL)>0){
		eyelink_newest_float_sample(&evt); //Get sample
		setGaze(); 
		drawGaze();
		processGaze();

	  }
	  /*
	if (SDL_PollEvent(&e) && e.type == SDL_KEYDOWN){
		switch (e.key.keysym.sym){
			case SDLK_p:{ 
				pauseExperiment();
				break;
			}
			case SDLK_u:{
				unpauseExperiment();
				break;
			}
		}
      	}
	*/
	  if (experimentState == Experiment_State_Init){
		  

		gotoExperimentState(Experiment_State_Prep);	
		gotoState(Trial_State_ITI);
		gotoStatePhase(State_Phase_Enter);
	  }// End of Experiment Init

	  else if (experimentState == Experiment_State_Prep){

		  if (currentBatchSequenceNumber%numberOfBatchFractals == 0) // Also check for reset upon error
		  {
			  currentBatchSequenceNumber = 0;
		  	printf("Reached the beginning of new experiment batch");
		  	eyemsg_printf("Reached the beginning of new experiment batch");
			//randomNumberExperimentType = rand()%numberOfBatchExperiments;

			trialTypePerm[numberOfBatchFractals]; // target present or absent
			randGen(36, trialTypePerm, Random_Permutation_Type_Trial_Type_Targert_Wise);

			fractalNumPerm[numberOfBatchFractals]; //3, 5, 7, 9 fractals on trial
			randGen(36, fractalNumPerm, Random_Permutation_Type_Fractal_Number);

		  	//Select the region to put the good fractal
		  	//currentGoodFractalRegion = rand() % currentNumOfFractals;
		/*	
			printf("ispresent/current good/all fracts\n");
			eyemsg_printf("ispresent/current good/all fracts\n");
			for (int i=0; i<numberOfBatchFractals; i++){
				if (trialTypePerm[i] == 0)
					printf("i = %d : absent\n", i);
					eyemsg_printf("i = %d : absent\n", i);
				else
					printf("i = %d : present, %d/%d\n",i ,goodFractRegionPerm[i], fractalNumPerm[i]);
					eyemsg_printf("i = %d : present, %d/%d\n",i ,goodFractRegionPerm[i], fractalNumPerm[i]);
			}
			*/
		  }
		  for (int i=0; i<Search_Event_Len; i++){
			searchEvents[i].hasExpired = 0;
			searchEvents[i].hasActed = 0;
			searchEvents[i].canBeExpired = 0;

		  }
		  for (int i=0; i<Trial_State_Len; i++){
			trialStates[i].hasExpired = 0;
		  }
		  searchEvents[Search_Event_Reject_Before_Exiting_Fixation].canBeExpired = 1;
		  currentTrialType = trialTypePerm[currentBatchSequenceNumber];
		  currentNumOfFractals = fractalNumPerm[currentBatchSequenceNumber]; 
		  setRegions();
		  eraseSensationWindows();
		  //Select the region to put the good fractal
		  currentGoodFractalRegion = (rand() % currentNumOfFractals) + 1;
		  printf("current batch num = %d", currentBatchSequenceNumber);
		  eyemsg_printf("current batch num = %d", currentBatchSequenceNumber);
		  if (currentTrialType == 0){
			  printf("\n\n\n Launching Target absent trial\n\n\n");
			  eyemsg_printf("\n\n\n Launching Target absent trial\n\n\n");
		  }
		  else{
			  printf("\n\n\n Launching Target present trial\n\n\n");
			  eyemsg_printf("\n\n\n Launching Target present trial\n\n\n");
			  printf("Current good fract reg = %d/%d\n\n", currentGoodFractalRegion,currentNumOfFractals);
			  eyemsg_printf("Current good fract reg = %d/%d\n\n", currentGoodFractalRegion,currentNumOfFractals);
		  
		  }
		  placeFracts();
		  drawGrid();
		  Region_Choice_Idx = -1;
		  setITI();
		  writeConfigToEyelink();

		  gotoExperimentState(Experiment_State_Run);
	  }

	  else if (experimentState == Experiment_State_Run){
		if (trialState == Trial_State_ITI){
			if (statePhase == State_Phase_Enter){
				printf("ITI Enter\n");
				eyemsg_printf("ITI Enter\n");
				sendEventToNeuralData(CODES_SEARCH_TRIAL_START);
				updateFrameMonkey();
				eraseFixation();
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					eraseSensationWindows();
					drawNextFrameMonkey(Trial_State_Wait_For_Gaze);
					prepareFrameMonkey();
				}
				if (!reloadedConf){
					reloadConf();
				}
				//printf("ITI In\n");
				//eyemsg_printf("ITI In\n");
				if (timedOut()){
					gotoStatePhase(State_Phase_Exit);
				}	
			}
			else if (statePhase == State_Phase_Exit){
				setNextFrameMonkeyUndrawn();
				setNextFrameMonkeyUnprepared();
				printf("ITI Exit\n");
				eyemsg_printf("ITI Exit\n");
				gotoState(Trial_State_Wait_For_Gaze);
			}
		}// End of Trial_State_ITI
		else if (trialState == Trial_State_Wait_For_Gaze){
			if (statePhase == State_Phase_Enter){
				printf("Wait Enter\n");
				eyemsg_printf("Wait Enter\n");
				updateFrameMonkey();
				drawFixation();
				drawFixationWindow(Color_Fixation_Window_Neutral);
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				//printf("Wait In\n");
				//eyemsg_printf("Wait In\n");
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					drawNextFrameMonkey(Trial_State_Fixation_Fix);
					prepareFrameMonkey();
				}
				if (hasLookedIntoFixationWindow()){
					gotoStatePhase(State_Phase_Exit);
				}
				else if (timedOut()){
					gotoStatePhase(State_Phase_Exit);
				}	

			}
			else if (statePhase == State_Phase_Exit){
				setNextFrameMonkeyUndrawn();
				setNextFrameMonkeyUnprepared();
				printf("Wait Out\n");
				eyemsg_printf("Wait Out\n");
				if (hasLookedIntoFixationWindow()){
					printf("Looked!\n");
					eyemsg_printf("Looked!\n");
					gotoState(Trial_State_Fixation_Fix);
				}
				else{
					gotoState(Trial_State_Result);
				}
			}
		}// End of Trial_State_Wait_For_Gaze

		else if (trialState == Trial_State_Fixation_Fix){
			if (statePhase == State_Phase_Enter){
				updateFrameMonkey();
				eraseFracts();
				printf("FractFix Enter\n");
				eyemsg_printf("FractFix Enter\n");
				drawFixationWindow(Color_Fixation_Window_Hold);
				//drawSensationWindowGood();
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				//printf("Fix In\n");
				//eyemsg_printf("Fix In\n");
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
				//if (!isNextFrameDrawnMonkey){
					printf("Making Next Search frame\n");
					eyemsg_printf("Making Next Search frame\n");
					drawNextFrameMonkey(Trial_State_Search);
					prepareFrameMonkey();
				}
				if (hasBrokenFixation()){
					printf("Got out for break");
					eyemsg_printf("Got out for break");
					gotoStatePhase(State_Phase_Exit);
				}	
				if (timedOut()){
					printf("Got out for timeout");
					eyemsg_printf("Got out for timeout");
					gotoStatePhase(State_Phase_Exit);
				}
			}
			else if (statePhase == State_Phase_Exit){
				setNextFrameMonkeyUndrawn();
				setNextFrameMonkeyUnprepared();
				printf("Fractix Out\n");
				eyemsg_printf("Fractix Out\n");
				if (!hasBrokenFixation()){
					gotoState(Trial_State_Search);
				}
				else if (hasBrokenFixation()){
					gotoState(Trial_State_Result);
				}
			}

		}// End of Trial_State_Fract_Off

		else if (trialState == Trial_State_Search){

			if (statePhase == State_Phase_Enter){
		  		drawSensationWindowBad();
				updateFrameMonkey();
				if (currentTrialType == 1)
					drawSensationWindowGood();
				drawFracts();
				searchEvents[Search_Event_Bring_Back_Fixation].startTime = getTime();
		  		searchEvents[Search_Event_Bring_Back_Fixation].canBeExpired = 1;
				eraseFixation();
				drawFixationBringBackWindow(Color_Fixation_Search_Window);
				printf("Search Enter\n");
				eyemsg_printf("Search Enter\n");
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){

				if (isPhotodiodeCheckOn && !gotPhotodiodeAck()){
                    //printf("inside photodiode check on\n");
                    if (!initedPhotodiodeAck){
                        initgetPhotodiodeAck();
                    }
                    //printf("got back from init ack photodiode\n");
                    while (!gotPhotodiodeAck() && !timedOut()){
                        //getPhotodiodeAckAndSendEvent(CODES_FORCE_STIM_ONSET[getRegion(randomSequenseNumbers[currentSequenceNumber])]);
                    //printf("inside while");
                        getPhotodiodeAck();
                    }
                    //printf("\noutside while\n");
                    if (gotPhotodiodeAck()){
					double offset = Regions_Offset + ANGLE/2; 
                     if (currentNumOfFractals == 3){
						int offsetnum = offset/(0.349066); // 20/180*PI 
												 //CHECK IF DONE IN DOUBLE
						printf("TYPE = %d, DS = %d, TR = %d,OFFSET = %f, OFFSETNUM = %d, CODE = %S\n", currentTrialType, currentNumOfFractals, currentGoodFractalRegion, offset, offsetnum, CODES_SEARCH_DS3_STIM_ONSET[currentTrialType*6*currentGoodFractalRegion+offsetnum]);
						sendEventToNeuralData(CODES_SEARCH_DS3_STIM_ONSET[currentTrialType*6*currentGoodFractalRegion+offsetnum]);
						eyemsg_printf("TAG: %s", CODES_SEARCH_DS3_STIM_ONSET[currentTrialType*6*currentGoodFractalRegion+offsetnum]);
					}
					else if (currentNumOfFractals == 5){
						int offsetnum = offset/(0.314159); // 18/180*PI
												 //CHECK IF DONE IN DOUBLE
						printf("TYPE = %d, DS = %d, TR = %d,OFFSET = %f, OFFSETNUM = %d, CODE = %S\n", currentTrialType, currentNumOfFractals, currentGoodFractalRegion-1, offset, offsetnum, CODES_SEARCH_DS5_STIM_ONSET[currentTrialType*4*currentGoodFractalRegion+offsetnum]);
						sendEventToNeuralData(CODES_SEARCH_DS5_STIM_ONSET[currentTrialType*4*currentGoodFractalRegion+offsetnum]);
						eyemsg_printf("TAG: %s", CODES_SEARCH_DS5_STIM_ONSET[currentTrialType*4*currentGoodFractalRegion+offsetnum]);
					}
					else if (currentNumOfFractals == 7){
						int offsetnum = offset/(0.299199); // (360/(7*3)) ~= 17.142 /180*PI
												 //CHECK IF DONE IN DOUBLE
						printf("TYPE = %d, DS = %d, TR = %d,OFFSET = %f, OFFSETNUM = %d, CODE = %S\n", currentTrialType, currentNumOfFractals, currentGoodFractalRegion-1, offset, offsetnum, CODES_SEARCH_DS7_STIM_ONSET[currentTrialType*3*currentGoodFractalRegion+offsetnum]);
						sendEventToNeuralData(CODES_SEARCH_DS7_STIM_ONSET[currentTrialType*3*currentGoodFractalRegion+offsetnum]);
						eyemsg_printf("TAG: %s", CODES_SEARCH_DS7_STIM_ONSET[currentTrialType*3*currentGoodFractalRegion+offsetnum]);
					}
					else if (currentNumOfFractals == 9){
						int offsetnum = offset/(0.349066); // (360/(9*2)) = 20 /180*PI
						printf("TYPE = %d, DS = %d, TR = %d,OFFSET = %f, OFFSETNUM = %d, CODE = %S\n", currentTrialType, currentNumOfFractals, currentGoodFractalRegion-1, offset, offsetnum, CODES_SEARCH_DS9_STIM_ONSET[currentTrialType*2*currentGoodFractalRegion+offsetnum]);
						sendEventToNeuralData(CODES_SEARCH_DS9_STIM_ONSET[currentTrialType*2*currentGoodFractalRegion+offsetnum]);
						eyemsg_printf("TAG: %s", CODES_SEARCH_DS9_STIM_ONSET[currentTrialType*2*currentGoodFractalRegion+offsetnum]);
					}
                    stopPollingForPhotodiodeAck();
                    }
                }
				processSearchTimes();
				actSearchTimes();	
			}
			else if (statePhase == State_Phase_Exit){
				printf("Search Out\n");
				eyemsg_printf("Search Out\n");
				if (isPhotodiodeCheckOn && !gotPhotodiodeAck()){
                        printf("ERROR! FAILED TO GET PHOTODIODE SIGNAL");
                        writeToEyelink("ERROR! FAILED TO GET PHOTODIODE SIGNAL");
				}
				setNextFrameMonkeyUndrawn();
				setNextFrameMonkeyUnprepared();
				if (hasCommittedOnAFract())
					gotoState(Trial_State_Fractal_Fix);
				else 
					gotoState(Trial_State_Result);
			}

		}
			/*
		else if (trialState == Trial_State_Fract_Overlap){
			if (statePhase == State_Phase_Enter){
				updateFrameMonkey();
				printf("FractOverlap Enter\n");
				eyemsg_printf("FractOverlap Enter\n");
				drawFixationWindow(Color_Fixation_Window_Hold);
				drawSensationWindowGood();
				drawSensationWindowBad();
				drawFracts();
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				//printf("Fix In\n");
				//eyemsg_printf("Fix In\n");
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					printf("Making Next Off frame\n");
					eyemsg_printf("Making Next Off frame\n");
					//if numoffract reached iti else off
					drawNextFrameMonkey(Trial_State_Wait_For_Saccade);
					//isNextFrameDrawnMonkey = 1;
					prepareFrameMonkey();
					//isNextFramePreparedMonkey = 1;
				}
				if (hasBrokenFixation()){
					printf("Got out for break");
					eyemsg_printf("Got out for break");
				gotoStatePhase(State_Phase_Exit);
				}	
				if (timedOut()){
					printf("Got out for timeout");
					eyemsg_printf("Got out for timeout");
					gotoStatePhase(State_Phase_Exit);
				}
			}
			else if (statePhase == State_Phase_Exit){
				//prepareFrameMonkey();
				setNextFrameMonkeyUndrawn();
				setNextFrameMonkeyUnprepared();
				printf("FractOn Out\n");
				eyemsg_printf("FractOn Out\n");
				if (!hasBrokenFixation()){
					setFixationSuccess();
					gotoState(Trial_State_Wait_For_Saccade);
				}
				else if (hasBrokenFixation()){
					gotoState(Trial_State_Result);
				}
			}
		
		}// End of Trial_State_Fract_Off
		else if (trialState == Trial_State_Wait_For_Saccade){
			if (statePhase == State_Phase_Enter){
				updateFrameMonkey();
				printf("WaitSaccade Enter\n");
				eyemsg_printf("WaitSaccade Enter\n");
				eraseFixation();
				drawSensationWindowGood();
				drawSensationWindowBad();
				drawFracts();
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					printf("Making Next Off frame\n");
					eyemsg_printf("Making Next Off frame\n");
					//if numoffract reached iti else off
					drawNextFrameMonkey(Trial_State_Fractal_Fix);
					//isNextFrameDrawnMonkey = 1;
					prepareFrameMonkey();
					//isNextFramePreparedMonkey = 1;
				}
				if (hasLookedIntoSensationWindow()){
					printf("Got out for saccade");
					eyemsg_printf("Got out for saccade");
					gotoStatePhase(State_Phase_Exit);
				}	
				if (timedOut()){
					printf("Got out for timeout");
					eyemsg_printf("Got out for timeout");
					gotoStatePhase(State_Phase_Exit);
				}
			}
			else if (statePhase == State_Phase_Exit){
				setNextFrameMonkeyUndrawn();
				setNextFrameMonkeyUnprepared();
				printf("Wait Out\n");
				eyemsg_printf("Wait Out\n");
				if (hasLookedIntoSensationWindow()){
					printf("Looked Sensation!\n");
					eyemsg_printf("Looked Sensation!\n");
					gotoState(Trial_State_Fractal_Fix);
				}
				else{
					gotoState(Trial_State_Result);
				}
			}
		}
*/
		else if (trialState == Trial_State_Fractal_Fix){
			if (statePhase == State_Phase_Enter){
				//updateFrameMonkey();
				printf("FractalFix Enter\n");
				eyemsg_printf("FractalFix Enter\n");
				//eraseFixation();
				//drawSensationWindowGood();
				//drawSensationWindowBad();
				//drawFracts();
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				//printf("Fix In\n");
				//eyemsg_printf("Fix In\n");
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
				//if (!isNextFrameDrawnMonkey){
					drawNextFrameMonkey(Trial_State_ITI);
					prepareFrameMonkey();
				}
				if (hasBrokenFractFixation()){
					printf("Got out for FractFix break");
					eyemsg_printf("Got out for FractFix break");
					gotoStatePhase(State_Phase_Exit);
				}	
				if (timedOut()){
					printf("Got out for timeout");
					eyemsg_printf("Got out for timeout");
					setFractFixationSuccess();
					if (currentTrialType == 1 && Region_Choice_Idx == currentGoodFractalRegion){
						trialRunResult = Trial_Run_Result_Fixed_On_Good;
						printf("\n\n\n=== Result === Fixed on good fractal\n\n\n");
						eyemsg_printf("\n\n\n=== Result === Fixed on good fractal\n\n\n");
					}
					else{
						trialRunResult = Trial_Run_Result_Fixed_On_Bad;
						printf("\n\n\n === Result === Fixed on bad fractal\n\n\n");
						eyemsg_printf("\n\n\n === Result === Fixed on bad fractal\n\n\n");
					}
					gotoStatePhase(State_Phase_Exit);
				}
			}
			else if (statePhase == State_Phase_Exit){
				setNextFrameMonkeyUndrawn();
				setNextFrameMonkeyUnprepared();
				printf("FractFix Exit\n");
				eyemsg_printf("FractFix Exit\n");
				if (!hasBrokenFractFixation()){
					printf("Kept from exit");
					eyemsg_printf("Kept from exit");
					setFractFixationSuccess();
					gotoState(Trial_State_Result);
				}
				else if (hasBrokenFractFixation()){
					printf("Gaze Result From Fix Exit: %s\n", (gazeFractFixResult == Gaze_Fract_Fix_Result_KeptGaze) ? "Kept" : ((gazeFractFixResult == Gaze_Fract_Fix_Result_Broke) ? "Broke" : "Undet"));
					eyemsg_printf("Gaze Result From Fix Exit: %s\n", (gazeFractFixResult == Gaze_Fract_Fix_Result_KeptGaze) ? "Kept" : ((gazeFractFixResult == Gaze_Fract_Fix_Result_Broke) ? "Broke" : "Undet"));
					printf("Broke from exit");
					eyemsg_printf("Broke from exit");
					gotoState(Trial_State_Result);
				}
			}
		}

		else if (trialState == Trial_State_Result){
			if (statePhase == State_Phase_Enter){
				updateFrameMonkey();
				printf("Result Enter\n");
				eyemsg_printf("Result Enter\n");
				if (currentNumOfFractals == 3)
					currentDispSize3++;
				else if (currentNumOfFractals == 5)
					currentDispSize5++;
				else if (currentNumOfFractals == 7)
					currentDispSize7++;
				else if (currentNumOfFractals == 9)
					currentDispSize9++;


				//printf("Gaze Result From Result Enter: %s\n", (gazeFractFixResult == Gaze_Fract_Fix_Result_KeptGaze) ? "Kept" : ((gazeFractFixResult == Gaze_Fract_Fix_Result_Broke) ? "Broke" : "Undet"));
				//eyemsg_printf("Gaze Result From Result Enter: %s\n", (gazeFractFixResult == Gaze_Fract_Fix_Result_KeptGaze) ? "Kept" : ((gazeFractFixResult == Gaze_Fract_Fix_Result_Broke) ? "Broke" : "Undet"));
				if (hasKeptFractFixaton()){
					system("play -q -v 0.5 Success.wav &");
					currentNumberOfTrials++;
					if (currentTrialType == 0)
						currentNumberOfTargetAbsentTrials++;
					else if (currentTrialType == 1)
						currentNumberOfTargetPresentTrials++;
					drawFixationWindow(Color_Fixation_Window_Success);
					if (hasFixedOnBadFract()){
						sendSmallReward();
						sendEventToNeuralData(CODES_REWARD_SMALL);
						printf("Giving Bad Fract Reward\n");
						eyemsg_printf("Giving Bad Fract Reward\n");
					}
					else if (hasFixedOnGoodFract()){
						sendBigReward();
						sendEventToNeuralData(CODES_REWARD_LARGE);
						printf("Giving Good Fract Reward\n");
						eyemsg_printf("Giving Good Fract Reward\n");
						currentNumberOfCorrectChoicesInTargetPresentTrials++;
						if (currentNumOfFractals == 3)
							currentDispSize3Correct++;
						else if (currentNumOfFractals == 5)
							currentDispSize5Correct++;
						else if (currentNumOfFractals == 7)
							currentDispSize7Correct++;
						else if (currentNumOfFractals == 9)
							currentDispSize9Correct++;


					}
					else if (hasRejectedTrialBeforeExitingSearchFixationWindow()){
						int myRand = rand()%100;
						printf("Random number is %d %s %d\n", myRand,
						(myRand < Reward_Skip_Probability) ? "<" : 
						((myRand == Reward_Skip_Probability) ? "=" : ">"),Reward_Skip_Probability);
						eyemsg_printf("Random number is %d %s %d\n", myRand,
						(myRand < Reward_Skip_Probability) ? "<" : 
						((myRand == Reward_Skip_Probability) ? "=" : ">"),Reward_Skip_Probability);
						if (myRand <= Reward_Skip_Probability){
							/*char rwd[10];
							char dur[5];
							sprintf(dur, "%d", Reward_Skip_Duration);
							strcpy(rwd, "RD");
							strcat(rwd, dur);
							strcat(rwd, "\n");
							printf("Dispensing probabilistic reward for %d ms\n", Reward_Skip_Duration);
							eyemsg_printf("Dispensing probabilistic reward for %d ms\n", Reward_Skip_Duration);
						
							write(fd, rwd, sizeof(rwd));*/
							sendMediumReward();
							sendEventToNeuralData(CODES_REWARD_MEDIUM);
						}

						if (currentTrialType == 0){
							currentNumberOfSkipsInTargetAbsentTrials++;
							if (currentNumOfFractals == 3)
								currentDispSize3Correct++;
							else if (currentNumOfFractals == 5)
								currentDispSize5Correct++;
							else if (currentNumOfFractals == 7)
								currentDispSize7Correct++;
							else if (currentNumOfFractals == 9)
								currentDispSize9Correct++;
						}
						else if (currentTrialType == 1){
							currentNumberOfSkipsInTargetPresentTrials++;
						}
					


					}
					else if (hasRejectedTrialAfterExitingSearchFixationWindow()){
						int myRand = rand()%100;
						printf("Random number is %d %s %d\n", myRand,
						(myRand < Reward_Skip_Probability) ? "<" : 
						((myRand == Reward_Skip_Probability) ? "=" : ">"),Reward_Skip_Probability);
						eyemsg_printf("Random number is %d %s %d\n", myRand,
						(myRand < Reward_Skip_Probability) ? "<" : 
						((myRand == Reward_Skip_Probability) ? "=" : ">"),Reward_Skip_Probability);
						if (myRand <= Reward_Skip_Probability){
							char rwd[10];
							char dur[5];
							sprintf(dur, "%d", Reward_Skip_Duration);
							strcpy(rwd, "RD");
							strcat(rwd, dur);
							strcat(rwd, "\n");
							printf("Dispensing probabilistic reward for %d ms\n", Reward_Skip_Duration);
							eyemsg_printf("Dispensing probabilistic reward for %d ms\n", Reward_Skip_Duration);
							write(fd, rwd, sizeof(rwd));
							//write(Reward_Skip_Duration);
						
						}
						if (currentTrialType == 0){
							currentNumberOfSkipsInTargetAbsentTrials++;
							if (currentNumOfFractals == 3)
								currentDispSize3Correct++;
							else if (currentNumOfFractals == 5)
								currentDispSize5Correct++;
							else if (currentNumOfFractals == 7)
								currentDispSize7Correct++;
							else if (currentNumOfFractals == 9)
								currentDispSize9Correct++;
						}
						else if (currentTrialType == 1){
							currentNumberOfSkipsInTargetPresentTrials++;
						}
					
					}
					//sound
					//reward
					//eyelink event
					//blackrock event
				}
				else if (!hasKeptFractFixaton()){
					currentNumberOfErroneousTrials++;
					drawFixationWindow(Color_Fixation_Window_Error);
					system("play -q -v 0.4 Error.wav &");
					//sound
					//eyelink event
					//blackrock event
				}
				gotoStatePhase(State_Phase_Inside);

			}
			else if (statePhase == State_Phase_Inside){
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					//if numoffract reached iti else off
					drawNextFrameMonkey(Trial_State_ITI);
					prepareFrameMonkey();
				}
				gotoStatePhase(State_Phase_Exit);

			}
			else if (statePhase == State_Phase_Exit){
				printf("Result Out");
				eyemsg_printf("Result Out");
				printf("\n========== STATS =========\n");
				eyemsg_printf("\n========== STATS =========\n");
				printf("Current Number Of Erroneous Trials: %d\n",
					       	currentNumberOfErroneousTrials);
				eyemsg_printf("Current Number Of Erroneous Trials: %d\n",
					       	currentNumberOfErroneousTrials);
				printf("Current Number Of Non Erroneous Trials: %d\n",
					       	currentNumberOfTrials);
				eyemsg_printf("Current Number Of Non Erroneous Trials: %d\n",
					       	currentNumberOfTrials);
				printf("Current Number Of Non Erroneous Target Present Trials: %d/%d\n",
					       	currentNumberOfTargetPresentTrials,
					       	currentNumberOfTrials);
				eyemsg_printf("Current Number Of Non Erroneous Target Present Trials: %d/%d\n",
					       	currentNumberOfTargetPresentTrials,
					       	currentNumberOfTrials);
				printf("Current Number Of Skips In Non Erroneous Target Absent Trials: %d/%d\n",
					       	currentNumberOfSkipsInTargetAbsentTrials,
					       	currentNumberOfTargetAbsentTrials);
				eyemsg_printf("Current Number Of Skips In Non Erroneous Target Absent Trials: %d/%d\n",
					       	currentNumberOfSkipsInTargetAbsentTrials,
					       	currentNumberOfTargetAbsentTrials);
				printf("Current Number Of Skips In Non Erroneous Target Present Trials: %d/%d\n",
					       	currentNumberOfSkipsInTargetPresentTrials,
					       	currentNumberOfTargetPresentTrials);

				eyemsg_printf("Current Number Of Skips In Non Erroneous Target Present Trials: %d/%d\n",
					       	currentNumberOfSkipsInTargetPresentTrials,
					       	currentNumberOfTargetPresentTrials);
				printf("Current Number Of Correct Choices in Non Erroneous Target Present Trials: %d/%d\n",
					       	currentNumberOfCorrectChoicesInTargetPresentTrials, 
						currentNumberOfTargetPresentTrials);
				
				eyemsg_printf("Current Number Of Correct Choices in Non Erroneous Target Present Trials: %d/%d\n",
					       	currentNumberOfCorrectChoicesInTargetPresentTrials, 
						currentNumberOfTargetPresentTrials);
				printf("Display Size #3: %d/%d\n",
					       	currentDispSize3Correct, 
						currentDispSize3);
				eyemsg_printf("Display Size #3: %d/%d\n",
					       	currentDispSize3Correct, 
						currentDispSize3);
				printf("Display Size #5: %d/%d\n",
					       	currentDispSize5Correct, 
						currentDispSize5);
				eyemsg_printf("Display Size #5: %d/%d\n",
					       	currentDispSize5Correct, 
						currentDispSize5);
				printf("Display Size #7: %d/%d\n",
					       	currentDispSize7Correct, 
						currentDispSize7);
				eyemsg_printf("Display Size #7: %d/%d\n",
					       	currentDispSize7Correct, 
						currentDispSize7);
				printf("Display Size #9: %d/%d\n",
					       	currentDispSize9Correct, 
						currentDispSize9);
				eyemsg_printf("Display Size #9: %d/%d\n",
					       	currentDispSize9Correct, 
						currentDispSize9);



				printf("========== STATS =========");
				eyemsg_printf("========== STATS =========");
				printf("\n\n");
                                eraseFixation();
                                eraseFracts();
                                eraseSensationWindows();
                                currentBatchSequenceNumber++;
                                setNextFrameMonkeyUnprepared();
                                setNextFrameMonkeyUndrawn();
                                prepareNextTrial();
                                gotoExperimentState(Experiment_State_Prep);
                                gotoStatePhase(State_Phase_Enter);
                                gotoState(Trial_State_ITI);
                        }

                }// End of Trial_State_Result

          }// End of Experiment Run

          else if (experimentState == Experiment_State_Exit){

          }// End of Experiment Exit

          //Update Screen
          if ( getTime() > SDL_LastRefresh + RefreshInterval ){
                  updateFrame();

          }

                /* BUTTON RESPONSE TEST */
                /* Check for eye-tracker buttons pressed */
                /* This is the preferred way to get response data or end trials  */
          button = eyelink_last_button_press(NULL);
          if(button!=0)       /* button number, or 0 if none pressed */
                {
                  /* message to log the button press */
                        eyemsg_printf("ENDBUTTON %d", button);
                        end_trial(); /*  local function to stop recording */
                        break;       /* exit trial loop */
                }
        }                       /* END OF RECORDING LOOP */
        end_realtime_mode();    /* safety cleanup code   */
        while(getkey());        /* dump any accumulated key presses */

        /* report response result: 0=timeout, else button number */
        eyemsg_printf("TRIAL_RESULT %d", button);
        /* Call this at the end of the trial, to handle special conditions */
        return check_record_exit();
}

void drawGrid(){
	SDL_SetRenderTarget(renderer2, Texture_Grid);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Grid.r, Color_Grid.g, Color_Grid.b, Color_Grid.a);
	for (int i = -ceil(disptheta); i<ceil(disptheta)+1; i++){
		SDL_Rect rv, rh;
		rv.x = ((bound2.w)/2) + degToPixel(1.0*i);
		rv.y = bound2.y;
		rv.w = (i%5==0) ? 4 : 1;
		rv.h = bound2.h;
		rh.x = 0;
		rh.y = ((bound2.h)/2) + degToPixel(1.0*i);
		rh.w = bound2.w;
		rh.h = rv.w;
		SDL_RenderFillRect(renderer2, &rv);
		SDL_RenderFillRect(renderer2, &rh);
		if (i>0)
			drawCircle(renderer2, Texture_Grid, Color_Grid, bound2.w/2, bound2.h/2, degToPixel(1.0*i));
	}
	printf("ceil = %f\n", ceil(16.0/currentNumOfFractals));
	for (int i=0; i<=ceil(16.0/currentNumOfFractals); i++){
		    SDL_SetRenderDrawColor(renderer2, 16, 16, 0, Color_Grid.a);
			printf("i= %d, rad = %f\n", i, i*ANGLE/ceil(16/currentNumOfFractals));
		printf("sin = %f\n", sin(1.0*i*ANGLE/ceil(16.0/currentNumOfFractals)));	
		printf("sinfull = %d\n", degToPixel(13.0)* sin(1.0*i*ANGLE/ceil(16.0/currentNumOfFractals)));
			printf("x1 = %d, y1= %d, x2 = %d, y2 = %d\n", 
				getTransformedEdge(Regions_Center_X), 
				getTransformedEdge(Regions_Center_Y), 
				getTransformedEdge(Regions_Center_X + degToPixel(13.0)* cos((i*(ANGLE/ceil(16.0/currentNumOfFractals))) - ANGLE/2)), 
				getTransformedEdge(Regions_Center_Y + degToPixel(13.0)* sin((i*(ANGLE/ceil(16.0/currentNumOfFractals))) - ANGLE/2)));
			SDL_RenderDrawLine(renderer2,
							   getTransformedEdge(Regions_Center_X), 
							   getTransformedEdge(Regions_Center_Y), 
							   getTransformedEdge(Regions_Center_X + degToPixel(13.0)* cos((i*(ANGLE/ceil(16.0/currentNumOfFractals))) - ANGLE/2)), 
							   getTransformedEdge(Regions_Center_Y + degToPixel(13.0)* sin((i*(ANGLE/ceil(16.0/currentNumOfFractals))) - ANGLE/2)));

	}
}

void drawFixation(){
	SDL_SetRenderTarget(renderer2, Texture_Fixation);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Fixation.r, Color_Fixation.g, Color_Fixation.b, Color_Fixation.a);
	SDL_RenderFillRect(renderer2, &Rect_Fixation);
}

void drawFixationMonkey(){
	SDL_SetRenderTarget(renderer, Texture_Fixation_Monkey);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Fixation.r, Color_Fixation.g, Color_Fixation.b, Color_Fixation.a);
	SDL_RenderFillRect(renderer, &Rect_Fixation_Monkey);
	eyemsg_printf("Fixation Rect x,y,w,h = %d,%d,%d,%d", Rect_Fixation_Monkey.x,
			Rect_Fixation_Monkey.y, Rect_Fixation_Monkey.w, Rect_Fixation_Monkey.h);
}
	
//For what state as arg
void drawNextFrameMonkey(trialStatesEnum state){
	if (state == Trial_State_ITI){
		eraseFixationMonkey();
		eraseFractsMonkey();
        erasePhotodiodeMonkey();
	}
	else if (state == Trial_State_Wait_For_Gaze){
		eraseFractsMonkey();
		drawFixationMonkey();
        erasePhotodiodeMonkey();
	}
	else if (state == Trial_State_Fixation_Fix){
		eraseFractsMonkey();
		drawFixationMonkey();
        erasePhotodiodeMonkey();
	}
	else if (state == Trial_State_Search){
		eraseFixationMonkey();
		drawFractsMonkey();
		if (isPhotodiodeCheckOn)
            drawPhotodiodeMonkey();
	}
	else if (state == Trial_State_Fractal_Fix){
		eraseFixationMonkey();
		drawFractsMonkey();
        erasePhotodiodeMonkey();
	}
	isNextFrameDrawnMonkey = 1;
}

void eraseFixation(){
	SDL_SetRenderTarget(renderer2, Texture_Fixation);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
}

void eraseFixationMonkey(){
	SDL_SetRenderTarget(renderer, Texture_Fixation_Monkey);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	//isNextFrameDrawnMonkey = 0;
	//isNextFramePreparedMonkey = 0;
}

void drawFixationWindow(SDL_Color color){
	//SDL_Rect r;
	//r = getTransformedRect(Rect_Fixation_Window);
	SDL_SetRenderTarget(renderer2, Texture_Fixation_Window);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, color.r, color.b, color.g, color.a);
	SDL_RenderDrawRect(renderer2, &Rect_Fixation_Window);
	SDL_Rect r;
	r = getUntransformedRect(Rect_Fixation_Window);
	eyemsg_printf("Fixation Window Rect x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
}


void drawFixationBringBackWindow(SDL_Color color){
	//SDL_Rect r;
	//r = getTransformedRect(Rect_Fixation_Window);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, color.r, color.b, color.g, color.a);
	SDL_RenderDrawRect(renderer2, &Rect_Fixation_Search_Window);
	SDL_Rect r;
	r = getUntransformedRect(Rect_Fixation_Search_Window);
	eyemsg_printf("Bring Back Window Rect x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
}

	/*
	r.x = (Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].x -
                                 ((Rect_Sensation.w - Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].w)/2)); 
	r.y = (Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].y -
                                 ((Rect_Sensation.h - Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].h)/2));
	//r.x = Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].x - ((Rect_Sensation.w - Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].w)/2);		
	//r.y = Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].y - ((Rect_Sensation.h - Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])].h)/2);		
	r.w = Rect_Sensation.w;
	r.h = Rect_Sensation.h;
	*/


void drawSensationWindowGood(){
	if (currentTrialType == 1){
	SDL_Rect r;
	SDL_Color c;
	c = Color_Fixation_Window_Success;
       //	= {0,0,0,0};
	r = getSensationRectGood();
	//c = (getFractal(randomSequenseNumbers[currentSequenceNumber]) < 4) ? Color_Fixation_Window_Success : Color_Fixation_Window_Error;
	SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
	//SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	//SDL_RenderClear(renderer2);
	//printf("Good R: x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
	//eyemsg_printf("Good R: x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, c.r,c.g,c.b,255);
	SDL_RenderDrawRect(renderer2, &r);
	r = getUntransformedRect(r);
	eyemsg_printf("Good Sensation Window of region %d = %d,%d,%d,%d", currentGoodFractalRegion, r.x, r.y, r.w, r.h);


	}
}

void drawSensationWindowBad(){

	SDL_Rect r;
	SDL_Color c;
	c = Color_Fixation_Window_Error;
	SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
       //	= {0,0,0,0};
	for (int i=1; i<=currentNumOfFractals; i++){
		/*if (i==1){
			c.r = 127;
			c.g = 127;
			c.b = 0;
		}
		else
			c = Color_Fixation_Window_Error;*/
		if (i!=1 && currentTrialType == 1 && i == currentGoodFractalRegion)
			continue;
		r.x = Rect_Regions[i].x + Rect_Regions[i].w/2 - Rect_Sensation.w/2;
		r.y = Rect_Regions[i].y + Rect_Regions[i].h/2 - Rect_Sensation.h/2;
		r.w = Rect_Sensation.w;
		r.h = Rect_Sensation.h;
		r = getTransformedRect(r);
		SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer2, c.r,c.g,c.b,255);
		SDL_RenderDrawRect(renderer2, &r);
		r = getUntransformedRect(r);
		eyemsg_printf("Bad Sensation Window of region %d = Rect x,y,w,h = %d,%d,%d,%d", i, r.x, r.y, r.w, r.h);

	}
}

void eraseSensationWindows(){
	SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
}
	/*
	printf("randomSeq: ");
	eyemsg_printf("randomSeq: ");
	for (int i=0; i<16; i++){
		printf("%d, ", randomSequenseNumbers[i]);
		eyemsg_printf("%d, ", randomSequenseNumbers[i]);
	}
	printf("\n");
	eyemsg_printf("\n");
	printf("randomSeqiReg: ");
	eyemsg_printf("randomSeqiReg: ");
	for (int i=0; i<16; i++){
		printf("%d, ", randomSequenseNumbersRegions[i]);
		eyemsg_printf("%d, ", randomSequenseNumbersRegions[i]);
	}
	printf("\n");
	eyemsg_printf("\n");
	printf("GetGood Fract:");
	eyemsg_printf("GetGood Fract:");
	for (int i=0; i<16; i++){
		printf("%d, ", getFractalGood(randomSequenseNumbersRegions[i]));
		eyemsg_printf("%d, ", getFractalGood(randomSequenseNumbersRegions[i]));
	}
	printf("\n");
	eyemsg_printf("\n");
	printf("GetGood Reg:");
	eyemsg_printf("GetGood Reg:");
	for (int i=0; i<16; i++){
		printf("%d, ", getRegionGood(randomSequenseNumbersRegions[i]));
		eyemsg_printf("%d, ", getRegionGood(randomSequenseNumbersRegions[i]));
	}
	printf("\n");
	eyemsg_printf("\n");
	printf("GetBad Fract:");
	eyemsg_printf("GetBad Fract:");
	for (int i=0; i<16; i++){
		printf("%d, ", getFractalBad(randomSequenseNumbersRegions[i]));
		eyemsg_printf("%d, ", getFractalBad(randomSequenseNumbersRegions[i]));
	}
	printf("\n");
	eyemsg_printf("\n");
	printf("GetBad Reg:");
	eyemsg_printf("GetBad Reg:");
	for (int i=0; i<16; i++){
		printf("%d, ", getRegionBad(randomSequenseNumbersRegions[i]));
		eyemsg_printf("%d, ", getRegionBad(randomSequenseNumbersRegions[i]));
	}
	printf("\n");
	eyemsg_printf("\n");



	printf("Good: Pos: %d, Fract: %d", getRegionGood(randomSequenseNumbersRegions[currentSequenceNumber]), getFractalGood(randomSequenseNumbers[currentSequenceNumber]));
	eyemsg_printf("Good: Pos: %d, Fract: %d", getRegionGood(randomSequenseNumbersRegions[currentSequenceNumber]), getFractalGood(randomSequenseNumbers[currentSequenceNumber]));
	printf("Bad: Pos: %d, Fract: %d", getRegionBad(randomSequenseNumbersRegions[currentSequenceNumber]), getFractalBad(randomSequenseNumbers[currentSequenceNumber]));
	eyemsg_printf("Bad: Pos: %d, Fract: %d", getRegionBad(randomSequenseNumbersRegions[currentSequenceNumber]), getFractalBad(randomSequenseNumbers[currentSequenceNumber]));
	*/
void drawFracts(){
	SDL_SetRenderTarget(renderer2, Texture_Fractal);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);	
	SDL_RenderCopy(renderer2, Texture_Fractal_Prep, NULL, NULL);

}

void eraseFracts(){
	SDL_SetRenderTarget(renderer2, Texture_Fractal);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
}

void drawFractsMonkey(){
	drawFractalsMonkey = 1;
}

void eraseFractsMonkey(){
	drawFractalsMonkey = 0;
}

void drawPhotodiodeMonkey(){
    SDL_SetRenderTarget(renderer, Texture_Photodiode_Monkey);
    SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, Color_Photodiode.r, Color_Photodiode.g, Color_Photodiode.b, Color_Fixation.a);
    SDL_RenderFillRect(renderer, &Rect_Photodiode_Monkey);
}
void erasePhotodiodeMonkey(){
    SDL_SetRenderTarget(renderer, Texture_Photodiode_Monkey);
    SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
    SDL_RenderClear(renderer);
}


/*
void drawFract(){
	SDL_SetRenderTarget(renderer2, Texture_Fractal);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	
	//for (int i=0; i<8; i++){
		
		//SDL_Rect fract = scaleRenderer();
		
		//fract.x = rand()%bound2.w;
		//fract.y = rand()%bound2.h;
		//fract.w = 100;
		//fract.h = 100;
		
		//for (int j=0; j<4; j++){
		//	SDL_RenderCopy(renderer2, Texture_Fractals[i], NULL, &Rect_Regions[4*i+j]);
		//	SDL_RenderCopy(renderer, Texture_Fractals_Monkey[i], NULL, &Rect_Regions[4*i+j]);
		//}
	//}
	
	
	//SDL_RenderCopy(renderer2, Texture_Fractals[getFractal(randomSequenseNumbers[currentSequenceNumber])], NULL, &Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])]);
	//SDL_RenderCopy(renderer, Texture_Fractals_Monkey[getFractal(randomSequenseNumbers[currentSequenceNumber])], NULL, &Rect_Regions[getRegion(randomSequenseNumbers[currentSequenceNumber])]);
	

}
*/

void eraseFixationWindow(){
	SDL_SetRenderTarget(renderer2, Texture_Fixation_Window);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
}

void drawGaze(){
	SDL_SetRenderTarget(renderer2, Texture_Gaze);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Gaze.r, Color_Gaze.g, Color_Gaze.b, Color_Gaze.a);
	SDL_RenderFillRect(renderer2, &Rect_Gaze);
}


void eraseGaze(){
	SDL_SetRenderTarget(renderer2, Texture_Gaze);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
}

void setGaze(){
	float renderer2ScaleFractX = (float) bound2.w / (float) bound3.w;
	float renderer2ScaleFractY = (float) bound2.h / (float) bound3.h;
	Rect_Gaze.x = ((evt.fs.gx[eye_used] - Rect_Gaze.w/2)- bound3.x)*renderer2ScaleFractX + gazeXOffset;
	Rect_Gaze.y = (evt.fs.gy[eye_used] - Rect_Gaze.h/2)*renderer2ScaleFractY + gazeYOffset;
}

void updateFrame(){
	SDL_SetRenderTarget(renderer2, NULL);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 255);
	SDL_RenderClear(renderer2);
	SDL_RenderCopy(renderer2, Texture_Grid, NULL, NULL);
	//SDL_RenderCopy(renderer2, content2Texture, NULL, NULL);
	//SDL_RenderCopy(renderer2, content2TextureRect, NULL, NULL);
	float renderer2ScaleFractX = (float) bound3.w / (float) bound2.w;
	float renderer2ScaleFractY = (float) bound3.h / (float) bound2.h;
	SDL_RenderSetScale(renderer2, 1/renderer2ScaleFractX, 1/renderer2ScaleFractY); //Parameterize
	SDL_RenderCopy(renderer2, Texture_Fractal, NULL, NULL);
	SDL_RenderSetScale(renderer2, 1/renderer2ScaleX, 1/renderer2ScaleY); //Parameterize
	SDL_RenderCopy(renderer2, Texture_Fixation, NULL, NULL);
	SDL_RenderCopy(renderer2, Texture_Fixation_Window, NULL, NULL);
	SDL_RenderCopy(renderer2, Texture_Sensation_Window, NULL, NULL);
	SDL_RenderCopy(renderer2, Texture_Gaze, NULL, NULL);
	SDL_RenderPresent(renderer2);
       	SDL_LastRefresh = getTime();
}

void prepareFrameMonkey(){
	/*
	SDL_Rect r;
	r.h = bound1.h;
	r.w = r.h;
	r.x = (bound1.w - r.w)/2;
	r.y = bound1.y;
	*/
	SDL_SetRenderTarget(renderer,NULL);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 255);
	SDL_RenderClear(renderer);
	if (drawFractalsMonkey)
		SDL_RenderCopy(renderer, Texture_Fractal_Monkey, NULL, &bound3);
	SDL_RenderCopy(renderer, Texture_Fixation_Monkey, NULL, &bound3);
    SDL_RenderCopy(renderer, Texture_Photodiode_Monkey, NULL, &bound1);
	isNextFramePreparedMonkey = 1;

}

void updateFrameMonkey(){
	eyemsg_printf("Updated Display of Monkey\n");
	SDL_SetRenderTarget(renderer,NULL);
	SDL_RenderPresent(renderer);
}

void writeToEyelink(char *msg){
    char msgToSend[200];
    strcpy(msgToSend, "EE ");
    strcat(msgToSend, msg);
    eyemsg_printf(msgToSend);
}

/*
void writeToBlackrock(char *msg){
	char msgToSend[30];
	strcpy(msgToSend, msg);
	strcat(msgToSend, "\n");
	write(fd, msgToSend, sizeof(msgToSend)/sizeof(char));
}

void writeToEyelink(char *msg){
	char msgToSend[200];
	strcpy(msgToSend, "EE ");
	strcat(msgToSend, msg);
	eyemsg_printf(msgToSend);
}

void playError(){
	char playstr[200];
	strcpy(playstr, "play -v  ");
	strcat(playstr, errorAmp);
	strcat(playstr, " ");
	strcat(playstr, errorTonePath);
	system(playstr);
	writeToEyelink("Sound Success Played");
}

void playSuccess(){
	char playstr[200];
	strcpy(playstr, "play -v  ");
	strcat(playstr, successAmp);
	strcat(playstr, " ");
	strcat(playstr, successTonePath);
	system(playstr);
	writeToEyelink("Sound Error Played");
}
*/
void gotoExperimentState(experimentStateEnum state){
	experimentState = state;	  
}

void gotoState(trialStatesEnum state){
	trialStateStartTime = getTime();
	statePhase = State_Phase_Enter;
	trialState = state;
	if (state == Trial_State_Search)
		searchEvents[Search_Event_Reject_Before_Exiting_Fixation].startTime = trialStateStartTime;
}

void gotoStatePhase (statePhaseEnum state){
	statePhase = state;
}

void processSearchTimes(){
	UINT32 currentMcs = getTime();
//	printf("search[bring].can = %s, cur (%d) - search start(%d) + exp (%d) = %d\n", (searchEvents[Search_Event_Bring_Back_Fixation].canBeExpired == 1) ? "1" : "0", currentMcs, searchEvents[Search_Event_Bring_Back_Fixation].startTime, searchEvents[Search_Event_Bring_Back_Fixation].expirationTime, currentMcs - searchEvents[Search_Event_Bring_Back_Fixation].startTime + searchEvents[Search_Event_Bring_Back_Fixation].expirationTime);
//	eyemsg_printf("search[bring].can = %s, cur (%d) - search start(%d) + exp (%d) = %d\n", (searchEvents[Search_Event_Bring_Back_Fixation].canBeExpired == 1) ? "1" : "0", currentMcs, searchEvents[Search_Event_Bring_Back_Fixation].startTime, searchEvents[Search_Event_Bring_Back_Fixation].expirationTime, currentMcs - searchEvents[Search_Event_Bring_Back_Fixation].startTime + searchEvents[Search_Event_Bring_Back_Fixation].expirationTime);
	if (!trialStates[trialState].hasExpired && currentMcs >= trialStateStartTime + trialStates[trialState].expirationTime){
		printf("Expired state after %d\n", currentMcs - trialStateStartTime);
		eyemsg_printf("Expired state after %d\n", currentMcs - trialStateStartTime);
		trialStates[trialState].hasExpired = 1;
	}
	for (int i=0; i< Search_Event_Len; i++){
		if ( !searchEvents[i].hasExpired &&
		     searchEvents[i].canBeExpired &&
		     currentMcs >= searchEvents[i].startTime + searchEvents[i].expirationTime){
			char str[100];
			switch(i){
				case 0:
					strcpy(str, "\n\n\n BRING BACK!!!!! \n\n\n");
				case 1:
					strcpy(str, "reject before\n");
				case 2:
					strcpy(str, "reject after\n");
				case 3:
					strcpy(str, "commit\n");
					
			}
			printf("Expired event %s after %d",str, currentMcs - searchEvents[i].startTime);
			eyemsg_printf("Expired event %s after %d",str, currentMcs - searchEvents[i].startTime);
		       searchEvents[i].hasExpired = 1;
		}
	}
}
/*
processFractFixTimes(){
	UINT32 currentMcs = getTime();
	if (!trialStates[trialState].hasExpired && currentMcs >= trialStateStartTime + trialStates[trialState].expirationTime){
		printf("Expired state after %d\n", currentMcs - trialStateStartTime);
		eyemsg_printf("Expired state after %d\n", currentMcs - trialStateStartTime);
	}
}
*/
void drawSearchFixation(){
	//printf("*****************inside draw search fix\n\n\n");
	//eyemsg_printf("*****************inside draw search fix\n\n\n");
	SDL_SetRenderTarget(renderer2, Texture_Fixation);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Fixation_Search.r, Color_Fixation_Search.g, Color_Fixation_Search.b, 255);
	SDL_RenderFillRect(renderer2, &Rect_Fixation_Search);

}

void drawSearchFixationMonkey(){
	SDL_SetRenderTarget(renderer, Texture_Fixation_Monkey);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Fixation_Search.r, Color_Fixation_Search.g, Color_Fixation_Search.b, 255);
	SDL_RenderFillRect(renderer, &Rect_Fixation_Search_Monkey);
	eyemsg_printf("Search Fixation Rect x,y,w,h = %d,%d,%d,%d", Rect_Fixation_Search_Monkey.x,
			Rect_Fixation_Search_Monkey.y, Rect_Fixation_Search_Monkey.w, Rect_Fixation_Search_Monkey.h);
}

void actSearchTimes(){
	if ( !searchEvents[Search_Event_Bring_Back_Fixation].hasActed &&
	     searchEvents[Search_Event_Bring_Back_Fixation].hasExpired){
		printf("Bringing Fixation Back\n");
		eyemsg_printf("Bringing Fixation Back\n");
		drawSearchFixation();
		drawSearchFixationMonkey();
		prepareFrameMonkey();
		updateFrameMonkey();
		searchEvents[Search_Event_Bring_Back_Fixation].hasActed = 1;
		searchEvents[Search_Event_Bring_Back_Fixation].canBeExpired =0;

	}
	if ( !searchEvents[Search_Event_Reject_Before_Exiting_Fixation].hasActed &&
	     searchEvents[Search_Event_Reject_Before_Exiting_Fixation].hasExpired){
		//has rejected before exitting	
		setFractFixationSuccess();
		trialRunResult = Trial_Run_Result_Reject_Before_Exitting_Fixation_Window;
		gotoStatePhase(State_Phase_Exit);
		printf("\n\n\n === Result === Rejected Before Exitting Fixation Window\n\n\n");
		eyemsg_printf("\n\n\n === Result === Rejected Before Exitting Fixation Window\n\n\n");
		searchEvents[Search_Event_Reject_Before_Exiting_Fixation].hasActed = 1;
	}
	if ( !searchEvents[Search_Event_Reject_After_Exiting_Fixation].hasActed &&
	     searchEvents[Search_Event_Reject_After_Exiting_Fixation].hasExpired){
		//has rejected after exitting	
		setFractFixationSuccess();
		trialRunResult = Trial_Run_Result_Reject_After_Exitting_Fixation_Window;
		gotoStatePhase(State_Phase_Exit);
		printf("\n\n\n === Result === Rejected After Exitting Fixation Window\n\n\n");
		eyemsg_printf("\n\n\n === Result === Rejected After Exitting Fixation Window\n\n\n");
		searchEvents[Search_Event_Reject_After_Exiting_Fixation].hasActed = 1;
	}
	if (trialStates[trialState].hasExpired){
		printf("\n\n\n === Result === Failed To Reach a Decision Before Timeout\n\n\n");
		eyemsg_printf("\n\n\n === Result === Failed To Reach a Decision Before Timeout\n\n\n");
		trialRunResult = Trial_Run_Result_Timeout;
		gotoStatePhase(State_Phase_Exit);
	}
	//Act of commitment is handled on processGaze
}

int timedOut(){
	UINT32 currentMsc = getTime();
	if (currentMsc >= trialStateStartTime + trialStates[trialState].expirationTime){
		printf("Expired after %d\n", currentMsc - trialStateStartTime);
		eyemsg_printf("Expired after %d\n", currentMsc - trialStateStartTime);
		return 1;
	}
	return 0;	
}

void prepareNextTrial(){
	monkeyGazeState = Monkey_Gaze_State_HasNotLookedInsideFixationWindow;
	gazeFixResult = Gaze_Fix_Result_Undetermined;
	gazeFractFixResult = Gaze_Fract_Fix_Result_Undetermined;
	monkeyFractGazeState = Monkey_Fract_Gaze_State_Hasnt_Left_Fixation_Window;
	monkeyFractChoice = Monkey_Fract_Choice_Undetermined;
	lastTrialRunResult = trialRunResult;
	reloadedConf = 0;
    photodiodeAck = 0;
    initedPhotodiodeAck = 0;
    sentEventToNeuralData = 0;

	//Clean ups	
}

int isInsideFixationWindow(){
	if (isPointInsideRegion(
	(Rect_Gaze.x + (Rect_Gaze.w/2)),
	(Rect_Gaze.y + (Rect_Gaze.h/2)),
	Rect_Fixation_Window))
		return 1;
	return 0;
}

int isInsideSensationWindow(int Region_Idx){
	if (isPointInsideRegion(
	(Rect_Gaze.x + (Rect_Gaze.w/2)),
	(Rect_Gaze.y + (Rect_Gaze.h/2)),
	getTransformedRect(getSensationRect(Rect_Regions[Region_Idx]))))
		return 1;
	return 0;
}

int isInsideBringBackFixationWindow(){
	if (isPointInsideRegion(
	(Rect_Gaze.x + (Rect_Gaze.w/2)),
	(Rect_Gaze.y + (Rect_Gaze.h/2)),
	Rect_Fixation_Search_Window))
		return 1;
	return 0;
}

SDL_Rect getSensationRect(SDL_Rect i){
	SDL_Rect r; 
	r.x = i.x + i.w/2 - Rect_Sensation.w/2;
	r.y = i.y + i.h/2 - Rect_Sensation.h/2;
	r.w = Rect_Sensation.w;
	r.h = Rect_Sensation.h;
	return r;
}

int getCurrentGazeRegion(){
	if (isInsideFixationWindow())
		return 0;
	for (int i=1; i<=currentNumOfFractals; i++){
		if (isInsideSensationWindow(i))
			return i;
	}
	return -1;
}

int isPointInsideRegion(int x, int y, SDL_Rect r){
	if ((x > r.x) &&
	    (x < (r.x + r.w)) &&
	    (y > r.y) &&
	    (y < (r.y + r.h))){
		return 1;
	}
	return 0;
}

void processGaze(){
	if(trialState == Trial_State_Wait_For_Gaze){
		if (monkeyGazeState == Monkey_Gaze_State_HasNotLookedInsideFixationWindow &&
		   isInsideFixationWindow())
			monkeyGazeState = Monkey_Gaze_State_HasLookedInsideFixationWindow;
	}
	else if (trialState == Trial_State_Fixation_Fix){
		if (!isInsideFixationWindow()){
			gazeFixResult = Gaze_Fix_Result_Broke;
			printf("Broken now!");
			eyemsg_printf("Broken now!");
		}
	}/*
	else if (trialState == Trial_State_Search){
		int currentGazeRegion = getCurrentGazeRegion();
		//if (rand()%20 == 0)
		//	printf("Current region = %d\n", currentGazeRegion);
		//	eyemsg_printf("Current region = %d\n", currentGazeRegion);
		if (currentGazeRegion != Region_Last){
			currentNumberOfOutOfRegionSamples++;
		}
		else if (currentNumberOfOutOfRegionSamples>0){
			currentNumberOfOutOfRegionSamples = 0;
		}
		if (currentNumberOfOutOfRegionSamples > numberOfOutOfRegionSamplesAllowed){
			
		}
		if (!searchEvents[Search_Event_Bring_Back_Fixation].hasActed &&
		    searchEvents[Search_Event_Bring_Back_Fixation].canBeExpired &&
		    !isInsideBringBackFixationWindow()){
			searchEvents[Search_Event_Bring_Back_Fixation].hasExpired = 1;
		}
		if ( !searchEvents[Search_Event_Commit].hasActed &&
	     	     searchEvents[Search_Event_Commit].hasExpired){
			if (currentTrialType == 1 && currentGazeRegion == currentGoodFractalRegion){
				trialRunResult = Trial_Run_Result_Commit_On_Good;
				printf("\n\n\n=== Result === Chose good fractal\n\n\n");
				eyemsg_printf("\n\n\n=== Result === Chose good fractal\n\n\n");
				}
				else {

		//has committed	
				trialRunResult = Trial_Run_Result_Commit_On_Bad;
				printf("\n\n\n === Result === Chose bad fractal\n\n\n");
				eyemsg_printf("\n\n\n === Result === Chose bad fractal\n\n\n");
			}
			Region_Choice_Idx =  currentGazeRegion;
			gotoStatePhase(State_Phase_Exit);
			searchEvents[Search_Event_Commit].hasActed = 1;
		}
	}*/

	    else if (trialState == Trial_State_Search){
        int currentGazeRegion = getCurrentGazeRegion();
        //if (rand()%20 == 0)
        //  printf("Current region = %d\n", currentGazeRegion);
        //  eyemsg_printf("Current region = %d\n", currentGazeRegion);
        if (currentGazeRegion != Region_Last){
            currentNumberOfOutOfRegionSamples++;
        }
        else if (currentNumberOfOutOfRegionSamples>0){
            currentNumberOfOutOfRegionSamples = 0;
        }
        if (currentNumberOfOutOfRegionSamples > numberOfOutOfRegionSamplesAllowed){
            printf("Setting new region: %d\n", currentGazeRegion);
            eyemsg_printf("Setting new region: %d\n", currentGazeRegion);
            setCurrentGazeRegion(currentGazeRegion);
			double offset = Regions_Offset + ANGLE/2; 
			if (currentTrialType == 1 && currentGazeRegion == currentGoodFractalRegion){
				trialRunResult = Trial_Run_Result_Commit_On_Good;
				printf("\n\n\n=== Result === Chose good fractal\n\n\n");
				eyemsg_printf("\n\n\n=== Result === Chose good fractal\n\n\n");

				if (currentNumOfFractals == 3){	
				int offsetnum = offset/(0.349066); // 20/180*PI 
				sendEventToNeuralData(CODES_SEARCH_DS3_GOOD_SACC_ONSET[6*(currentGazeRegion-1)+offsetnum]);	
				printf("TAG: %s", CODES_SEARCH_DS3_GOOD_SACC_ONSET[6*(currentGazeRegion-1)+offsetnum]);
				eyemsg_printf("TAG: %s", CODES_SEARCH_DS3_GOOD_SACC_ONSET[6*(currentGazeRegion-1)+offsetnum]);
				}
				else if (currentNumOfFractals == 5){
				int offsetnum = offset/(0.314159); // 18/180*PI
				sendEventToNeuralData(CODES_SEARCH_DS5_GOOD_SACC_ONSET[4*(currentGazeRegion-1)+offsetnum]);	
				printf("TAG: %s", CODES_SEARCH_DS5_GOOD_SACC_ONSET[4*(currentGazeRegion-1)+offsetnum]);
				eyemsg_printf("TAG: %s", CODES_SEARCH_DS5_GOOD_SACC_ONSET[4*(currentGazeRegion-1)+offsetnum]);
				}	
				else if (currentNumOfFractals == 7){
				int offsetnum = offset/(0.299199);
				sendEventToNeuralData(CODES_SEARCH_DS7_GOOD_SACC_ONSET[3*(currentGazeRegion-1)+offsetnum]);
				printf("TAG: %s", CODES_SEARCH_DS7_GOOD_SACC_ONSET[3*(currentGazeRegion-1)+offsetnum]);
				eyemsg_printf("TAG: %s", CODES_SEARCH_DS7_GOOD_SACC_ONSET[3*(currentGazeRegion-1)+offsetnum]);
				}
				else if (currentNumOfFractals == 9){
				int offsetnum = offset/(0.349066);
                sendEventToNeuralData(CODES_SEARCH_DS9_GOOD_SACC_ONSET[2*(currentGazeRegion-1)+offsetnum]);
				eyemsg_printf("TAG: %s", CODES_SEARCH_DS9_GOOD_SACC_ONSET[2*(currentGazeRegion-1)+offsetnum]);
				printf("TAG: %s", CODES_SEARCH_DS9_GOOD_SACC_ONSET[2*(currentGazeRegion-1)+offsetnum]);
				}
			}
			else if (currentTrialType == 0){
				if (currentNumOfFractals == 3){
                int offsetnum = offset/(0.349066); // 20/180*PI 
                sendEventToNeuralData(CODES_SEARCH_DS3_BAD_SACC_ONSET[6*(currentGazeRegion-1)+offsetnum]);
                eyemsg_printf("TAG: %s", CODES_SEARCH_DS3_BAD_SACC_ONSET[6*(currentGazeRegion-1)+offsetnum]);
                printf("TAG: %s", CODES_SEARCH_DS3_BAD_SACC_ONSET[(6*currentGazeRegion-1)+offsetnum]);
                }
                else if (currentNumOfFractals == 5){
                int offsetnum = offset/(0.314159); // 18/180*PI
                sendEventToNeuralData(CODES_SEARCH_DS5_BAD_SACC_ONSET[(4*currentGazeRegion-1)+offsetnum]);
                eyemsg_printf("TAG: %s", CODES_SEARCH_DS5_BAD_SACC_ONSET[4*(currentGazeRegion-1)+offsetnum]);
                printf("TAG: %s", CODES_SEARCH_DS5_BAD_SACC_ONSET[4*(currentGazeRegion-1)+offsetnum]);
                }
                else if (currentNumOfFractals == 7){
                int offsetnum = offset/(0.299199);
                sendEventToNeuralData(CODES_SEARCH_DS7_BAD_SACC_ONSET[3*(currentGazeRegion-1)+offsetnum]);
                eyemsg_printf("TAG: %s", CODES_SEARCH_DS7_BAD_SACC_ONSET[3*(currentGazeRegion-1)+offsetnum]);
                printf("TAG: %s", CODES_SEARCH_DS7_BAD_SACC_ONSET[3*(currentGazeRegion-1)+offsetnum]);
                }
                else if (currentNumOfFractals == 9){
                int offsetnum = offset/(0.349066);
                sendEventToNeuralData(CODES_SEARCH_DS9_BAD_SACC_ONSET[2*(currentGazeRegion-1)+offsetnum]);
                eyemsg_printf("TAG: %s", CODES_SEARCH_DS9_BAD_SACC_ONSET[2*(currentGazeRegion-1)+offsetnum]);
                printf("TAG: %s", CODES_SEARCH_DS9_BAD_SACC_ONSET[2*(currentGazeRegion-1)+offsetnum]);
           		}
			}

			
        }
        if (!searchEvents[Search_Event_Bring_Back_Fixation].hasActed &&
            searchEvents[Search_Event_Bring_Back_Fixation].canBeExpired &&
            !isInsideBringBackFixationWindow()){
            searchEvents[Search_Event_Bring_Back_Fixation].hasExpired = 1;
        }
        if ( !searchEvents[Search_Event_Commit].hasActed &&
                 searchEvents[Search_Event_Commit].hasExpired){
        //has committed 
            if (currentTrialType == 1 && currentGazeRegion == currentGoodFractalRegion){
                trialRunResult = Trial_Run_Result_Commit_On_Good;
                printf("\n\n\n=== Result === Chose good fractal\n\n\n");
                eyemsg_printf("\n\n\n=== Result === Chose good fractal\n\n\n");
            }
            else{
                trialRunResult = Trial_Run_Result_Commit_On_Bad;
                printf("\n\n\n === Result === Chose bad fractal\n\n\n");
                eyemsg_printf("\n\n\n === Result === Chose bad fractal\n\n\n");
            }
            Region_Choice_Idx =  currentGazeRegion;
            gotoStatePhase(State_Phase_Exit);
            searchEvents[Search_Event_Commit].hasActed = 1;
        }
    }

	
	else if (trialState == Trial_State_Fractal_Fix){
		if (!isInsideSensationWindow(Region_Choice_Idx)){
			gazeFractFixResult = Gaze_Fract_Fix_Result_Broke;
		}	
	}
    
/*
		if (monkeyFractGazeState == Monkey_Fract_Gaze_State_Hasnt_Left_Fixation_Window &&
		   Region_Last == 0){
			printf("Toggled to Has looked sensation win good\n");	
			eyemsg_printf("Toggled to Has looked sensation win good\n");	
			monkeyFractGazeState = Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowGood;
		}
		if (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasNotLookedInsideAnySensationWindow &&
		   isInsideSensationWindowBad()){
			printf("Toggled to Has looked sensation win bad\n");	
			eyemsg_printf("Toggled to Has looked sensation win bad\n");	
			monkeyFractGazeState = Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowBad;
		}
	}
	else if (trialState == Trial_State_Fractal_Fix){
		if ( monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowGood &&
		     !isInsideSensationWindowGood()){
			gazeFractFixResult = Gaze_Fract_Fix_Result_Broke;	
		}
		if ( monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowBad &&
		     !isInsideSensationWindowBad()){
			gazeFractFixResult = Gaze_Fract_Fix_Result_Broke;	
		}

	}
	*/	
}

int hasLookedIntoFixationWindow(){
	if (monkeyGazeState == Monkey_Gaze_State_HasLookedInsideFixationWindow)
		return 1;
	return 0;
}

int hasBrokenFixation(){
	if (gazeFixResult == Gaze_Fix_Result_Broke)
		return 1;
	return 0;
}

int hasKeptFixation(){
	if (gazeFixResult == Gaze_Fix_Result_KeptGaze)
		return 1;
	return 0;
}

void setFixationSuccess(){
	gazeFixResult = Gaze_Fix_Result_KeptGaze;
}

/*
int hasLookedIntoSensationWindow(){
	if (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowGood ||
	    monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowBad)
		return 1;
	return 0;
}
*/

int hasBrokenFractFixation(){
	if (gazeFractFixResult == Gaze_Fract_Fix_Result_Broke)
		return 1;
	return 0;
}

int hasKeptFractFixaton(){
	if (gazeFractFixResult == Gaze_Fract_Fix_Result_KeptGaze)
		return 1;
	return 0;
}

void setFractFixationSuccess(){
	gazeFractFixResult = Gaze_Fract_Fix_Result_KeptGaze;
}

void setMonkeyFractChoiceGood(){
	monkeyFractChoice = Monkey_Fract_Choice_Good;
}

void setMonkeyFractChoiceBad(){
	monkeyFractChoice = Monkey_Fract_Choice_Bad;
}

int hasChosenGoodFract(){
	if (monkeyFractChoice == Monkey_Fract_Choice_Good)
		return 1;
	return 0;
}

int hasChosenBadFract(){
	if (monkeyFractChoice == Monkey_Fract_Choice_Bad)
		return 1;
	return 0;
}


/*
int isNextFramePreparedMonkey(){
	if (isNextFramePreparedMonkey == 1)
		return 1;
	return 0;
}

int isNextFrameDrawnMonkey(){
	if (isNextFrameDrawnMonkey)
		return 1;
	return 0;
	
}
*/

void setCurrentGazeRegion(int i){
	currentRegionStartTime = getTime();
	if (Region_Last == 0 &&
	    monkeyFractGazeState == Monkey_Fract_Gaze_State_Hasnt_Left_Fixation_Window){
		monkeyFractGazeState = Monkey_Fract_Gaze_State_Has_Left_Fixation_Window;
		searchEvents[Search_Event_Reject_Before_Exiting_Fixation].canBeExpired = 0;
	}
	if (Region_Last != 0 && 
	    i == 0 &&
	    monkeyFractGazeState == Monkey_Fract_Gaze_State_Has_Left_Fixation_Window){
		searchEvents[Search_Event_Reject_After_Exiting_Fixation].startTime = currentRegionStartTime;
		searchEvents[Search_Event_Reject_After_Exiting_Fixation].canBeExpired = 1;
		searchEvents[Search_Event_Commit].canBeExpired = 0;

	}
	if (i!=0 && i!=-1){
		searchEvents[Search_Event_Commit].startTime = currentRegionStartTime;
		searchEvents[Search_Event_Commit].canBeExpired = 1;
//		printf("I allowed expiration of commit!\n");
//		eyemsg_printf("I allowed expiration of commit!\n");
		searchEvents[Search_Event_Reject_After_Exiting_Fixation].canBeExpired = 0;
	}
	if (i == -1){
		searchEvents[Search_Event_Reject_Before_Exiting_Fixation].canBeExpired = 0;
		searchEvents[Search_Event_Reject_After_Exiting_Fixation].canBeExpired = 0;
		searchEvents[Search_Event_Commit].canBeExpired = 0;
	}
	Region_Last = i;
}

void setNextFrameMonkeyUnprepared(){
	isNextFramePreparedMonkey = 0;
}

void setNextFrameMonkeyUndrawn(){
	isNextFrameDrawnMonkey = 0;
}

/*
SDL_Rect scaleRenderer(SDL_Rect i){
	SDL_Rect o;
	o.x = i.x;
	o.y = ;
	o.w = i.w / (bound.w/bound2.w);
	o.h = i.h / (bound.h/bound2.h);
	return r;
}
*/

int getRegionGood(int n){
	return n%8;
}

int getRegionBad(int n){
	return (4+getRegionGood(n))%8;
	//return ((4 - getRegionGood(n)) > 0) ? (4 - getRegionGood(n)) : (4 + getRegionGood(n));
}


/*
int getFractal(int n){
	return n%8;
}
*/
int getFractalGood(int n){
	return n/4;
}

int getFractalBad(int n){
	return 4+(n%4);
}
SDL_Rect getSensationRectGood(){
	SDL_Rect r;
	SDL_Rect c = Rect_Regions[currentGoodFractalRegion];
	r.x = c.x + c.w/2 - Rect_Sensation.w/2;
	r.y = c.y + c.h/2 - Rect_Sensation.h/2;
	r.w = Rect_Sensation.w;
	r.h = Rect_Sensation.h;
	return getTransformedRect(r);
}

/*
SDL_Rect getSensationRectBad(){
	SDL_Rect r;
	SDL_Rect rect;
	r.x = (Rect_Regions[getRegionBad(randomSequenseNumbersRegions[currentSequenceNumber])].x -
                                 ((Rect_Sensation.w - Rect_Regions[getRegionBad(randomSequenseNumbersRegions[currentSequenceNumber])].w)/2)); 
	r.y = (Rect_Regions[getRegionBad(randomSequenseNumbersRegions[currentSequenceNumber])].y -
                                 ((Rect_Sensation.h - Rect_Regions[getRegionBad(randomSequenseNumbersRegions[currentSequenceNumber])].h)/2));
	r.w = Rect_Sensation.w;
	r.h = Rect_Sensation.h;
	rect = getTransformedRect(r);
	return rect;
}
*/

SDL_Rect getTransformedRect(SDL_Rect i){
	float renderer2ScaleFractX = (float) bound2.w / (float) bound3.w;
	float renderer2ScaleFractY = (float) bound2.h / (float) bound3.h;
	SDL_Rect o;
	o.x = renderer2ScaleFractX * i.x;
	o.y = renderer2ScaleFractY * i.y;
	o.w = renderer2ScaleFractX * i.w;
	o.h = renderer2ScaleFractY * i.h;
	return o;
}

SDL_Rect getUntransformedRect(SDL_Rect i){
	float renderer2ScaleFractX = (float) bound3.w / (float) bound2.w;
	float renderer2ScaleFractY = (float) bound3.h / (float) bound2.h;
	SDL_Rect o;
	o.x = renderer2ScaleFractX * i.x;
	o.y = renderer2ScaleFractY * i.y;
	o.w = renderer2ScaleFractX * i.w;
	o.h = renderer2ScaleFractY * i.h;
	return o;
}

double getUntransformedEdge(int in){
	float renderer2ScaleFractX = (float) bound3.w / (float) bound2.w;
	//printf("inside untransform: i (%d) * scale (%f)\n", in, renderer2ScaleFractX);
	return in * renderer2ScaleFractX;
}

int getTransformedEdge(int in){
	float renderer2ScaleFractX = (float) bound2.w / (float) bound3.w;
	//printf("inside untransform: i (%d) * scale (%f)\n", in, renderer2ScaleFractX);
	return in * renderer2ScaleFractX;
}

int reloadConf(){
	FILE *fp;
	char str[200];
	fp = fopen("searchConf.txt", "r");
	if (!fp){
		printf("Failed to open file");
		eyemsg_printf("Failed to open file");
		return -1;
	}

	//printf("\n======== PARAMS =========\n");
	//eyemsg_printf("\n======== PARAMS =========\n");
	//while (!feof(fp)){
	while (fgets(str, 200, fp)){
		//fgets(str, 80, fp);
		if (strstr(str, "RejectWithoutExitTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("RejectWithoutExitTime: %d\n", parsedInt);
			searchEvents[Search_Event_Reject_Before_Exiting_Fixation].expirationTime = parsedInt;
		}
		if (strstr(str, "RejectAfterExitTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("RejectAfterExitTime: %d\n", parsedInt);
			searchEvents[Search_Event_Reject_After_Exiting_Fixation].expirationTime = parsedInt; 
		}
		if (strstr(str, "FixationBackTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("FixationBackTime: %d\n", parsedInt);
			searchEvents[Search_Event_Bring_Back_Fixation].expirationTime = parsedInt;
		}
		if (strstr(str, "CommitTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("CommitTime: %d\n", parsedInt);
			searchEvents[Search_Event_Commit].expirationTime = parsedInt;
		}
		if (strstr(str, "GazeWaitTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("GazeWaitTime: %d\n", parsedInt);
			trialStates[Trial_State_Wait_For_Gaze].expirationTime = 2000;
		}

		if (strstr(str, "FixationFixTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("FixationFixTime: %d\n", parsedInt);
			trialStates[Trial_State_Fixation_Fix].expirationTime = parsedInt;
		}
		if (strstr(str, "SearchTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("SearchTime: %d\n", parsedInt);
			trialStates[Trial_State_Search].expirationTime = parsedInt;
		}
		if (strstr(str, "FractFixTime")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("FractFixTime: %d\n", parsedInt);
			trialStates[Trial_State_Fractal_Fix].expirationTime = parsedInt;
		}
		if (strstr(str, "EccentricityDegree")!= NULL){
			char parsedStr[200];
			double parsedDouble;
			parsedDouble = atof(strstr(str, "=") + 1);	
			//printf("EccentricityDegree: %f\n", parsedDouble);
			Regions_Inter_Ring_Distance = (int)degToPixel(1.0*parsedDouble);
		}
		if (strstr(str, "FractalSizeDegree")!= NULL){
			char parsedStr[200];
			double parsedDouble;
			parsedDouble = atof(strstr(str, "=") + 1);	
			//printf("FractalSizeDegree: %f\n", parsedDouble);
			Regions_Width = (int)degToPixel(1.0*parsedDouble);
			Regions_Height = (int)degToPixel(1.0*parsedDouble);
		}
		if (strstr(str, "FixationWindowDegree")!= NULL){
			char parsedStr[200];
			double parsedDouble;
			parsedDouble = atof(strstr(str, "=") + 1);	
			//printf("FixationWindowDegree: %f\n", parsedDouble);
			Rect_Fixation_Window.w = (int)degToPixel(1.0*parsedDouble);
			Rect_Fixation_Window.h = (int)degToPixel(1.0*parsedDouble);
			Rect_Fixation_Window.x = Rect_Fixation_Monkey.x - ((Rect_Fixation_Window.w - Rect_Fixation_Monkey.w)/2);
			Rect_Fixation_Window.y = Rect_Fixation_Monkey.y - ((Rect_Fixation_Window.h - Rect_Fixation_Monkey.h)/2);
			Rect_Regions[0] = Rect_Fixation_Window;
			Rect_Fixation_Window = getTransformedRect(Rect_Fixation_Window);
		}
		if (strstr(str, "SensationWindowDegree")!= NULL){
			char parsedStr[200];
			double parsedDouble;
			parsedDouble = atof(strstr(str, "=") + 1);	
			//printf("SensationWindowDegree: %f\n", parsedDouble);
			Rect_Sensation.w = (int)degToPixel(1.0*parsedDouble);
			Rect_Sensation.h = (int)degToPixel(1.0*parsedDouble);
			Rect_Sensation = getTransformedRect(Rect_Sensation);
		}
		if (strstr(str, "BringFixationBackWindowDegree")!= NULL){
			char parsedStr[200];
			double parsedDouble;
			parsedDouble = atof(strstr(str, "=") + 1);	
			//printf("BringFixationBackWindowDegree: %f\n", parsedDouble);
			Rect_Fixation_Search_Window.w = (int)degToPixel(1.0*parsedDouble);
			Rect_Fixation_Search_Window.h = (int)degToPixel(1.0*parsedDouble);
			Rect_Fixation_Search_Window.x = 
				Rect_Fixation_Search_Monkey.x - ((Rect_Fixation_Search_Window.w - Rect_Fixation_Search_Monkey.w)/2);
			Rect_Fixation_Search_Window.y = 
				Rect_Fixation_Search_Monkey.y - ((Rect_Fixation_Search_Window.h - Rect_Fixation_Search_Monkey.h)/2);
			Rect_Fixation_Search_Window = getTransformedRect(Rect_Fixation_Search_Window);
		}
		if (strstr(str, "TrialSkipRewardDuration")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("TrialSkipRewardDuration: %d\n", parsedInt);
			Reward_Skip_Duration = parsedInt;
		}
		if (strstr(str, "TrialSkipRewardProbability")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("TrialSkipRewardProbability: %d\n", parsedInt);
			Reward_Skip_Probability = parsedInt;
		}
		if (strstr(str, "GazeXOffset")!= NULL){
            char parsedStr[200];
            double parsedDouble;
            parsedDouble = atof(strstr(str, "=") + 1);
            gazeXOffset  = (int)degToPixel(1.0*parsedDouble);
        }
        if (strstr(str, "GazeYOffset")!= NULL){
            char parsedStr[200];
            double parsedDouble;
            parsedDouble = atof(strstr(str, "=") + 1);
        	gazeYOffset  = (int)degToPixel(1.0*parsedDouble);
        }
		if (strstr(str, "PhotodiodeChack")!= NULL){
            char parsedStr[200];
            int parsedInt;
            parsedInt = atoi(strstr(str, "=") + 1);
            printf("Photodiode Check: %d\n", parsedInt);
            isPhotodiodeCheckOn = parsedInt;
        }


	}
	//printf("\n==========PARAMS=========\n\n");
	fclose(fp);
	reloadedConf = 1;
	return 1;
}

void setRegions(){
	Regions_Center_X = bound3.w/2;
	Regions_Center_Y = bound3.h/2;

	srand(time(NULL));
	ANGLE = (M_PI/currentNumOfFractals)*2;
	Regions_Offset = (double)rand()/RAND_MAX*ANGLE - ANGLE/2;
	printf("Offset = %f\n", Regions_Offset);

	for (int i=1; i<=currentNumOfFractals; i++){
		Rect_Regions[i].w = Regions_Width;
		Rect_Regions[i].h = Regions_Height;
		Rect_Regions[i].x = Regions_Center_X +
			Regions_Inter_Ring_Distance * (cos(((i-1)*ANGLE + Regions_Offset))) -
			//Regions_Inter_Ring_Distance * (cos((i-1)*ANGLE + Regions_Offset* M_PI/180)) -
			Regions_Width/2;
		Rect_Regions[i].y = Regions_Center_Y +
			Regions_Inter_Ring_Distance * (sin(((i-1)*ANGLE + Regions_Offset))) -
			//Regions_Inter_Ring_Distance * (sin((i-1)*ANGLE + Regions_Offset* M_PI/180)) -
		Regions_Height/2;
		//printf("region[%d].x,y = %d, %d\n",i, Rect_Regions[i].x, Rect_Regions[i].y);
		eyemsg_printf("region[%d].x,y,w,h = %d, %d, %d, %d\n",i, Rect_Regions[i].x, Rect_Regions[i].y, Rect_Regions[i].w, Rect_Regions[i].h);
	}
}

void placeFracts(){
	//place log for which fractal is loaded in what region
	int curNumOfBadPlacedFracts = 0;
	int fractIdxs[currentNumOfFractals];
	int badFractalPosIdx;
	//Clean texture
	SDL_SetRenderTarget(renderer2, Texture_Fractal_Prep);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);	
	
	SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);	
	//printf("TYPE = %d, DS = %d, GOOD = %d\n", currentTrialType, currentNumOfFractals, currentGoodFractalRegion);
	for (int i=1; i<=currentNumOfFractals; i++){
		//printf("PLACING PRACT %d\n", i);
		if (currentTrialType == 1 && i == currentGoodFractalRegion){
			int goodFractalPosIdx = getNextGoodFractPosIdx();
			//Loaded fractal idx = %d
				SDL_SetRenderTarget(renderer2, Texture_Fractal_Prep);
				SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
				SDL_RenderCopy(renderer2, Texture_Fractals[goodFractalQueueIdx[goodFractalPosIdx]], NULL, &Rect_Regions[i]);
				SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
				SDL_RenderCopy(renderer, Texture_Fractals_Monkey[goodFractalQueueIdx[goodFractalPosIdx]], NULL, &Rect_Regions[i]);
				//printf("Placed fract %d on %d\n", goodFractalQueueIdx[goodFractalPosIdx], i+1);
				//eyemsg_printf("Placed fract %d on %d\n", goodFractalQueueIdx[goodFractalPosIdx], i+1);
				//printf("Placed good fract %d (%8 = %d, %s) on %d\n", goodFractalQueueIdx[goodFractalPosIdx], goodFractalQueueIdx[goodFractalPosIdx]%8, (goodFractalQueueIdx[goodFractalPosIdx]%8) < 4 ? "okay" : "NOT OKAY!!!!!!!", i);
				//eyemsg_printf("Placed good fract %d (%8 = %d, %s) on %d\n", goodFractalQueueIdx[goodFractalPosIdx], goodFractalQueueIdx[goodFractalPosIdx]%8, (goodFractalQueueIdx[goodFractalPosIdx]%8) < 4 ? "okay" : "NOT OKAY!!!!!!!", i);
				//printf("Placed good fract %d on region %d\n", goodFractalQueueIdx[goodFractalPosIdx],i);
				eyemsg_printf("Placed good fract %d on region %d\n", goodFractalQueueIdx[goodFractalPosIdx],i);
		}
		else{
			badFractalPosIdx = getNextBadFractPosIdx();
			//printf("got posidx before loop : %d, translates to %d \n", badFractalPosIdx, badFractalQueueIdx[badFractalPosIdx]);
			//eyemsg_printf("got posidx before loop : %d, translates to %d \n", badFractalPosIdx, badFractalQueueIdx[badFractalPosIdx]);
			for (int j=0; j<curNumOfBadPlacedFracts; j++){
				if (badFractalQueueIdx[badFractalPosIdx] == fractIdxs[j]){
					//printf("CONDITION TRUE! ON %d", j);
					//eyemsg_printf("CONDITION TRUE! ON %d", j);
					//pauseExperiment();
					badFractalPosIdx = getNextBadFractPosIdx();
					//printf("got new idx inside loop : %d, translated to %d \n", badFractalPosIdx, badFractalQueueIdx[badFractalPosIdx]); 
					//eyemsg_printf("got new idx inside loop : %d, translated to %d \n", badFractalPosIdx, badFractalQueueIdx[badFractalPosIdx]); 
					j = 0;
					//printf("reset loop");
					//eyemsg_printf("reset loop");
				}
				//debug
			}
			fractIdxs[curNumOfBadPlacedFracts++] = badFractalQueueIdx[badFractalPosIdx];
			//printf("the list after push \n=====\n");
			//eyemsg_printf("the list after push \n=====\n");
			for (int j=0; j<curNumOfBadPlacedFracts; j++){
				//printf("fractIdxs[%d] = %d\n", j, fractIdxs[j]);
				//eyemsg_printf("fractIdxs[%d] = %d\n", j, fractIdxs[j]);
			}
			//Loaded fractal idx = %d
				SDL_SetRenderTarget(renderer2, Texture_Fractal_Prep);
				SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
				SDL_RenderCopy(renderer2, Texture_Fractals[badFractalQueueIdx[badFractalPosIdx]], NULL, &Rect_Regions[i]);
				SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
				SDL_RenderCopy(renderer, Texture_Fractals_Monkey[badFractalQueueIdx[badFractalPosIdx]], NULL, &Rect_Regions[i]);
				//printf("Placed bad fract %d (%8 = %d, %s) on %d\n", badFractalQueueIdx[badFractalPosIdx], badFractalQueueIdx[badFractalPosIdx]%8, (badFractalQueueIdx[badFractalPosIdx]%8) >=4 ? "okay" : "NOT OKAY!!!!!!!", i);
				//eyemsg_printf("Placed bad fract %d (%8 = %d, %s) on %d\n", badFractalQueueIdx[badFractalPosIdx], badFractalQueueIdx[badFractalPosIdx]%8, (badFractalQueueIdx[badFractalPosIdx]%8) >=4 ? "okay" : "NOT OKAY!!!!!!!", i);
				//printf("Placed bad fract %d on region %d\n", badFractalQueueIdx[badFractalPosIdx],i);
				eyemsg_printf("Placed bad fract %d on region %d\n", badFractalQueueIdx[badFractalPosIdx],i);
		}	
	}
}

/*
void placeFractsMonkey(){
	//place log for which fractal is loaded in what region
	
	//Clean texture
	SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);

	
	for (int i=0; i<currentNumOfFractals; i++){
		if (i == currentGoodFractalRegion){
			int goodFractalPosIdx = getNextGoodFractPosIdx();
			//Loaded fractal idx = %d
				SDL_RenderCopy(renderer, Texture_Fractals[goodFractalQueueIdx[goodFractalPosIdx]], NULL, &Rect_Regions[i+1]);
		}
		else{
			int badFractalPosIdx = getNextBadFractPosIdx();
			//Loaded fractal idx = %d
				SDL_RenderCopy(renderer, Texture_Fractals[badFractalQueueIdx[badFractalPosIdx]], NULL, &Rect_Regions[i+1]);
		}	
	}
}

*/

int getNextGoodFractPosIdx(){
	int idxToReturn = currentGoodFractalQueuePos;
	if (currentGoodFractalQueuePos >= 35){
		randGen(36, goodFractalQueueIdx, Random_Permutation_Type_Fract_Idx_Good);
		currentGoodFractalQueuePos = 0;
	}
	else
		currentGoodFractalQueuePos++;
	return idxToReturn;
}

int getNextBadFractPosIdx(){
	int idxToReturn = currentBadFractalQueuePos;
	if (currentBadFractalQueuePos >= 35){
		randGen(36, badFractalQueueIdx, Random_Permutation_Type_Fract_Idx_Bad);
		currentBadFractalQueuePos = 0;
	}
	else
		currentBadFractalQueuePos++;
	return idxToReturn;
}

double pixelToDeg(int pixels){
	return 1.0*pixels*dispDegPerPixel; 
}

int degToPixel(double degrees){
	//printf("dispPixelPerDeg (%f) * deg (%f) = %f\n", dispPixelPerDeg, degrees, degrees* dispPixelPerDeg);
	//eyemsg_printf("dispPixelPerDeg (%f) * deg (%f) = %f\n", dispPixelPerDeg, degrees, degrees* dispPixelPerDeg);
	return (int)((double)degrees * dispPixelPerDeg); 
}

double degToCm(double degrees){
	//printf("degrees (%f) / dispDegPerCm (%f) = %f\n", degrees, dispDegPerCm, degrees/dispDegPerCm);
	//eyemsg_printf("degrees (%f) / dispDegPerCm (%f) = %f\n", degrees, dispDegPerCm, degrees/dispDegPerCm);
	return degrees / dispDegPerCm;
}

void drawCircle(SDL_Renderer *renderer, SDL_Texture * texture, SDL_Color color, int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);
   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);
   SDL_SetRenderTarget(renderer, texture);
   SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
   SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}
int hasCommittedOnAFract(){
	return (trialRunResult == Trial_Run_Result_Commit_On_Good || 
		trialRunResult == Trial_Run_Result_Commit_On_Bad) ? 1 : 0;
}

int hasFixedOnBadFract(){
	return (trialRunResult == Trial_Run_Result_Fixed_On_Bad) ? 1 : 0;
}

int hasFixedOnGoodFract(){
	return (trialRunResult == Trial_Run_Result_Fixed_On_Good) ? 1 : 0;
}

int hasRejectedTrialBeforeExitingSearchFixationWindow(){
	return (trialRunResult == Trial_Run_Result_Reject_Before_Exitting_Fixation_Window) ? 1 : 0;
}
		
int hasRejectedTrialAfterExitingSearchFixationWindow(){
	return (trialRunResult == Trial_Run_Result_Reject_After_Exitting_Fixation_Window) ? 1 : 0;
}

void setITI(){
	switch (lastTrialRunResult){
		case Trial_Run_Result_Undetermined:
		case Trial_Run_Result_Timeout:
		case Trial_Run_Result_Commit_On_Good:
		case Trial_Run_Result_Commit_On_Bad:
		case Trial_Run_Result_Fixed_On_Good:
		case Trial_Run_Result_Fixed_On_Bad:
			setITINormal();
			break;
		case Trial_Run_Result_Reject_Before_Exitting_Fixation_Window:
		case Trial_Run_Result_Reject_After_Exitting_Fixation_Window:
		      trialStates[Trial_State_ITI].expirationTime = 400;
		      printf("Setting Skip ITI duration : %d\n", trialStates[Trial_State_ITI].expirationTime);
		      eyemsg_printf("Setting Skip ITI duration : %d\n", trialStates[Trial_State_ITI].expirationTime);
		      break;
	}
}

void setITINormal(){
	trialStates[Trial_State_ITI].expirationTime = 1000 + (rand()%500);
	printf("Setting Normal ITI (Non-skip) duration : %d\n", trialStates[Trial_State_ITI].expirationTime);
	eyemsg_printf("Setting Normal ITI (Non-skip) duration : %d\n", trialStates[Trial_State_ITI].expirationTime);
}

int getEdgeFromRect(SDL_Rect r){
	return r.w;
}

void writeConfigToEyelink(){
	eyemsg_printf(" -sets  %d,%d,%d", setsToLoad[0], setsToLoad[1], setsToLoad[2]);
	eyemsg_printf(" -iti  %d ", trialStates[Trial_State_ITI].expirationTime);
	eyemsg_printf(" -gazewait  %d ", trialStates[Trial_State_Wait_For_Gaze].expirationTime);
	eyemsg_printf(" -fixationfix  %d ", trialStates[Trial_State_Fixation_Fix].expirationTime);
	eyemsg_printf(" -search  %d ", trialStates[Trial_State_Search].expirationTime);
	eyemsg_printf(" -rejectwithoutexit  %d ", searchEvents[Search_Event_Reject_Before_Exiting_Fixation].expirationTime);
	eyemsg_printf(" -rejectafterexit  %d ", searchEvents[Search_Event_Reject_After_Exiting_Fixation].expirationTime);
	eyemsg_printf(" -bringbackfixation  %d ", searchEvents[Search_Event_Bring_Back_Fixation].expirationTime);
	eyemsg_printf(" -commit  %d ", searchEvents[Search_Event_Commit].expirationTime);
	eyemsg_printf(" -fractfix  %d ", trialStates[Trial_State_Fractal_Fix].expirationTime);
    eyemsg_printf(" -fixsize  %f ", pixelToDeg(Rect_Fixation_Monkey.w));
        eyemsg_printf(" -searchfixsize  %f ", pixelToDeg(Rect_Fixation_Search_Monkey.w));
        eyemsg_printf(" -fixwin  %f ", pixelToDeg(getUntransformedEdge(Rect_Fixation_Window.w)));
        eyemsg_printf(" -bringbackfixwin  %f ", pixelToDeg(getUntransformedEdge(Rect_Fixation_Search_Window.w)));
        eyemsg_printf(" -sensewin  %f ", pixelToDeg(getUntransformedEdge(Rect_Sensation.w)));
        eyemsg_printf(" -radius  %f ", pixelToDeg(Regions_Inter_Ring_Distance));
        eyemsg_printf(" -fractsize  %f ", pixelToDeg(Regions_Width));
        eyemsg_printf(" -photodiode = %d", isPhotodiodeCheckOn);
        eyemsg_printf(" -gazexoffset = %d ", gazeXOffset);
        eyemsg_printf(" -gazeyoffset = %d ", gazeYOffset);

}

int gotPhotodiodeAck(){
    return photodiodeAck;
}

void initgetPhotodiodeAck(){
    set_blocking (fd, 0);
    for (int i=0; i<7; i++){
        write(fd, CODE_PHOTODIODE_REQ, sizeof(CODE_PHOTODIODE_REQ)-1);
        usleep ( (2) * 100 );
    }
    initedPhotodiodeAck = 1;
}


void stopPollingForPhotodiodeAck(){
    set_blocking (fd, 0);
    for (int i=0; i<3; i++){
   		write(fd, CODE_PHOTODIODE_NEQ, sizeof(CODE_PHOTODIODE_NEQ)-1);
    	usleep ( (2) * 100 );
	}
}

void getPhotodiodeAck(){

unsigned char ack[2] = {0};
		//printf("Took %d to get before serial read\n",current_time() - trialStateStartTime);
        //printf("code = %s, len= %d", CODE_PHOTODIODE_REQ, sizeof(CODE_PHOTODIODE_REQ)-1);
        //write (fd, "P\n", 2);
        int serialread = read(fd, ack, 2);
		//printf("Took %d to get after serial read\n",current_time() - trialStateStartTime);
        printf("insode get ack, got %s\n", ack);
		if (ack[0] == 80 || ack[1] == 80){
        	photodiodeAck = 1;
        	//printf("made ack 1\n");
	    }
   		else
        photodiodeAck = 0;

}

void sendEventToNeuralData(char *msg){
    write (fd, msg, sizeof(msg)-1);
    usleep ( (6) * 100 );
    printf("code = %s, len = %d\n", msg, sizeof(msg)-1);
    sentEventToNeuralData = 1;
}
void sendBigReward(){
        set_blocking (fd, 0);
		for (int i=0;i<7; i++){
        	write (fd, "RL\n", 3);
		}
        usleep ( (3) * 100 );
}

void sendMediumReward(){
        set_blocking (fd, 0);
		for (int i=0;i<7; i++){
        	write (fd, "RM\n", 3);
		}
        usleep ( (3) * 100 );
}


void sendSmallReward(){
        set_blocking (fd, 0);
		for (int i=0;i<7; i++){
        	write (fd, "RS\n", 3);
		}
        usleep ( (3) * 100 );
}


