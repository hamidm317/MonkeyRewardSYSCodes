#define Dispenser_Pin A3
#define Photodiode_Pin A4
#define Mesg_Enable_Pin A5
#define Photodiode_Threshold 200   // threshold to detect a photodiode onset signal
                                   // obtained emperically by observing brightness sensitivity
#define Pins_Keep_Duration 67      // duration to keep single message on pins (us)
//#define Sync_Keep_Duration 67
//#define Sync_Repetitions 3
#define Mesg_Watchdog 2000

const int Mesg_States_Repetitions[] = {3,5,3,0};
                                   // times to send message header, body and footer 
                                   // respectively to ensure receipt
unsigned int Mesg_States_Messages[] = {32767,0,1}; // reserve 2^15-1 and 1 for header and footer respectively

enum mesgSendStateEnum {MesgSendingHeader=0, MesgSendingBody=1, MesgSendingFooter=2, MesgSendingOff=3};
                                   // define 4 states about sending a message on pins
mesgSendStateEnum mesgSendState = MesgSendingOff;
                                   // make an instance to keep either of 4 stages
                                   // initialize with base state (not sending any message now)

byte mesgPin[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
// pins in order: 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2

char inputString[10] = {'\0'};           // a string to hold incoming data
int inputStringItr = 0;
unsigned int chkitr = 0;
bool isSendingSync = false;
unsigned long syncTimer = 0;
unsigned int syncCounter = 0;
bool justReceivedString = false;   // whether a new string is received now
bool dispenseOngoing = false;      // whether a reward dispension is happening
//bool mesgOnPins = false;         // whether there's a message on pins
bool photodiodeCheck = false;      // whether should check for photodiode signal
                                   // 16-bit timers can keep 65 seconds with ms precision
//bool mesgSendingOngoing = false; // is a repetition of a message and zero is being sent
bool curBitsOnPins = false;        // whether right now a message is on pins or zero
                                   // false is for zeros and true is for message itself
                                   // is only valid is mesgSendingOngoing is true

bool curBitsOnSync = false;    
unsigned long rewardTimer = 0;      // keep track of time since reward dispense start
unsigned long mesgTimer = 0;       // keep track of time since message on pins start
unsigned int mesgCounter = 0;      // keep track of times a message went on pins
unsigned int rewardDuration = 0;   // duration of reward command
//unsigned int daleyMeasureTimer = 0;

///////////////////////// UbHTOs /////////////

unsigned int n_o_s = 2; // number of states that monkey may get reward
unsigned int a_o_r = 0; // state that monkey give reward in Rewarding message

//////////////////////////////////////////////

void setup() {
  for (int i=0; i<sizeof(mesgPin)/sizeof(mesgPin[0]); i++){
      pinMode(mesgPin[i], OUTPUT); // message pins are output
      digitalWrite(i, LOW);
    }
  pinMode(Dispenser_Pin, OUTPUT);
  pinMode(Photodiode_Pin, INPUT);
  pinMode(Mesg_Enable_Pin, OUTPUT);
  digitalWrite(Mesg_Enable_Pin, HIGH);
  Serial.begin(115200);
}

void loop() {
  // Check if new string is received
  if (justReceivedString == true){
//    Serial.println(inputString[0]);
//    Serial.println(inputString[0]);
    
    // DON'T TYPE BACK THE INPUT STRING ON DEPLOY
    // WILL CAUSE FAULT IN PHOTODIODE SIGNAL DETECTION
//    Serial.println("mesg");
//    Serial.println(inputString);
//    Serial.println("inputString[0]");
//    Serial.println((unsigned int)inputString.charAt(0));
//    Serial.println("inputString[1]");
//    Serial.println((unsigned int)inputString.charAt(1));
//    Serial.print("Received string = ");
    Serial.println(inputString);
    
//    delayMicroseconds(100);
//    if (inputString[0] == 0)
//          chkitr = 1;
    if (inputString[0] == 'R') ////// UbHTOs ///////
    { 
      //Serial.println(inputString[0]);
      //Serial.println(inputString[1]);
      //Serial.println(inputString[2]);
      n_o_s = inputString[1];
      a_o_r = inputString[2];
      //Serial.println("Number of states is ");
      //Serial.println(n_o_s);
      //for (int c=0; c < 50 ; c++);
      //Serial.println("Amount of Reward is ");
      //Serial.println(a_o_r);
      //for (int c=0; c < 50 ; c++);
      rewardDuration = 100 + (a_o_r - 48) * 200 / (n_o_s - 1 - 48);
      //Serial.println("Reward value is ");
      Serial.println(rewardDuration);
      //Serial.println("\n");

      //for (int c=0; c < 50 ; c++);
      
//    Serial.println('r');
      rewardTimer = millis();    // set reward dispense start time
//      Serial.print("rewardTimer = ");
//      Serial.println(rewardTimer);
      dispenseOngoing = true;    // set reward dispense ongoing
      digitalWrite(Dispenser_Pin, HIGH); // set reward dispense
//      Serial.println('R');
    }
    else if (inputString[0] == 'p'){  // if string is photodiode check enable
      photodiodeCheck = true;    // set photodiode check
//      Serial.println("p");
    }
    else if (inputString[0] == 'Q'){  // if string is photodiode check disable
      photodiodeCheck = false;   // set photodiode check
    }
//    else if (inputString[0] == 'S'){  // if string is photodiode check disable
//      digitalWrite(16, HIGH);
//      delayMicroseconds(1);
//      digitalWrite(Mesg_Enable_Pin, LOW);
//      isSendingSync = true;
//      curBitsOnSync = true;
//      syncTimer = micros();
//    }
//    if (inputString[0] == 'Z'){
//      digitalWrite(Mesg_Enable_Pin, HIGH);
//    }
    else if (inputString[0] == 'M'){  // if string is digital message
      if (mesgSendState == MesgSendingOff){
        mesgSendState = MesgSendingHeader;
        Mesg_States_Messages[MesgSendingBody] = 0; 
                                 // initialize message body with 0
        for (int c=0; c<3; c++){ // for each received char
          byte b = ((inputString[c+1] - 65));
                                 // shift s.t. 'A' becomes 0
                                 // concatenate byte into message
          if (b>=0 && b<=31){
//            Serial.print("Byte ");
//            Serial.print(c);
//            Serial.print(" is ");
//            Serial.println(b);
            Mesg_States_Messages[MesgSendingBody] |= (b << (5*(2-c)));
          }
          else{
//            Serial.print("Illegal value on byte ");
//            Serial.print(c);
//            Serial.print(".");
            Serial.println("INV");
            mesgSendState = MesgSendingOff;
            break;
          }
        }
//        Serial.println(Mesg_States_Messages[MesgSendingBody]);
//            ("Final Mesg is ");
//            Serial.println(Mesg_States_Messages[MesgSendingBody]);
        if (mesgSendState == MesgSendingHeader){
//          Serial.println("Writing to pins");
          for (int i=0; i<sizeof(mesgPin)/sizeof(mesgPin[0]); i++){
              digitalWrite(mesgPin[i], bitRead(Mesg_States_Messages[mesgSendState], i));
//              Serial.println(bitRead(Mesg_States_Messages[mesgSendState], i));
          }
          delayMicroseconds(1);     // wait to make everything stable
          digitalWrite(Mesg_Enable_Pin, LOW);
          curBitsOnPins = true;     // keep note that currently message is on pins
          mesgTimer = micros();     // keep track of time where they were set to mesg
        }
      }
      else{
        Serial.println("!");
        Serial.print("st ");
        Serial.println(mesgSendState);
        Serial.print("ctr ");
        Serial.println(mesgCounter);
      }
    }
    for (int i=0; i<10; i++){
        inputString[i] = '\0';
      }
    chkitr = 0;
    inputStringItr = 0;
    justReceivedString = false; // clear input string
  }

  
  // Check if timers are up
  if (dispenseOngoing && (millis() > (rewardTimer + rewardDuration))){
                                // if a reward dispense was ongoing and its time was up
    dispenseOngoing = false;    // set reward dispense ongoing false
    rewardDuration = 0;
//    Serial.println("==On Disable==");
//    Serial.print("millis() = ");
//    Serial.println(millis());
//    Serial.print("rewardTimer = ");
//    Serial.println(rewardTimer);
    digitalWrite(Dispenser_Pin, LOW); // stop reward dispense
  }
  
  if (mesgSendState != MesgSendingOff && (micros() > (mesgTimer + Pins_Keep_Duration))){

    if (mesgCounter >= Mesg_States_Repetitions[mesgSendState]){
                                // if current state has been repeated enough times
        mesgSendState = (mesgSendStateEnum) (mesgSendState + 1);
        mesgCounter = 0;
        //Serial.println(" d ");
    }
                                // go to next state
    if (mesgSendState != MesgSendingOff && curBitsOnPins == true){
                                // if a message has been on pins and its time is up now
      digitalWrite(Mesg_Enable_Pin, HIGH); // make all message pins zero
      mesgCounter++;            // count as a message being on pins
      curBitsOnPins = false;    // keep note that currently zeros are on pins
      mesgTimer = micros();     // keep track of time where they were set to zero
      ///Serial.print(" h ");
    }
    else if (mesgSendState != MesgSendingOff && curBitsOnPins == false){
      for (int i=0; i<sizeof(mesgPin)/sizeof(mesgPin[0]); i++){
            digitalWrite(mesgPin[i], bitRead(Mesg_States_Messages[mesgSendState], i));
      }
      delayMicroseconds(1);     // wait the minimum amount arduino can keep track
                                // accurately, to make everything stable
      digitalWrite(Mesg_Enable_Pin, LOW);
      curBitsOnPins = true;     // keep note that currently message is on pins
      mesgTimer = micros();     // keep track of time where they were set to mesg
      //Serial.print(" r ");
    }
  }
//
//  if (isSendingSync && (micros() > (syncTimer + Sync_Keep_Duration))){
//
//    if (curBitsOnSync == true){
//      digitalWrite(Mesg_Enable_Pin, HIGH);
//      syncCounter++; 
//      curBitsOnPins = false;
//      syncCounter = micros();
//    }
//    else if (curBitsOnSync == false){
//      digitalWrite(16, HIGH);
//      delayMicroseconds(1);
//      digitalWrite(Mesg_Enable_Pin, LOW);
//      curBitsOnSync = true;     // keep note that currently message is on pins
//      syncTimer = micros();     // keep track of time where they were set to mesg
//      //Serial.print(" r ");
//    }
//    if(syncCounter >= 3){
//      isSendingSync = false;
//      syncCounter = 0;
//    }
//  }
  if (mesgSendState != MesgSendingOff && (micros() > (mesgTimer + Mesg_Watchdog))){
    mesgSendState = MesgSendingOff;
    mesgCounter = 0;
  }

  if (photodiodeCheck){
//    daleyMeasureTimer = micros();
    int analogreadval = analogRead(Photodiode_Pin);
//    Serial.print("HD");
    if (analogreadval > Photodiode_Threshold){
//    Serial.println(analogreadval);
//          daleyMeasureTimer = micros() - daleyMeasureTimer;
//          Serial.print("Delay = ");
//          Serial.println(daleyMeasureTimer);
      Serial.write(80); // serial write 'P' (for Photodiode signal)
//      delayMicroseconds(3);     // wait the minimum amount arduino can keep track
      Serial.write(80); // serial write 'P' (for Photodiode signal)
//      delayMicroseconds(3);     // wait the minimum amount arduino can keep track
      Serial.write(80); // serial write 'P' (for Photodiode signal)
//      delayMicroseconds(3);     // wait the minimum amount arduino can keep track
      Serial.write(80); // serial write 'P' (for Photodiode signal)


//    Serial.println("Writing photodiode ack");
//    Serial.println("P");
      photodiodeCheck = false;
    }
  }
}

void serialEvent() {
//  daleyMeasureTimer = micros();
  while (Serial.available()) {
//    Serial.println(inputStringItr);
    inputString[inputStringItr] = (char)Serial.read();
//    Serial.println(inputString[inputStringItr]);
    if (inputString[inputStringItr] == '\n') { // if the string is completed
      justReceivedString = true; // set a flag indicating a string is received
    }
    inputStringItr++;
  }
}
