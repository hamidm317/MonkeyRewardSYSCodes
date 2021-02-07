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
#include "../coding.h"
#include <stdio.h>

SDL_Texture *content2Texture;
SDL_Texture *content2TextureRect;
SDL_Rect gazeRect[100];
float SDL_LastRefresh = 0;
float RefreshInterval = 1000/85.0;

UINT32 trialStateStartTime;
experimentTypeEnum currentExperimentType;
experimentStateEnum experimentState;
trialStatesEnum trialState;
statePhaseEnum statePhase;
monkeyGazeStateEnum monkeyGazeState;
monkeyFractGazeStateEnum monkeyFractGazeState;
gazeFixResultEnum gazeFixResult;
gazeFixResultEnum gazeFractFixResult;
monkeyFractChoiceEnum monkeyFractChoice;

int photodiodeAck = 0;
int initedPhotodiodeAck = 0;
int sentEventToNeuralData = 0;
int currentSequenceNumberForce;
int currentSequenceNumberChoice;
int currentGoodFractChoices;
int isNextFramePreparedMonkey;
int isNextFrameDrawnMonkey;
int isRedo;
int isPhotodiodeCheckOn = 1;
int reloadedConf;
int randomNumberExperimentType;
int regionOverride = 0;
int gazeXOffset = 0;
int gazeYOffset = 0;
int fractSetToLoad;

int numberOfFractsPerTrial = 5;
int AsAdmin = 0;
int TypeTempVar = 1;
int currentLabel = 0;

char LabelingType[20];

// types of labeling
char Type1[100];
char Type2[100];
char Type3[100];

strcpy(Type1, "Binary_Labeling")
strcpy(Type2, "Amount_Labeling")
strcpy(Type3, "Probable_Labeling")




/***************************** PERFORM AN EXPERIMENTAL TRIAL  ***************/

/* End recording: adds 100 msec of data to catch final events */
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
	//SDL_Texture *bit;
	//SDL_Texture *bit2;
	//SDL_Surface *hbm = SDL_CreateRGBSurface(SDL_SWSURFACE,SCRWIDTH, SCRHEIGHT, dispinfo.bits,0,0,0,0);
		int frameCount = 0;
		int contentUpdateCountre = 0;
		int contentUpdateCountreRect = 0;
	unsigned short currGazeIdx = 0;
	currentSequenceNumber = 0;
	currentSequenceNumberChoice = 0;
	currentSequenceNumberForce = 0;
	currentBatchSequenceNumber = 0;
	currentGoodFractChoices = 0;
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
	monkeyFractGazeState = Monkey_Fract_Gaze_State_HasNotLookedInsideAnySensationWindow;
	gazeFixResult = Gaze_Fix_Result_Undetermined;
	gazeFractFixResult = Gaze_Fract_Fix_Result_Undetermined;
	monkeyFractChoice = Monkey_Fract_Choice_Undetermined;
	SDL_SetRenderTarget(renderer,NULL);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 255);
	SDL_RenderClear(renderer);
	
	isNextFramePreparedMonkey = 0;
	isNextFrameDrawnMonkey = 0;
	reloadedConf = 0;
	totalRun = 0;
	totalRunForce = 0;
	totalRunChoice = 0;
	drawGrid();
	reloadConf();

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
	  if(current_time() > trial_start+20000L)
		{
			eyemsg_printf("TIMEOUT");// message to log the timeout 
			end_trial();             // local function to stop recording
			button = 0;              // trial result message is 0 if timeout 
			break;                   // exit trial loop
		}
	*/
	  /*check for program termination or ALT-F4 or CTRL-C keys*/
	  if (currentSequenceNumber >= numberOfFractals - 1){
		end_trial();
		break;
	  }
	  if(break_pressed())
		{
			end_trial();         /* local function to stop recording */
			return ABORT_EXPT;   /* return this code to terminate experiment */
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

	  if (experimentState == Experiment_State_Init){
		  

		gotoExperimentState(Experiment_State_Prep);	
		gotoState(Trial_State_ITI);
		gotoStatePhase(State_Phase_Enter);
	  }// End of Experiment Init

	  else if (experimentState == Experiment_State_Prep){
		  if (!isRedo && (currentBatchSequenceNumber%numberOfBatchExperiments == 0)) // Also check for reset upon error
		  {
		  	printf("Reached the beginning of new experiment batch");
			writeToEyelink("Reached new batch");
			randomNumberExperimentType = rand()%numberOfBatchExperiments;
			printf("Trial number %d in this batch of %d experiments will be choice",
				       	randomNumberExperimentType+1, numberOfBatchExperiments);
			eyemsg_printf("Trial number %d in this batch of %d experiments will be choice", randomNumberExperimentType+1, numberOfBatchExperiments);
		  }
		  if (currentSequenceNumber%numberOfBatchExperiments == randomNumberExperimentType){
			  currentExperimentType = Experiment_Type_Choice;
			  printf("Current Experiment : Choice");
			  eyemsg_printf("Current Experiment : Choice");
		  }
		  else {
			  currentExperimentType = Experiment_Type_Force;	
			  printf("Current Experiment : Force");
			  eyemsg_printf("Current Experiment : Force");
		  }
		  setRegions();
		  gotoExperimentState(Experiment_State_Run);
	  }

	  else if (experimentState == Experiment_State_Run){
		if (trialState == Trial_State_ITI){
			if (statePhase == State_Phase_Enter){
				printf("ITI Enter\n");
				eyemsg_printf("ITI Enter\n");
				updateFrameMonkey();
				if (currentExperimentType == Experiment_Type_Force){
					sendEventToNeuralData(CODES_FORCE_TRIAL_START);
					writeToEyelink(CODES_FORCE_TRIAL_START);
					printf("sending force start\n");	
				}
				else if (currentExperimentType == Experiment_Type_Choice){
					sendEventToNeuralData(CODES_CHOICE_TRIAL_START);
					writeToEyelink(CODES_CHOICE_TRIAL_START);
					printf("sending choice start\n");
				}
				eraseFixation();
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					eraseSensationWindows();
					drawNextFrameMonkey(Trial_State_Wait_For_Gaze);
					prepareFrameMonkey();
				}
				if (loadFromConf && !reloadedConf){
					reloadConf();
		  			writeConfigToEyelink();
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
					eyemsg_printf("Looked inside Fixation window!\n");
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
				if (currentExperimentType == Experiment_Type_Choice){
					if (APP_VER == 0)
					{
						drawSensationWindowGood();
						drawSensationWindowBad();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				else if (currentExperimentType == Experiment_Type_Force){
					drawSensationWindow();
					if (APP_VER == 0)
					{
						drawSensationWindow();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				//printf("Fix In\n");
				//eyemsg_printf("Fix In\n");
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
				//if (!isNextFrameDrawnMonkey){
					printf("Making Next On frame\n");
					drawNextFrameMonkey(Trial_State_Fract_Overlap);
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
					gotoState(Trial_State_Fract_Overlap);
				}
				else if (hasBrokenFixation()){
					gotoState(Trial_State_Result);

				}
			}

		}// End of Trial_State_Fract_Off

		else if (trialState == Trial_State_Fract_Overlap){
			if (statePhase == State_Phase_Enter){
				updateFrameMonkey();
				//printf("Took %d to update monkey\n", trialStateStartTime - current_time());
				printf("FractOverlap Enter\n");
				eyemsg_printf("FractOverlap Enter\n");
				drawFixationWindow(Color_Fixation_Window_Hold);
				if (currentExperimentType == Experiment_Type_Choice){
					// drawSensationWindowGood();
					// drawSensationWindowBad();
					// drawFracts();

					if (APP_VER == 0)
					{
						drawSensationWindowGood();
						drawSensationWindowBad();
						drawFracts();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				else if (currentExperimentType == Experiment_Type_Force){
					// drawSensationWindow();
					// drawFract();

					if (APP_VER == 0)
					{
						drawSensationWindow();
						drawFracts();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				//printf("Took %d to update experimenter\n", trialStateStartTime - current_time());
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				//printf("Fix In\n");
				//eyemsg_printf("Fix In\n");
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					printf("Making Next Off frame\n");
					//if numoffract reached iti else off
					drawNextFrameMonkey(Trial_State_Wait_For_Saccade);
					//isNextFrameDrawnMonkey = 1;
					prepareFrameMonkey();
					//isNextFramePreparedMonkey = 1;
					//printf("Took %d to update mext monkey\n", trialStateStartTime - current_time());
				}
				if (hasBrokenFixation()){
					printf("Got out for break");
					eyemsg_printf("Got out for break");
					gotoStatePhase(State_Phase_Exit);
					if(isPhotodiodeCheckOn)
						stopPollingForPhotodiodeAck();
				}	
				if (isPhotodiodeCheckOn && !gotPhotodiodeAck() && APP_VER == 0){
                                        //printf("inside photodiode check on\n");
										printf("inited ack before = %d\n", initedPhotodiodeAck);
                                        if (!initedPhotodiodeAck){
                                                initgetPhotodiodeAck();
												//printf("Took %d to init photodiode ack\n",current_time() - trialStateStartTime);
                                        }
										printf("inited ack after = %d\n", initedPhotodiodeAck);
                                        //printf("got back from init ack photodiode\n");
										printf("!gotAck() = %d, !timeout() = %d", !gotPhotodiodeAck(),!timedOut());
                                        while (!gotPhotodiodeAck() && !timedOut()){
                                                //initgetPhotodiodeAck();
                                                //getPhotodiodeAckAndSendEvent(CODES_FORCE_STIM_ONSET[getRegion(randomSequenseNumbers[currentSequenceNumber])]);
                                        //printf("inside while");
                                                getPhotodiodeAck();
												//printf("Took %d to get photodiode ack\n",current_time() - trialStateStartTime);
                                        }
                                        //printf("\noutside while\n");
                                        if (gotPhotodiodeAck()){
				if (currentExperimentType == Experiment_Type_Choice){
					sendEventToNeuralData(CODES_CHOICE_GOOD_STIM_ONSET[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);
				}
				else if (currentExperimentType == Experiment_Type_Force){
					if (APP_VER = 0)
					{
						if (getFractal(randomSequenseNumbersForce[currentSequenceNumberForce]) < 4){
								sendEventToNeuralData(CODES_FORCE_GOOD_STIM_ONSET[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])]);
							}
							else {
								sendEventToNeuralData(CODES_FORCE_BAD_STIM_ONSET[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])]);
							}
					}
				}

                                        stopPollingForPhotodiodeAck();
                                        }
                                }
				if (timedOut()){
                    if (!isPhotodiodeCheckOn || (isPhotodiodeCheckOn && gotPhotodiodeAck())){
                        printf("Got out for timeout");
                        writeToEyelink("Fractal Overlap Timeout Reached");
                        gotoStatePhase(State_Phase_Exit);
                    }
                    else if (isPhotodiodeCheckOn && !gotPhotodiodeAck()){
                        printf("ERROR! FAILED TO GET PHOTODIODE SIGNAL");
                        writeToEyelink("ERROR! FAILED TO GET PHOTODIODE SIGNAL");
                        gotoStatePhase(State_Phase_Exit);
                    }
                }

				/*
				if (timedOut()){
					printf("Got out for timeout");
					eyemsg_printf("Got out for timeout");
					gotoStatePhase(State_Phase_Exit);
				}*/
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
				if (currentExperimentType == Experiment_Type_Choice){
					// drawSensationWindowGood();
					// drawSensationWindowBad();
					// drawFracts();

					if (APP_VER == 0)
					{
						drawSensationWindowGood();
						drawSensationWindowBad();
						drawFracts();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				else if (currentExperimentType == Experiment_Type_Force){
					// drawSensationWindow();
					// drawFract();

					if (APP_VER == 0)
					{
						drawSensationWindow();
						drawFracts();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				if (!isNextFrameDrawnMonkey || !isNextFramePreparedMonkey){
					printf("Making Next Off frame\n");
					//if numoffract reached iti else off
					drawNextFrameMonkey(Trial_State_Fractal_Fix);
					//isNextFrameDrawnMonkey = 1;
					prepareFrameMonkey();
					//isNextFramePreparedMonkey = 1;
				}
				if (hasLookedIntoSensationWindow() && APP_VER == 0){
					printf("Got out for saccade");
					eyemsg_printf("Got out for saccade");
					
					if (currentExperimentType == Experiment_Type_Choice){
						if (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowGood){
							sendEventToNeuralData(CODES_CHOICE_GOOD_SACC_ONSET[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);
						}
						else if (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowBad){
							sendEventToNeuralData(CODES_CHOICE_BAD_SACC_ONSET[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);
						}
					}
					else if (currentExperimentType == Experiment_Type_Force){
						if (getFractal(randomSequenseNumbersForce[currentSequenceNumberForce]) < 4){
                        	sendEventToNeuralData(CODES_FORCE_GOOD_SACC_ONSET[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])]);
						}
                    	else {
                    	    sendEventToNeuralData(CODES_FORCE_BAD_SACC_ONSET[getRegion(randomSequenseNumbersForce
[currentSequenceNumberForce])]);
						}

					}

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

		else if (trialState == Trial_State_Fractal_Fix){
			if (statePhase == State_Phase_Enter){
				updateFrameMonkey();
				printf("FractalFix Enter\n");
				eyemsg_printf("FractalFix Enter\n");
				
				eraseFixation();
				if (currentExperimentType == Experiment_Type_Choice){
					// drawSensationWindowGood();
					// drawSensationWindowBad();
					// drawFracts();

					if (APP_VER == 0)
					{
						drawSensationWindowGood();
						drawSensationWindowBad();
						drawFracts();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				else if (currentExperimentType == Experiment_Type_Force){
					// drawSensationWindow();
					// drawFract();

					if (APP_VER == 0)
					{
						drawSensationWindow();
						drawFracts();
					}
					else if (APP_VER == 1)
					{
						
					}
				}
				gotoStatePhase(State_Phase_Inside);
			}
			else if (statePhase == State_Phase_Inside){
				//printf("Fix In\n");
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
				totalRun++;
				if (currentExperimentType == Experiment_Type_Force)
					totalRunForce++;
				else if (currentExperimentType == Experiment_Type_Choice)
					totalRunChoice++;
				printf("Result Enter\n");
				eyemsg_printf("Result Enter\n");
				printf("Gaze Result From Result Enter: %s\n", (gazeFractFixResult == Gaze_Fract_Fix_Result_KeptGaze) ? "Kept" : ((gazeFractFixResult == Gaze_Fract_Fix_Result_Broke) ? "Broke" : "Undet"));
				if (hasKeptFractFixaton() && APP_VER == 0){
					drawFixationWindow(Color_Fixation_Window_Success);
					//printf("Set batch 0\n");
					//eyemsg_printf("Set batch 0\n");
					isRedo = 0;
					if ((currentExperimentType == Experiment_Type_Force && getFractal(randomSequenseNumbersForce[currentSequenceNumberForce]) < 4) ||
					    (currentExperimentType == Experiment_Type_Choice && hasChosenGoodFract())){
						if (currentExperimentType == Experiment_Type_Choice && hasChosenGoodFract())
							currentGoodFractChoices++;
						printf("\n\n\ninside Reward big\n\n\n");
                        //write(fd, "RL\n", 3);
						sendBigReward();
						sendEventToNeuralData(CODES_REWARD_LARGE);
						writeToEyelink("Good Fractal Reward Delivered");
					}
					else if ((currentExperimentType == Experiment_Type_Force && getFractal(randomSequenseNumbersForce[currentSequenceNumberForce]) < 8) ||
						 (currentExperimentType == Experiment_Type_Choice && hasChosenBadFract())){
						printf("\n\n\ninside Reward small\n\n\n");
                        //write(fd, "RS\n", 3);
						sendSmallReward();
						sendEventToNeuralData(CODES_REWARD_SMALL);
						writeToEyelink("Bad Fractal Reward Delivered");
					}

					playSuccess();
					//sound
					//reward
					//eyelink event
					//blackrock event
				}
				else if (!hasKeptFractFixaton()){
					drawFixationWindow(Color_Fixation_Window_Error);
					playError();
					currentSequenceNumber--;
					if (currentExperimentType == Experiment_Type_Choice)
						currentSequenceNumberChoice--;
					else if (currentExperimentType == Experiment_Type_Force)
						currentSequenceNumberForce--;
					isRedo = 1;
					//currentSequenceNumber -= ((currentSequenceNumber - 1) %numberOfBatchFractals);
					currentBatchSequenceNumber--;
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
				printf("Result Out\n");
				eyemsg_printf("Result Out\n");
				eraseFixation();
				eraseFracts();
				eraseSensationWindows();
				currentSequenceNumber++;
				currentBatchSequenceNumber++;
				if (currentExperimentType == Experiment_Type_Choice)
					currentSequenceNumberChoice++;
				else if (currentExperimentType == Experiment_Type_Force)
					currentSequenceNumberForce++;
				//eraseFixationMonkey();
				//prepareFrameMonkey();
				eyemsg_printf("Current Trial count (Success/Total) :\n");
				eyemsg_printf("Force: %d / %d\n", currentSequenceNumberForce, totalRunForce);
				eyemsg_printf("Choice: %d / %d\n", currentSequenceNumberChoice, totalRunChoice);
				eyemsg_printf("Choice of good fractal: %d/%d\n", currentGoodFractChoices, currentSequenceNumberChoice); 
				eyemsg_printf("Force and Choice: %d / %d\n", currentSequenceNumber, totalRun);
				printf("Current Trial count (Success/Total) :\n");
				printf("Force: %d / %d\n", currentSequenceNumberForce, totalRunForce);
				printf("Choice: %d / %d\n", currentSequenceNumberChoice, totalRunChoice);
				printf("Choice of good fractal: %d/%d\n", currentGoodFractChoices, currentSequenceNumberChoice); 
				printf("Force and Choice: %d / %d\n", currentSequenceNumber, totalRun);
				setNextFrameMonkeyUnprepared();
				setNextFrameMonkeyUndrawn();
				prepareNextTrial();
				gotoStatePhase(State_Phase_Enter);
				gotoState(Trial_State_ITI);
				gotoExperimentState(Experiment_State_Prep);
				
			}

		}// End of Trial_State_Result

	  }// End of Experiment Run

	  else if (experimentState == Experiment_State_Exit){

	  }// End of Experiment Exit

	  //Update Screen
	  if ( SDL_GetTicks() > SDL_LastRefresh + RefreshInterval ){
		  updateFrame();
	  
	  }

		/* BUTTON RESPONSE TEST */
		/* Check for eye-tracker buttons pressed */
		/* This is the preferred way to get response data or end trials	 */
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
	
	//long t;


	/* report response result: 0=timeout, else button number */
	eyemsg_printf("TRIAL_RESULT %d", button);
	/* Call this at the end of the trial, to handle special conditions */
	return check_record_exit();
}


void drawFixation(){ // یک پس زمینه ایجاد می‌کند و بعد مستطیل مربوط به فیکسیشن را می‌سازد!
	SDL_SetRenderTarget(renderer2, Texture_Fixation);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Fixation.r, Color_Fixation.g, Color_Fixation.b, Color_Fixation.a);
	SDL_RenderFillRect(renderer2, &Rect_Fixation);
}

void drawFixationMonkey(){ // از قرار معلوم، همان کار بالا را برای مانیتور دیگر می‌کند؟
	eyemsg_printf("Fixation Rect x,y,w,h = %d,%d,%d,%d", Rect_Fixation_Monkey.x,
			Rect_Fixation_Monkey.y, Rect_Fixation_Monkey.w, Rect_Fixation_Monkey.h);
	SDL_SetRenderTarget(renderer, Texture_Fixation_Monkey);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Fixation.r, Color_Fixation.g, Color_Fixation.b, Color_Fixation.a);
	SDL_RenderFillRect(renderer, &Rect_Fixation_Monkey);
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
	else if (state == Trial_State_Fract_Overlap){
		drawFixationMonkey();
		if (currentExperimentType == Experiment_Type_Choice)
			drawFractsMonkey();
		else if (currentExperimentType == Experiment_Type_Force)
			drawFractMonkey();
                if (isPhotodiodeCheckOn)
                        drawPhotodiodeMonkey();
	}
	else if (state == Trial_State_Wait_For_Saccade){
		eraseFixationMonkey();
		if (currentExperimentType == Experiment_Type_Choice)
			drawFractsMonkey();
		else if (currentExperimentType == Experiment_Type_Force)
			drawFractMonkey();
                erasePhotodiodeMonkey();

	}
	else if (state == Trial_State_Fractal_Fix){
		eraseFixationMonkey();
		if (currentExperimentType == Experiment_Type_Choice)
			drawFractsMonkey();
		else if (currentExperimentType == Experiment_Type_Force)
			drawFractMonkey();
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
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	//isNextFrameDrawnMonkey = 0;
	//isNextFramePreparedMonkey = 0;
}

void drawFixationWindow(SDL_Color color){
	//SDL_Rect r;
	//r = getTransformedRect(Rect_Fixation_Window);
	SDL_Rect r;
	r = getUntransformedRect(Rect_Fixation_Window);
	eyemsg_printf("Fixation Window Rect x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
	SDL_SetRenderTarget(renderer2, Texture_Fixation_Window);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, color.r, color.b, color.g, color.a);
	SDL_RenderDrawRect(renderer2, &Rect_Fixation_Window);
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

void drawSensationWindow(){
        SDL_Rect r;
        SDL_Rect r_msg;
        SDL_Color c;
       //       = {0,0,0,0};
        r = getSensationRect();
	r_msg = getUntransformedRect(r);
	eyemsg_printf("Sensation Window Rect x,y,w,h = %d,%d,%d,%d", r_msg.x, r_msg.y, r_msg.w, r_msg.h);
	eyemsg_printf("Fractal Type = %s", (getFractal(randomSequenseNumbersForce[currentSequenceNumberForce]) < 4) ? "Good" : "Bad");
        c = (getFractal(randomSequenseNumbersForce[currentSequenceNumberForce]) < 4) ? Color_Fixation_Window_Success : Color_Fixation_Window_Error;
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
        SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
        SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
        SDL_RenderClear(renderer2);
        SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer2, c.r,c.g,c.b,c.a);
        SDL_RenderDrawRect(renderer2, &r);
}


void drawSensationWindowGood(){

	SDL_Rect r;
	SDL_Rect r_msg;
	SDL_Color c;
	c = Color_Fixation_Window_Success;
       //	= {0,0,0,0};
	r = getSensationRectGood();
	r_msg = getUntransformedRect(r);
	eyemsg_printf("Good Sensation Window Rect x,y,w,h = %d,%d,%d,%d", r_msg.x, r_msg.y, r_msg.w, r_msg.h);
	//c = (getFractal(randomSequenseNumbers[currentSequenceNumber]) < 4) ? Color_Fixation_Window_Success : Color_Fixation_Window_Error;
//	SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
//	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
//	SDL_RenderClear(renderer2);
	//printf("Good R: x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, c.r,c.g,c.b,255);
	SDL_RenderDrawRect(renderer2, &r);
}

///////////////////////////////////////////////////// UpDaTeS By HT_OSRU //////////////////////////////////////////////////////

void HT_OSRU_drawSensationWindowLabeled(int Label){

	SDL_Rect r;
	SDL_Rect r_msg;
	SDL_Color c;
	c = Color_Fixation_Window_Success;
       //	= {0,0,0,0};
	r = HT_OSRU_getSensationRectLabeled(Label);
	r_msg = getUntransformedRect(r);
	eyemsg_printf("Sensation Window Rect type: (%d, %d) x,y,w,h = %d,%d,%d,%d", TypeTempVar, Label, r_msg.x, r_msg.y, r_msg.w, r_msg.h);
	//c = (getFractal(randomSequenseNumbers[currentSequenceNumber]) < 4) ? Color_Fixation_Window_Success : Color_Fixation_Window_Error;
//	SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
//	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
//	SDL_RenderClear(renderer2);
	//printf("Good R: x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, c.r,c.g,c.b,255);
	SDL_RenderDrawRect(renderer2, &r);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void drawSensationWindowBad(){

	SDL_Rect r;
	SDL_Rect r_msg;
	SDL_Color c;
	c = Color_Fixation_Window_Error;
       //	= {0,0,0,0};
	r = getSensationRectBad();
	r_msg = getUntransformedRect(r);
	eyemsg_printf("Bad Sensation Window Rect x,y,w,h = %d,%d,%d,%d", r_msg.x, r_msg.y, r_msg.w, r_msg.h);
	//c = (getFractal(randomSequenseNumbers[currentSequenceNumber]) < 4) ? Color_Fixation_Window_Success : Color_Fixation_Window_Error;
//	SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
//	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
//	SDL_RenderClear(renderer2);
	//printf("Bad R: x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, c.r,c.g,c.b,255);
	SDL_RenderDrawRect(renderer2, &r);
}


void eraseSensationWindows(){
	SDL_SetRenderTarget(renderer2, Texture_Sensation_Window);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
}
	/*
	printf("randomSeq: ");
	for (int i=0; i<16; i++){
		printf("%d, ", randomSequenseNumbers[i]);
	}
	printf("\n");
	printf("randomSeqiReg: ");
	for (int i=0; i<16; i++){
		printf("%d, ", randomSequenseNumbersRegions[i]);
	}
	printf("\n");
	printf("GetGood Fract:");
	for (int i=0; i<16; i++){
		printf("%d, ", getFractalGood(randomSequenseNumbersRegions[i]));
	}
	printf("\n");
	printf("GetGood Reg:");
	for (int i=0; i<16; i++){
		printf("%d, ", getRegionGood(randomSequenseNumbersRegions[i]));
	}
	printf("\n");
	printf("GetBad Fract:");
	for (int i=0; i<16; i++){
		printf("%d, ", getFractalBad(randomSequenseNumbersRegions[i]));
	}
	printf("\n");
	printf("GetBad Reg:");
	for (int i=0; i<16; i++){
		printf("%d, ", getRegionBad(randomSequenseNumbersRegions[i]));
	}
	printf("\n");



	printf("Good: Pos: %d, Fract: %d", getRegionGood(randomSequenseNumbersRegions[currentSequenceNumber]), getFractalGood(randomSequenseNumbers[currentSequenceNumber]));
	printf("Bad: Pos: %d, Fract: %d", getRegionBad(randomSequenseNumbersRegions[currentSequenceNumber]), getFractalBad(randomSequenseNumbers[currentSequenceNumber]));
	*/

void drawFract(){
        SDL_SetRenderTarget(renderer2, Texture_Fractal);
        SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
        SDL_RenderClear(renderer2);
        SDL_RenderCopy(renderer2, Texture_Fractals[getFractal(randomSequenseNumbersForce[currentSequenceNumberForce])], NULL, &Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])]);

}


void drawFracts(){
	SDL_SetRenderTarget(renderer2, Texture_Fractal);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);	
	SDL_RenderCopy(renderer2, Texture_Fractals[getFractalGood(randomSequenseNumbersChoice[currentSequenceNumberChoice])], NULL, &Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);
	SDL_RenderCopy(renderer2, Texture_Fractals[getFractalBad(randomSequenseNumbersChoice[currentSequenceNumberChoice])], NULL, &Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);

}

void HT_OSRU_drawFracts(){ // FIX REQUIRED
	SDL_SetRenderTarget(renderer2, Texture_Fractal);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);	


}

void eraseFracts(){
	SDL_SetRenderTarget(renderer2, Texture_Fractal);
	SDL_SetRenderDrawBlendMode(renderer2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer2, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer2);
}

void drawFractsMonkey(){
	SDL_Rect rg;
	SDL_Rect rb;
	rg = Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])];
	rb = Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])];
	eyemsg_printf("Good Fractal %d is set for region %d with x,y,w,h = %d,%d,%d,%d",
			getFractalGood(randomSequenseNumbersChoice[currentSequenceNumberChoice]),
			getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice]),
			rg.x, rg.y, rg.w, rg.h);
	eyemsg_printf("Bad Fractal %d is set for region %d with x,y,w,h = %d,%d,%d,%d",
			getFractalBad(randomSequenseNumbersChoice[currentSequenceNumberChoice]),
			getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice]),
			rb.x, rb.y, rb.w, rb.h);
	SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, Texture_Fractals_Monkey[getFractalGood(randomSequenseNumbersChoice[currentSequenceNumberChoice])], NULL, &Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);
	SDL_RenderCopy(renderer, Texture_Fractals_Monkey[getFractalBad(randomSequenseNumbersChoice[currentSequenceNumberChoice])], NULL, &Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);

}

void drawFractMonkey(){
	SDL_Rect r = Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])];
	eyemsg_printf("Fractal %d is set for region %d with x,y,w,h = %d,%d,%d,%d",
			getFractal(randomSequenseNumbersForce[currentSequenceNumberForce]), 
			getRegion(randomSequenseNumbersForce[currentSequenceNumberForce]),
			r.x, r.y, r.w, r.h);

	SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, Color_Background.r, Color_Background.g, Color_Background.b, 0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, Texture_Fractals_Monkey[getFractal(randomSequenseNumbersForce[currentSequenceNumberForce])], NULL, &Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])]);
}



void eraseFractsMonkey(){
	SDL_SetRenderTarget(renderer, Texture_Fractal_Monkey);
	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
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
       	SDL_LastRefresh = SDL_GetTicks();
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
	SDL_RenderCopy(renderer, Texture_Fractal_Monkey, NULL, &bound3);
	SDL_RenderCopy(renderer, Texture_Fixation_Monkey, NULL, &bound3);
	SDL_RenderCopy(renderer, Texture_Photodiode_Monkey, NULL, &bound1);
	isNextFramePreparedMonkey = 1;

}

void updateFrameMonkey(){
	SDL_SetRenderTarget(renderer,NULL);
	SDL_RenderPresent(renderer);
}
/*
void writeToBlackrock(char *msg){
	char msgToSend[30];
	strcpy(msgToSend, msg);
	strcat(msgToSend, "\n");
	write(fd, msgToSend, sizeof(msgToSend)/sizeof(char));
}
*/

void writeToEyelink(char *msg){
        char msgToSend[200];
        strcpy(msgToSend, "EE ");
        strcat(msgToSend, msg);
        eyemsg_printf(msgToSend);
}

void playError(){
        char errAmp[10];
        sprintf(errAmp, "%f", errorVolume);
        char playstr[200];
        strcpy(playstr, "play -q -v  ");
        strcat(playstr, errAmp);
        strcat(playstr, " Error.wav &");
        system(playstr);
        writeToEyelink("Sound Error Played");
}

void playSuccess(){
        char sucAmp[10];
        sprintf(sucAmp,"%f", successVolume);
        char playstr[200];
        strcpy(playstr, "play -q -v ");
        strcat(playstr, sucAmp);
        strcat(playstr, " Success.wav &");
        system(playstr);
        writeToEyelink("Sound Success Played");
}


void gotoExperimentState(experimentStateEnum state){
	experimentState = state;	  
}

void gotoState(trialStatesEnum state){
	trialStateStartTime = current_time();
	statePhase = State_Phase_Enter;
	trialState = state;
	printf("Goto to state %d at time %d\n", state, trialStateStartTime);
}

void gotoStatePhase (statePhaseEnum state){
	statePhase = state;
}

int timedOut(){
//	printf("curr = %d, start = %d, state = %d,  exp = %d, start + exp = %d \n",current_time(), trialStateStartTime, trialState, trialStates[trialState].expirationTime, trialStateStartTime + trialStates[trialState].expirationTime);
	if (current_time() >= trialStateStartTime + trialStates[trialState].expirationTime)
	{
		printf("Expired after %d\n", current_time() - trialStateStartTime);
		return 1;
	}
		return 0;
}

void prepareNextTrial(){
	monkeyGazeState = Monkey_Gaze_State_HasNotLookedInsideFixationWindow;
	gazeFixResult = Gaze_Fix_Result_Undetermined;
	gazeFractFixResult = Gaze_Fract_Fix_Result_Undetermined;
	monkeyFractGazeState = Monkey_Fract_Gaze_State_HasNotLookedInsideAnySensationWindow;
	monkeyFractChoice = Monkey_Fract_Choice_Undetermined;
	reloadedConf = 0;
    photodiodeAck = 0;
    initedPhotodiodeAck = 0;
    sentEventToNeuralData = 0;

	//Clean ups	
}

int isInsideFixationWindow(){
	if (Rect_Gaze.x + Rect_Gaze.w/2 > Rect_Fixation_Window.x &&
	    Rect_Gaze.x + Rect_Gaze.w/2 < Rect_Fixation_Window.x + Rect_Fixation_Window.w &&
		
    	    Rect_Gaze.y + Rect_Gaze.h/2 > Rect_Fixation_Window.y &&
	    Rect_Gaze.y + Rect_Gaze.h/2 < Rect_Fixation_Window.y + Rect_Fixation_Window.h)
		return 1;
	return 0;
}

//// what should have been done by HT_OSRU

int isInsideSensationWindow(){
        SDL_Rect r;
        r = getTransformedRect(Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])]);
        if (((Rect_Gaze.x + Rect_Gaze.w/2) > (r.x - ((Rect_Sensation.w - r.w)/2))) &&
            ((Rect_Gaze.x + Rect_Gaze.w/2) < (r.x + r.w + ((Rect_Sensation.w - r.w)/2))) &&
            ((Rect_Gaze.y + Rect_Gaze.h/2) > (r.y - ((Rect_Sensation.h - r.h)/2))) &&
            ((Rect_Gaze.y + Rect_Gaze.h/2) < (r.y + r.h +((Rect_Sensation.h - r.h)/2))))
                return 1;
        return 0;
}

int isInsideSensationWindowGood(){
	SDL_Rect r;
	r = getTransformedRect(Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);
	if (((Rect_Gaze.x + Rect_Gaze.w/2) > (r.x - ((Rect_Sensation.w - r.w)/2))) &&			
	    ((Rect_Gaze.x + Rect_Gaze.w/2) < (r.x + r.w + ((Rect_Sensation.w - r.w)/2))) &&
    	    ((Rect_Gaze.y + Rect_Gaze.h/2) > (r.y - ((Rect_Sensation.h - r.h)/2))) && 
	    ((Rect_Gaze.y + Rect_Gaze.h/2) < (r.y + r.h +((Rect_Sensation.h - r.h)/2)))) 
		return 1;
	return 0;
}

int isInsideSensationWindowBad(){
	SDL_Rect r;
	r = getTransformedRect(Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])]);
	if (((Rect_Gaze.x + Rect_Gaze.w/2) > (r.x - ((Rect_Sensation.w - r.w)/2))) &&			
	    ((Rect_Gaze.x + Rect_Gaze.w/2) < (r.x + r.w + ((Rect_Sensation.w - r.w)/2))) &&
    	    ((Rect_Gaze.y + Rect_Gaze.h/2) > (r.y - ((Rect_Sensation.h - r.h)/2))) && 
	    ((Rect_Gaze.y + Rect_Gaze.h/2) < (r.y + r.h +((Rect_Sensation.h - r.h)/2)))) 
		return 1;
	return 0;
}

void processGaze(){
	if(trialState == Trial_State_Wait_For_Gaze){
		if (monkeyGazeState == Monkey_Gaze_State_HasNotLookedInsideFixationWindow &&
		   isInsideFixationWindow())
			monkeyGazeState = Monkey_Gaze_State_HasLookedInsideFixationWindow;
	}
	else if (trialState == Trial_State_Fixation_Fix || trialState == Trial_State_Fract_Overlap){
		if (!isInsideFixationWindow()){
			gazeFixResult = Gaze_Fix_Result_Broke;
			printf("Broken now!");
		}
	}
	else if (trialState == Trial_State_Wait_For_Saccade){
		if (currentExperimentType == Experiment_Type_Choice){
			if (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasNotLookedInsideAnySensationWindow &&
			   isInsideSensationWindowGood()){
				eyemsg_printf("Has looked sensation win good\n");	
				printf("Toggled to Has looked sensation win good\n");	
				setMonkeyFractChoiceGood();
				monkeyFractGazeState = Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowGood;
			}
			if (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasNotLookedInsideAnySensationWindow &&
			   isInsideSensationWindowBad()){
				eyemsg_printf("Has looked sensation win bad\n");	
				printf("Toggled to Has looked sensation win bad\n");	
				setMonkeyFractChoiceBad();
				monkeyFractGazeState = Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowBad;
			}
		}
		else if (currentExperimentType == Experiment_Type_Force){
			if (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasNotLookedInsideAnySensationWindow&
                   isInsideSensationWindow()){
                    	    eyemsg_printf("Has looked sensation win\n");
			    monkeyFractGazeState = Monkey_Fract_Gaze_State_HasLookedInsideSensationWindow;
         	       }

		}

	}
	else if (trialState == Trial_State_Fractal_Fix){
		if (currentExperimentType == Experiment_Type_Choice){
			if ( monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowGood &&
			     !isInsideSensationWindowGood()){
				gazeFractFixResult = Gaze_Fract_Fix_Result_Broke;	
			}
			if ( monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowBad &&
			     !isInsideSensationWindowBad()){
				gazeFractFixResult = Gaze_Fract_Fix_Result_Broke;	
			}
		}
		else if (currentExperimentType == Experiment_Type_Force){
			if (!isInsideSensationWindow()){
                        	gazeFractFixResult = Gaze_Fract_Fix_Result_Broke;
                	}
		}
	}	
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


int hasLookedIntoSensationWindow(){
	if (currentExperimentType == Experiment_Type_Choice &&
	   (monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowGood ||
	    monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindowBad))
		return 1;
	if (currentExperimentType == Experiment_Type_Force &&
	    monkeyFractGazeState == Monkey_Fract_Gaze_State_HasLookedInsideSensationWindow)
		return 1;
	return 0;
}

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

void setFractFixationSuccess(){ /////////////// Should be revised HT_OSRU //////////////
	gazeFractFixResult = Gaze_Fract_Fix_Result_KeptGaze;
}

void setMonkeyFractChoiceGood(){ /////////////// Should be revised HT_OSRU //////////////
	if (currentExperimentType == Experiment_Type_Choice)
		monkeyFractChoice = Monkey_Fract_Choice_Good;
}

void setMonkeyFractChoiceBad(){ /////////////// Should be revised HT_OSRU //////////////
	if (currentExperimentType == Experiment_Type_Choice)
		monkeyFractChoice = Monkey_Fract_Choice_Bad;
}

int hasChosenGoodFract(){ /////////////// Should be revised HT_OSRU //////////////
	if (monkeyFractChoice == Monkey_Fract_Choice_Good)
		return 1;
	return 0;
}

int hasChosenBadFract(){ /////////////// Should be revised HT_OSRU //////////////
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

int getRegion(int n){
	if (regionOverride != 0){
		return (regionOverride-1);
	}
        return n/8;
}

int getFractal(int n){
        return n%8;
}

int getRegionGood(int n){
	if (regionOverride != 0){
		return (regionOverride-1);
	}
	return n%8;
}

int getRegionBad(int n){
	if (regionOverride != 0){
		return (regionOverride+3);
	}
	return (4+getRegionGood(n))%8;
	//return ((4 - getRegionGood(n)) > 0) ? (4 - getRegionGood(n)) : (4 + getRegionGood(n));
}


/////////////////////////////////////////////// UpDaTeS By HT_OSRU ////////////////////////////////////////////////////

int HT_OSRU_getRegion(int n){
	if (regionOverride != 0){
		return (regionOverride-1);
	}
        return n / numberOfFractsPerTrial;
}

int HT_OSRU_getFractal(int n){
        return n % numberOfFractsPerTrial;
}

int HT_OSRU_getRegionByLabel(int n, int Label)
{

    TypeTempVar = TypeTempVar();

    switch (TypeTempVar)
    {
        case 1: //Type is Binary Labeling and we set "Good" to first half and "Bad" to the others.
            if (Label == 0)
			{
				if (regionOverride != 0)
				{
					return (regionOverride + numberOfFractsPerTrial / 2 + 1);
				}

				return (numberOfFractsPerTrial / 2 + n % numberOfFractsPerTrial)) % numberOfFractsPerTrial;
			}
			else if (Label == 1)
			{
				if (regionOverride != 0)
				{
					return (regionOverride - 1);
				}

				return n % numberOfFractsPerTrial;
			}
            break;

        case 2: //Type is amount labeling and we set a range of 0 to 100 for this labels.
            if (regionOverride != 0)
				{
					return (regionOverride - 1);
				}

			return n % numberOfFractsPerTrial;
            break;

        case 3: //Type is probable labeling and we set a range of 0 to 100 for this labels. (Difference of this part and previous part is in rewarding part)
            if (regionOverride != 0)
				{
					return (regionOverride - 1);
				}

			return n % numberOfFractsPerTrial;
            break;
        default:;
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////// UpDaTeS By HT_OSRU ////////////////////////////////////////////////////

int HT_OSRU_TypeVar()
{
    TypeTempVar = 1; // set labeling type default to Binary mode
    if (strcmp(Type1, LabelingType) == 0)
    {
        TypeTempVar = 1;
    }
    else if (strcmp(Type2, LabelingType) == 0)
    {
        TypeTempVar = 2;
    }
    else if (strcmp(Type3, LabelingType) == 0)
    {
        TypeTempVar = 3;
    }

	return TypeTempVar;
}


int HT_OSRU_getFractalByLabel(int n, int label)
{
	
    switch (TypeTempVar)
    {
        case 1: //Type is Binary Labeling for n fractal in set factal 1 to floor(n / 2) is good and floor(n / 2) + 1 to n is bad
            if (Label == 0)
			{
				
				return n / (numberOfFractsPerTrial / 2);

			}
			else if (Label == 1)
			{
				
				return numberOfFractsPerTrial / 2 + n % numberOfFractsPerTrial;
				
			}
            break;

        case 2: //Type is amount labeling and we set a range of 0 to 100 for this labels.
            
			return n % numberOfFractsPerTrial;

            break;

        case 3: //Type is probable labeling and we set a range of 0 to 100 for this labels. (Difference of this part and previous part is in rewarding part)
            
			return n % numberOfFractsPerTrial;

            break;
        default:;
            break;
    }
}

// SDL_Rect getSensationRectByLabel(){
// 	SDL_Rect r;
// 	SDL_Rect rect;
// 	r.x = (Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].x -
//              ((Rect_Sensation.w - Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].w)/2)); 
// 	r.y = (Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].y -
//              ((Rect_Sensation.h - Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].h)/2));
// 	r.w = Rect_Sensation.w;
// 	r.h = Rect_Sensation.h;
// 	eyemsg_printf("Current Good Sensation Window x,y,w,h = %d,%d,%d,%d\n", r.x, r.y, r.w, r.h);
// 	eyemsg_printf("Current Good Fractal Region = %d", getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice]));
// 	rect = getTransformedRect(r);
// 	return rect;
// }

SDL_Rect HT_OSRU_getSensationRectLabeled(int Label){
        SDL_Rect r;
        SDL_Rect rect;
        r.x = (Rect_Regions[HT_OSRU_getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].x -
                                 ((Rect_Sensation.w - Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].w)/2));
        r.y = (Rect_Regions[HT_OSRU_getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].y -
                                 ((Rect_Sensation.h - Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].h)/2));
        r.w = Rect_Sensation.w;
        r.h = Rect_Sensation.h;
	eyemsg_printf("Current Sensation type: (%d, %d) Window x,y,w,h = %d,%d,%d,%d", TypeTempVar, Label, r.x, r.y, r.w, r.h);
        rect = getTransformedRect(r);
        return rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int getFractalGood(int n){ /////////////// Should be revised HT_OSRU //////////////
	return n/4;
}

int getFractalBad(int n){ /////////////// Should be revised HT_OSRU //////////////
	return 4+(n%4);
}
SDL_Rect getSensationRectGood(){ /////////////// Should be revised HT_OSRU ///////////////
	SDL_Rect r;
	SDL_Rect rect;
	r.x = (Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].x -
             ((Rect_Sensation.w - Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].w)/2)); 
	r.y = (Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].y -
             ((Rect_Sensation.h - Rect_Regions[getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].h)/2));
	r.w = Rect_Sensation.w;
	r.h = Rect_Sensation.h;
	eyemsg_printf("Current Good Sensation Window x,y,w,h = %d,%d,%d,%d\n", r.x, r.y, r.w, r.h);
	eyemsg_printf("Current Good Fractal Region = %d", getRegionGood(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice]));
	rect = getTransformedRect(r);
	return rect;
}

SDL_Rect getSensationRectBad(){ /////////////// Should be revised HT_OSRU //////////////
	SDL_Rect r;
	SDL_Rect rect;
	r.x = (Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].x -
                                 ((Rect_Sensation.w - Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].w)/2)); 
	r.y = (Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].y -
                                 ((Rect_Sensation.h - Rect_Regions[getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice])].h)/2));
	r.w = Rect_Sensation.w;
	r.h = Rect_Sensation.h;
	eyemsg_printf("Current Bad Sensation Window x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
	eyemsg_printf("Current Bad Fractal Region = %d", getRegionBad(randomSequenseNumbersRegionsChoice[currentSequenceNumberChoice]));
	rect = getTransformedRect(r);
	return rect;
}

SDL_Rect getSensationRect(){ /////////////// Should be revised HT_OSRU //////////////
        SDL_Rect r;
        SDL_Rect rect;
        r.x = (Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].x -
                                 ((Rect_Sensation.w - Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].w)/2));
        r.y = (Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].y -
                                 ((Rect_Sensation.h - Rect_Regions[getRegion(randomSequenseNumbersForce[currentSequenceNumberForce])].h)/2));
        r.w = Rect_Sensation.w;
        r.h = Rect_Sensation.h;
	eyemsg_printf("Current Sensation Window x,y,w,h = %d,%d,%d,%d", r.x, r.y, r.w, r.h);
        rect = getTransformedRect(r);
        return rect;
}


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

/*

				-iti <iti time>. currently %d \n", trialStates[Trial_State_ITI].expirationTime);
                        printf("\t -gazewait <time to wait for gaze>. currently %d \n", trialStates[Trial_State_Wait_For_Gaze].expirationTime);
                        printf("\t -fixfix <time to fixate on fixation point>. currently %d \n", trialStates[Trial_State_Fixation_Fix].expirationTime);
                        printf("\t -overlap <overlap time of fixation and fractal(s)>. currently %d \n", trialStates[Trial_State_Fract_Overlap].expirationTime);
                        printf("\t -sacwait <time to wait for saccade>. currently %d \n", trialStates[Trial_State_Wait_For_Saccade].expirationTime);
                        printf("\t -fractfix <time to fixate on fractal>. currently %d \n", trialStates[Trial_State_Fractal_Fix].expirationTime);



*/

int reloadConf(){
	FILE *fp;
	char str[200];
	fp = fopen("FCConf.txt", "r");
	if (!fp){
		printf("Failed to open file");
		return -1;
	}
	//while (!feof(fp)){
	while (fgets(str, 200, fp)){
		//fgets(str, 80, fp);
		if (strstr(str, "iti")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("iti: %d\n", parsedInt);
			trialStates[Trial_State_ITI].expirationTime = parsedInt;
		}
		if (strstr(str, "gazewait")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("gazewait: %d\n", parsedInt);
			trialStates[Trial_State_Wait_For_Gaze].expirationTime = parsedInt;
		}
		if (strstr(str, "fixfix")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("fixfix: %d\n", parsedInt);
			trialStates[Trial_State_Fixation_Fix].expirationTime = parsedInt;
		}
		if (strstr(str, "overlap")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("overlap: %d\n", parsedInt);
			trialStates[Trial_State_Fract_Overlap].expirationTime = parsedInt;
		}
		if (strstr(str, "sacwait")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("fracton: %d\n", parsedInt);
			trialStates[Trial_State_Wait_For_Saccade].expirationTime = parsedInt;
		}
		if (strstr(str, "fractfix")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("fractfix: %d\n", parsedInt);
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
			//Rect_Regions[0] = Rect_Fixation_Window;
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
		if (strstr(str, "PhotodiodeCheck")!= NULL){
                        char parsedStr[200];
                        int parsedInt;
                        parsedInt = atoi(strstr(str, "=") + 1);
                        printf("Photodiode Check: %d\n", parsedInt);
                        isPhotodiodeCheckOn = parsedInt;
                }
		if (strstr(str, "RegionOverride")!= NULL){
			char parsedStr[200];
			int parsedInt;
			parsedInt = atoi(strstr(str, "=") + 1);	
			//printf("RegionOverride: %d\n", parsedInt);
			if (parsedInt>=0 && parsedInt<=32){
				 regionOverride = parsedInt;
			}
			else if (parsedInt != 0){
				printf("INVALID CONFIGURATION FOR REGION OVERRIDE, IGNORING...\n");
			}
		}
		if (strstr(str, "GazeXOffset")!= NULL){
                        char parsedStr[200];
                        double parsedDouble;
                        parsedDouble = atof(strstr(str, "=") + 1);
                        gazeXOffset  = (int)degToPixel(1.0*parsedDouble);
			printf("gazeXOffset = %d\n", gazeXOffset);
                }
                if (strstr(str, "GazeYOffset")!= NULL){
                        char parsedStr[200];
                        double parsedDouble;
                        parsedDouble = atof(strstr(str, "=") + 1);
                        gazeYOffset  = (int)degToPixel(1.0*parsedDouble);
			printf("gazeYOffset = %d\n", gazeYOffset);
                }
		
		///////////////////////////////////////// UpDaTeS bY HT_OSRU ////////////////////////////////////////
		
		// if (strstr(str, "LabelingType") != NULL)
		// {

		// 	char parsedStr[20];
		// 	parsedStr = strstr(str, "=");
		// 	strcpy(LabelingType, parsedStr);

		// }
		
		if (strstr(str, "numberOfFractPerTrial") != NULL)
		{

			int parsedInt;
			parsedInt = atoi(strstr(str, "="));
			numberOfFractsPerTrial = parsedInt;

		}

		if (strstr(str, "APP_VER") != NULL)
		{

			int parsedInt;
			parsedInt = atoi(strstr(str, "="));
			APP_VER = parsedInt;

		}

		if (strstr(str, "AsAdmin") != NULL)
		{

			int parsedInt;
			parsedInt = atoi(strstr(str, "="));
			AsAdmin = parsedInt;

		}		

	}
	fclose(fp);
	reloadedConf = 1;
	return 1;
}

void setRegions(){
	Regions_Center_X = bound3.w/2;
	Regions_Center_Y = bound3.h/2;
	double ANGLE = M_PI_4;

	for (int i=0; i<1; i++){
		for (int j=0; j<8; j++){
			Rect_Regions[8*i+j].w = Regions_Width;
			Rect_Regions[8*i+j].h = Regions_Height;
			Rect_Regions[8*i+j].x = Regions_Center_X +
				(i+1)* Regions_Inter_Ring_Distance * (cos(j*ANGLE + Regions_Offset* M_PI/180)) -
				Regions_Width/2;
			Rect_Regions[8*i+j].y = Regions_Center_Y +
				(i+1)* Regions_Inter_Ring_Distance * (sin(j*ANGLE + Regions_Offset* M_PI/180)) -
			Regions_Height/2;
			eyemsg_printf("region[%d].x,y,w,h = %d, %d\n",i, Rect_Regions[j].x, Rect_Regions[j].y, Rect_Regions[j].w, Rect_Regions[j].h);
		}
	}
}

///////////////////////////////////////////// UpDaTeS By HT_OSRU //////////////////////////////////////////////

void HT_OSRU_setRegions(){
	Regions_Center_X = bound3.w/2;
	Regions_Center_Y = bound3.h/2;
	double ANGLE = M_PI_4;

	for (int i=0; i < 1; i++){
		for (int j=0; j < numberOfFractsPerTrial; j++){
			Rect_Regions[numberOfFractsPerTrial * i + j].w = Regions_Width;
			Rect_Regions[numberOfFractsPerTrial * i + j].h = Regions_Height;
			Rect_Regions[numberOfFractsPerTrial * i + j].x = Regions_Center_X +
				(i+1)* Regions_Inter_Ring_Distance * (cos(j*ANGLE + Regions_Offset* M_PI/180)) -
				Regions_Width/2;
			Rect_Regions[numberOfFractsPerTrial * i + j].y = Regions_Center_Y +
				(i+1)* Regions_Inter_Ring_Distance * (sin(j*ANGLE + Regions_Offset* M_PI/180)) -
			Regions_Height/2;
			eyemsg_printf("region[%d].x,y,w,h = %d, %d\n",i, Rect_Regions[j].x, Rect_Regions[j].y, Rect_Regions[j].w, Rect_Regions[j].h);
		}
	}
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
		rv.w = (i%5==0) ? 2 : 1;
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
}

int degToPixel(double degrees){
	//printf("dispPixelPerDeg (%f) * deg (%f) = %f\n", dispPixelPerDeg, degrees, degrees* dispPixelPerDeg);
	return (int)((double)degrees * dispPixelPerDeg); 
}

double degToCm(double degrees){
	printf("degrees (%f) / dispDegPerCm (%f) = %f\n", degrees, dispDegPerCm, degrees/dispDegPerCm);
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

void writeConfigToEyelink(){ ///////////////////////// Should Be revised HT_OSRU /////////////////////////
	eyemsg_printf(" -numoftrials  %d ", numberOfBatchExperiments);
	eyemsg_printf(" -set  %d ", fractSetToLoad);
	eyemsg_printf(" -iti  %d ", trialStates[Trial_State_ITI].expirationTime);
	eyemsg_printf(" -gazewait  %d ", trialStates[Trial_State_Wait_For_Gaze].expirationTime);
	eyemsg_printf(" -fixationfix  %d ", trialStates[Trial_State_Fixation_Fix].expirationTime);
	eyemsg_printf(" -overlap  %d ", trialStates[Trial_State_Fract_Overlap].expirationTime);
	eyemsg_printf(" -sacwait  %d ", trialStates[Trial_State_Wait_For_Saccade].expirationTime);
	eyemsg_printf(" -fractfix  %d ", trialStates[Trial_State_Fractal_Fix].expirationTime);
        eyemsg_printf(" -fixsize  %d ", Rect_Fixation_Monkey.w);
        eyemsg_printf(" -fixwin  %d ", Rect_Fixation_Window.w);
        eyemsg_printf(" -sensewin  %d ", Rect_Sensation.w);
        eyemsg_printf(" -radius  %d ", Regions_Inter_Ring_Distance);
        eyemsg_printf(" -fractsize  %d ", Regions_Width);
	eyemsg_printf(" -photodiode = %d", isPhotodiodeCheckOn);
	eyemsg_printf(" -gazexoffset = %d ", gazeXOffset);
        eyemsg_printf(" -gazeyoffset = %d ", gazeYOffset);

}

int gotPhotodiodeAck(){
        return photodiodeAck;
}

void initgetPhotodiodeAck(){
        set_blocking (fd, 0);
		for (int i=0;i<7; i++){
	        write(fd, CODE_PHOTODIODE_REQ, sizeof(CODE_PHOTODIODE_REQ)-1);
    	    usleep ( (2) * 100 ); 
		}
        initedPhotodiodeAck = 1;
		printf("inited photodiode ack\n");
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
        //photodiodeAck = (ack[0] == 80 ? 1:0);
		//printf("Ack = %d \n",photodiodeAck);
        //int millis = clock();
        //while (photodiodeAck == 0 && millis + 100 > clock()){
        //      printf("milis = %d , clock = %d\n", millis, clock());
        //      int serialread = read(fd, ack, 2);
        //      usleep ( (3) * 100 );
        //      printf("read = %s\n", ack);
        //      photodiodeAck = (ack[0] == 80 ? 1:0);
        //}
        /*
        if (photodiodeAck == 1){
                printf("Got Stimulus Presentation Photodiode Ack Signal");
                writeToEyelink("Got Stimulus Presentation Photodiode Ack Signal");
                usleep ( (6) * 100 );
                write (fd, "MAAP\n", 5);
                write(fd, CODE_PHOTODIODE_ACK, sizeof(CODE_PHOTODIODE_ACK)-1);
                printf("code = %s, len = %d\n", CODE_PHOTODIODE_ACK,sizeof(CODE_PHOTODIODE_ACK)-1);
                printf("Sending Photodiode Message Receipt Ack\n");
        }*/
}

void sendEventToNeuralData(char *msg){ ////////////////////////// Should Be REVISED HT_ORSU
        usleep ( (6) * 100 ); 
        write (fd, msg, sizeof(msg)-1);
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

void sendSmallReward(){
        set_blocking (fd, 0);
		for (int i=0;i<7; i++){
        	write (fd, "RS\n", 3);
		}
        usleep ( (3) * 100 );
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