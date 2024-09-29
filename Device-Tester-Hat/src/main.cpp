#include <NodeControllerCore.h>

#define DEBUG 1

//Button pins
#define BUTTON_1 3
#define BUTTON_2 2
#define BUTTON_3 1
#define BUTTON_4 10 //Seems to be broken, probably a hardware issue

//LED pins
#define LED_1 9
#define LED_2 8
#define LED_3 5
#define LED_4 4

//Potentiometer pin
#define POTENTIOMETER 0

//Update intervals
#define POTENTIOMETER_UPDATE_INTERVAL 15000 //[ms]
#define BUTTON_UPDATE_INTERVAL 100 //[ms]

#define WARN_THRESHOLD 3000
#define ALERT_THRESHOLD 4000

#define WARN_ID 0x384 //900
#define ALERT_ID 0x385 //901

//Manual node ID
#define NODE_ID 0xAA

//State of the buttons last time they were read
uint64_t previousButtonStates[4] = {0, 0, 0, 0};

//Function prototypes

//Callback function for received messages from the CAN bus
void receive_message(uint8_t nodeID, uint16_t messageID, uint64_t data);

//Task to read the potentiometer value and send it to the CAN bus
void readPotentiometer(void *parameters);

//Task to read the button states and send them to the CAN bus
void readButtons(void *parameters);

//Node controller core object
NodeControllerCore core;

void receive_message(uint8_t nodeID, uint16_t messageID, uint64_t data) {
  Serial.println("Message received callback");

  //Check if the message is for this node
  if(nodeID ==  NODE_ID){
    Serial.println("Message received to self");
    //Check the message ID for the LED control messages
    switch (messageID) {
    case 0xC000:
      Serial.println("LED 1 to " + String(data));
      //PWM control of the LED based on the received data
      //TODO: Add a check for the data range
      analogWrite(LED_1, data);
      break;
    case 0xC001:
      Serial.println("LED 2 to " + String(data));
      analogWrite(LED_2, data);
      break;
    case 0xC002:
      Serial.println("LED 3 to " + String(data));
      analogWrite(LED_3, data);
      break;
    case 0xC003:
      Serial.println("LED 4 to " + String(data));
      analogWrite(LED_4, data);
      break;
    default:
      break;
    }
  }
}

void readPotentiometer(void *parameters) {
  uint8_t hasSendWarn = 0;
  uint8_t hasSendAlert = 0;
  uint8_t hasSendNOError = 0;
  while (true) {
    //Read the potentiometer value
    uint64_t data = analogRead(POTENTIOMETER);
    core.sendMessage(0xB000, &data);
    Serial.println("Potentiometer value: " + String(data));
    uint64_t errorData1 = 1;
    uint64_t errorData0 = 0;
    if(data >= WARN_THRESHOLD && data < ALERT_THRESHOLD){
      if(!hasSendWarn){
        core.sendMessage(ALERT_ID, &errorData0);
        core.sendMessage(WARN_ID, &errorData1);
        hasSendWarn = 1;
        hasSendNOError = 0;
      }
      
    }else if(data >= ALERT_THRESHOLD){
      if(!hasSendAlert){
        core.sendMessage(ALERT_ID, &errorData1);
        core.sendMessage(WARN_ID, &errorData0);
        hasSendAlert = 1;
        hasSendNOError = 0;
      }
    }
    else{
      if(!hasSendNOError){
        core.sendMessage(ALERT_ID, &errorData0);
        core.sendMessage(WARN_ID, &errorData0);
        hasSendNOError = 1;
        hasSendAlert = 0;
        hasSendWarn = 0;
      }
    }
    delay(POTENTIOMETER_UPDATE_INTERVAL);
  }
}

void readButtons(void *parameters) {
  previousButtonStates[0] = digitalRead(BUTTON_1);
  previousButtonStates[1] = digitalRead(BUTTON_2);
  previousButtonStates[2] = digitalRead(BUTTON_3);
  previousButtonStates[3] = digitalRead(BUTTON_4);

  //Send the initial button states
  core.sendMessage(0xB001, &previousButtonStates[0]);
  core.sendMessage(0xB002, &previousButtonStates[1]);
  core.sendMessage(0xB003, &previousButtonStates[2]);
  core.sendMessage(0xB004, &previousButtonStates[3]);
   
  while(1){
    delay(BUTTON_UPDATE_INTERVAL);

    //Read the button states
    uint64_t buttonStates[4] = {digitalRead(BUTTON_1), 
                                digitalRead(BUTTON_2), 
                                digitalRead(BUTTON_3), 
                                digitalRead(BUTTON_4)};
    //Send the button states to the CAM Bus if they have changed        
    if(buttonStates[0] != previousButtonStates[0]){
      core.sendMessage(0xB001,&buttonStates[0]);
      previousButtonStates[0] = buttonStates[0];
    }else if(buttonStates[1] != previousButtonStates[1]){
      core.sendMessage(0xB002,&buttonStates[1]);
      previousButtonStates[1] = buttonStates[1];
    }else if(buttonStates[2] != previousButtonStates[2]){
      core.sendMessage(0xB003,&buttonStates[2]);
      previousButtonStates[2] = buttonStates[2];
    }else if(buttonStates[3] != previousButtonStates[3]){
      core.sendMessage(0xB004,&buttonStates[3]);
      previousButtonStates[3] = buttonStates[3];
    }
  }
}

void setup() {
  
  //Initialize serial communication
  Serial.begin(115200);

  //Initialize pins
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  pinMode(POTENTIOMETER, INPUT);

  //Create the node controller core object
  core = NodeControllerCore();

  //Initialize the node controller core
  if (core.Init(receive_message, NODE_ID)) {
    #if DEBUG
      Serial.println("Driver device initialized");
    #endif
  } else {
    #if DEBUG
      Serial.println("Failed to initialize driver");
    #endif
  }

  //Tasks to read the potentiometer and buttons
  xTaskCreate(readPotentiometer, 
              "readPotentiometer", 
              2048, 
              NULL, 
              100, 
              NULL);
  
  xTaskCreate(readButtons,
              "readButtons",
              2048,
              NULL,
              100,
              NULL);

}

void loop() {
  //Empty loop as all the tasks are running in FreeRTOS

}