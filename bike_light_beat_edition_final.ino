#include <ArduinoTweeno.h>

// test.ino
//#include "ArduinoTweeno.h"
#include "Adafruit_NeoPixel.h"


//signal pins
#define PIN 2
#define PIN_L 5
#define PIN_R 13

#define NUM_TAIL_PX 16

// "unsigned char" is a lot to type, so just call it byte
typedef unsigned char byte;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_TAIL_PX, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_l = Adafruit_NeoPixel(1, PIN_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_r = Adafruit_NeoPixel(1, PIN_R, NEO_GRB + NEO_KHZ800);

Compositor comp;
Layer * rightSig;
Layer * leftSig;
Layer * brake;
Layer * running;

Compositor comp2;
Layer * frontRightRunning;
Layer * frontRightSig;

Compositor comp3;
Layer * frontLeftRunning;
Layer * frontLeftSig;

int lSwitchPin = 11; //input left switch
int rSwitchPin = 12; //input right switch
int brakePinL = 7; //input from brake
int brakePinR = 10; //input from brake

// state of various switches
// NOTE: assumes all switches are off
// Set all sensors to high to force initial check
// of sensor state
byte state[4] = {HIGH, HIGH, LOW, LOW};
// indices of state flags
byte BRAKE_RIGHT = 0;
byte BRAKE_LEFT = 1;
byte SIGNAL_RIGHT = 2;
byte SIGNAL_LEFT = 3;


void setup(){

    pinMode(lSwitchPin, INPUT);   //switch for pin input
    pinMode(rSwitchPin, INPUT);   //switch for pin input
    pinMode(brakePinL, INPUT);  
    pinMode(brakePinR, INPUT);   
    
    //pull up resistor init
    //digitalWrite(lSwitchPin, HIGH);
    //digitalWrite(rSwitchPin, HIGH);

    // start up the rear light compositor
    // with 16 pixels
    comp.init(16);

    // set opacity calculation applied 
    // to all layers. use this method
    // to dim the tail light to save battery
    comp.setGlobalOpacity(0.5);

    // running light that is always on
	byte runningPxData[16 * NUM_PX_PROPS] = {
		0,15,25,0, // r, g, b, x
		0,15,25,1, // r, g, b, x
		0,36,50,2, // r, g, b, x
		0,75,100,3, // r, g, b, x
		0,75,100,4, // r, g, b, x
		0,36,50,5, // r, g, b, x
		0,15,25,6, // r, g, b, x
		0,15,25,7, // r, g, b, x
		25,8,3,8, // r, g, b, x
		25,8,3,9, // r, g, b, x
		50,19,3,10, // r, g, b, x
		100,38,3,11, // r, g, b, x
		100,38,3,12, // r, g, b, x
		50,19,3,13, // r, g, b, x
		25,8,3,14, // r, g, b, x
		25,8,3,15, // r, g, b, x
	};
	running = comp.addLayer(16, runningPxData);


    // brake light
    // NOTE: passing in a single pixel and not passing
    // in a pixel count creates a fill layer which 
    // duplicates the one pixel over all available pixels
	byte brakePxData[1 * NUM_PX_PROPS] = {
		100,0,0,0, // r, g, b, x
	};
	brake = comp.addLayer(brakePxData);


    // right turn signal
	byte rightSigPxData[6 * NUM_PX_PROPS] = {
		50,50,0,9, // r, g, b, x
		100,100,0,10, // r, g, b, x
		100,100,0,11, // r, g, b, x
		100,100,0,12, // r, g, b, x
		100,100,0,13, // r, g, b, x
		50,50,0,14 // r, g, b, x
	};
	rightSig = comp.addLayer(6, rightSigPxData);
    

    // left turn signal
	byte leftSigPxData[6 * NUM_PX_PROPS] = {
		50,50,0,1, // r, g, b, x
		100,100,0,2, // r, g, b, x
		100,100,0,3, // r, g, b, x
		100,100,0,4, // r, g, b, x
		100,100,0,5, // r, g, b, x
		50,50,0,6 // r, g, b, x
	};
	leftSig = comp.addLayer(6, leftSigPxData);

	strip.begin();
	strip.show();
      
    running->spin(120); 


    // setup front right compositor
    comp2.init(1);

    // front right turn signal
    byte frontRightRunningPxData[1 * NUM_PX_PROPS] = {
        25,25,25,0,   
    };
    frontRightRunning = comp2.addLayer(1, frontRightRunningPxData);

	byte frontRightSigPxData[1 * NUM_PX_PROPS] = {
		100,100,0,0, // r, g, b, x
	};
	frontRightSig = comp2.addLayer(1, frontRightSigPxData);

	strip_r.begin();
	strip_r.show();

    frontRightRunning->on();


    // setup front left compositor
    comp3.init(1);

    // front right turn signal
    byte frontLeftRunningPxData[1 * NUM_PX_PROPS] = {
        25,25,25,0,   
    };
    frontLeftRunning = comp3.addLayer(1, frontLeftRunningPxData);

	byte frontLeftSigPxData[1 * NUM_PX_PROPS] = {
		100,100,0,0, // r, g, b, x
	};
	frontLeftSig = comp3.addLayer(1, frontLeftSigPxData);

	strip_l.begin();
	strip_l.show();

    frontLeftRunning->on();
}

void loop(){
 
  

    // UPDATE BRAKE RIGHT
    if(updateSensorState(brakePinR, BRAKE_RIGHT) || (updateSensorState(brakePinL, BRAKE_LEFT) )){
        // if brake switch is open, turn
        // the brake on
        if((state[BRAKE_RIGHT] == LOW)||(state[BRAKE_LEFT] == LOW)){
            brake->on();    
        } else {
            brake->off();
        }
    }

   /* // UPDATE BRAKE LEFT
    if(updateSensorState(brakePinL, BRAKE_LEFT)){
        // if brake switch is open, turn
        // the brake on
        if(state[BRAKE_LEFT] == LOW){
            brake->on();    
        } else {
            brake->off();
        }
    }*/

    // UPDATE SIGNAL RIGHT
    if(updateSensorState(rSwitchPin, SIGNAL_RIGHT)){
        if(state[SIGNAL_RIGHT] == HIGH){
            rightSig->pulse(8, 8, 8, 8);
            frontRightSig->pulse(8, 8, 8, 8);
        } else {
            rightSig->off();
            frontRightSig->off();
        }
    }

    // UPDATE SIGNAL LEFT
    if(updateSensorState(lSwitchPin, SIGNAL_LEFT)){
        if(state[SIGNAL_LEFT] == HIGH){
            leftSig->pulse(8, 8, 8, 8);
            frontLeftSig->pulse(8, 8, 8, 8);
        } else {
            leftSig->off();
            frontLeftSig->off();
        }
    }    
    
 
    


	comp.update(millis(), false);
	byte * composited;
	composited = comp.getComp();
    writePixelsToStrip(NUM_TAIL_PX, composited, &strip);

    comp2.update(millis(), false);
    byte * composited2;
    composited2 = comp2.getComp();
    writePixelsToStrip(1, composited2, &strip_r);

    comp3.update(millis(), false);
    byte * composited3;
    composited3 = comp3.getComp();
    writePixelsToStrip(1, composited3, &strip_l);
}

// TODO - infer index from pin (some sorta dictionary)
// TODO - completely encapsulate state array
// updates the state at index index from the switch
// at pin pin. Returns true if the state changed, or
// false if it was unchanged
bool updateSensorState(int pin, byte index){
    
    byte sensorState = digitalRead(pin);
    
    // if the state has changed
    if (sensorState != state[index]){
        if(sensorState == LOW){
            state[index] = LOW;
        } else {
            state[index] = HIGH;
        }
        return true;
    }
    return false;
}

// takes number of pixels, pixels array, and a strip to write to
void writePixelsToStrip(int numPx, byte *pixels, Adafruit_NeoPixel *strip){
	for(int i = 0; i < numPx * NUM_SUB_PX; i += NUM_SUB_PX){
		strip->setPixelColor(i / NUM_SUB_PX,  strip->Color(pixels[i], pixels[i+1], pixels[i+2]));
	}
    strip->show();
}