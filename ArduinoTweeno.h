// ArduinoTweeno
// -----------------
// An Arduino library for defining 1 dimensional arrays of
// RGB LED "pixels", animating, and compositing them.
// 
// What a terrible description. Find out more at the
// github repo: https://github.com/rDr4g0n/ArduinoTweeno

// number of sub-pixel values
// R, G, and B (maybe A in the future)
#define NUM_SUB_PX 3

// total number of pixel properties (NUM_SUB_PX plus addional properties)
// R, G, B, and X
#define NUM_PX_PROPS 4

// preallocated number of layers. If you want more layers
// you must increase this number.
#define NUM_LAYERS 4

// set FPS of animation updates and composition
#define FPS 60

// also put global includes in here so things
// are as crappy and gross as possible
#include <stdlib.h>

class Tween {
public: 
	// range: int array with 2 values: the min
	// and max values that the scale should return
	// duration: duration of the tween in frames
	void init(int duration, unsigned char rangeStart, unsigned char rangeEnd);

	// advances the playhead by one frame (or if
	// tween is reversing, reverses the playhead
	// by one frame)
	void tick();

	// returns the current output value
	unsigned char getX();

    void rewind();

    int getCurrFrame(){ return _currFrame; };

    bool isDone(){ return _isDone; };

private:
	// examines _domain and _range, and given x
	// returns the current output value
	unsigned char _scaleLinear(int x);
    
	// input domain
	int _domain [2];

	// output range
	unsigned char _range [2];

	// current frame / playhead position
	int _currFrame;

	// duration of this tween in frames
	int _duration;

    bool _isDone;
};
void Tween::init(int duration, unsigned char rangeStart, unsigned char rangeEnd){
	// TODO - ensure values are valid
	_domain[0] = 0;
	_domain[1] = duration;

	_range[0] = rangeStart;
	_range[1] = rangeEnd;

	_duration = duration;

    this->rewind();
}

void Tween::tick(){
    if(_isDone || _currFrame == _duration){
        _isDone = true;
        return;
    } else {
        _currFrame++;
    }
}

unsigned char Tween::getX(){

	// cout << "duration " << _duration << "\n";
	// if _duration is 0, then this tween
	// can just return a constant value
	if(_duration <= 0 || _isDone){
		return _range[1];
	} else {
		return _scaleLinear(_currFrame);
	}
}

void Tween::rewind(){
    _isDone = false;
    _currFrame = 0;
}

unsigned char Tween::_scaleLinear(int x){
	float scaleFactor = float(x) / _domain[1];
	unsigned char result = scaleFactor * (_range[1] - _range[0]) + _range[0];
	return result;
}
class Layer {
public:
	// fill layer initializer
	void init(unsigned char pxData[4], unsigned char maxNumPx);

	// configure a layer with a retardedly huge array
	void init(unsigned char numPx, unsigned char *pxData, unsigned char maxNumPx);

	void update();

	unsigned char numPx;
    unsigned char maxNumPx;
	unsigned char *results;
	unsigned char **pixels;

	bool fill;

    void on();
    void off();
    void pulse(unsigned char inDur, unsigned char onDur, unsigned char outDur, unsigned char offDur);
    void blink(unsigned char onDur, unsigned char offDur);
    void spin(unsigned char speed);

    bool isOff;

	Tween x;
	Tween opacity;

    // Layers chain tweens together for animation.
    // This var is the current position in the
    // tween chain (there are 4 right now)
    unsigned char tweenChainPosition;

    unsigned char pulseIn;
    unsigned char pulseOn;
    unsigned char pulseOut;
    unsigned char pulseOff;
    unsigned char spinSpeed;

	float getOpacity(){ return float(opacity.getX()) * .01; }

private: 
    void updateAnimation();
};
void Layer::init(unsigned char numPx, unsigned char *pxData, unsigned char maxNumPx){
    // TODO - ensure numPx doesn't exceed maxNumPx
	this->numPx = numPx;
    this->maxNumPx = maxNumPx;

    isOff = true;

    // by default, 100% opacity
    opacity.init(-1, 100, 100);

    // by default, no x movement
    x.init(-1, 0, 0);

    tweenChainPosition = 0;

    // allocate enough memory for pixels array
    pixels = (unsigned char**)malloc(numPx * sizeof(unsigned char*));
    // TODO - if pixels == NULL, asplode!
    
	for(int i = 0; i < numPx * NUM_PX_PROPS; i += NUM_PX_PROPS){
		int currPx = i / NUM_PX_PROPS;

        pixels[currPx] = (unsigned char*)malloc(NUM_PX_PROPS * sizeof(unsigned char));

		pixels[currPx][0] = pxData[i];
		pixels[currPx][1] = pxData[i+1];
		pixels[currPx][2] = pxData[i+2];
		pixels[currPx][3] = pxData[i+3];
	}

    // allocate memory for results array
    results = (unsigned char*)malloc(maxNumPx * NUM_SUB_PX * sizeof(unsigned char));
    // TODO - if results == NULL, asplode!

	// zero out results
	// TODO - intelligently zero out?
	for(int i = 0; i < maxNumPx * NUM_SUB_PX; i++){
		results[i] = 0;
	}
}

// fill layer initializer
void Layer::init(unsigned char *pxData, unsigned char maxNumPx){
    // TODO - ensure numPx doesn't exceed maxNumPx
	this->numPx = maxNumPx;
    this->maxNumPx = maxNumPx;

	fill = true;
    isOff = true;

    // allocate enough memory for pixels array
    pixels = (unsigned char**)malloc(numPx * sizeof(unsigned char*));
    // TODO - if pixels == NULL, asplode!

	for(int i = 0; i < numPx * NUM_PX_PROPS; i += NUM_PX_PROPS){
		int currPx = i / NUM_PX_PROPS;

        pixels[currPx] = (unsigned char*)malloc(NUM_PX_PROPS * sizeof(unsigned char));

		pixels[currPx][0] = pxData[0];
		pixels[currPx][1] = pxData[1];
		pixels[currPx][2] = pxData[2];
		pixels[currPx][3] = currPx;
	}
    
    // allocate memory for results array
    results = (unsigned char*)malloc(maxNumPx * NUM_SUB_PX * sizeof(unsigned char));
    // TODO - if results == NULL, asplode!
}

void Layer::update(){

    // if this layer is off, we done here!
    if(isOff){
        return; 
    }

    unsigned char xOffset = x.getX();

	// zero out results
	// TODO - intelligently zero out?
	for(int i = 0; i < maxNumPx * NUM_SUB_PX; i++){
		results[i] = 0;
	}

    this->updateAnimation();

	for(int i = 0; i < numPx; i++){
		int offset = ((pixels[i][3] + xOffset) % maxNumPx) * NUM_SUB_PX;
		results[offset] = pixels[i][0];
		results[offset+1] = pixels[i][1];
		results[offset+2] = pixels[i][2];

	}
}

void Layer::off(){
	for(int i = 0; i < maxNumPx * NUM_SUB_PX; i++){
		results[i] = 0;
	}

    isOff = true;

    x.rewind();
    opacity.rewind();
    tweenChainPosition = 0;
}

void Layer::on(){
    isOff = false;

    // passing a duration of -1 means the tween will never
    // finish, so it will stay on indefinitely. this also
    // means it will stay on the first range value, effectively
    // ignoreing the second range value
    opacity.init(-1, 100, 100);
}

void Layer::pulse(unsigned char inDur, unsigned char onDur, unsigned char outDur, unsigned char offDur){
    isOff = false;

    pulseIn = inDur;
    pulseOn = onDur;
    pulseOut = outDur;
    pulseOff = offDur;
    
    // fade in
    opacity.init(inDur, 0, 100);
}

void Layer::blink(unsigned char onDur, unsigned char offDur){
    isOff = false;

    pulseIn = 0;
    pulseOn = onDur;
    pulseOut = 0;
    pulseOff = offDur;
    
    // fade in
    opacity.init(0, 0, 100);
}

void Layer::spin(unsigned char speed){
    isOff = false;

    spinSpeed = speed;

    x.init(spinSpeed, 0, 16);
}

void Layer::updateAnimation(){
    opacity.tick();
    x.tick();

    // if rotation is done, repeat
    if(x.isDone()){
        x.init(spinSpeed, 0, 16);
    }

    // if this tween has completed, move to the
    // next tween in the chain
    if(opacity.isDone()){

        switch(tweenChainPosition){

            // stay on 
            case 0:
                opacity.init(pulseOn, 100, 100);
                tweenChainPosition++;
                break;

            // fade out 
            case 1:
                opacity.init(pulseOut, 100, 0);
                tweenChainPosition++;
                break;

            // stay off
            case 2:
                opacity.init(pulseOff, 0, 0);
                tweenChainPosition++;
                break;

            // reset and fade in
            case 3:
                opacity.init(pulseIn, 0, 100);
                tweenChainPosition= 0;
                break;
        }
    }
}
class Compositor{
public:
	void init(unsigned char numPx);

	// regular layer
	Layer * addLayer(unsigned char numPx, unsigned char *pxData);
	// fill layer
	Layer * addLayer(unsigned char *pxData);

	void update(long currentTime, bool force);

	unsigned char * getComp(){ return composited; };

    void setGlobalOpacity(float opacity){ globalOpacity = opacity; };

private:
	Layer layers [NUM_LAYERS];
	unsigned char currLayer;

	int tickRate;
	long lastTime;

    float globalOpacity;

	void composite();

	unsigned char *composited;

    unsigned char numPx;
    unsigned char numSubPx;
    unsigned char numElements;
};
void Compositor::init(unsigned char numPx){
    // TODO - prefix all member properties
    // with underscore
	currLayer = 0;
	tickRate = 1000 / FPS;
    globalOpacity = 1;
    this->numPx = numPx;
    numSubPx = NUM_SUB_PX;
    numElements = numPx * numSubPx;

    // allocate enough memory for composited array
    composited = (unsigned char*)malloc(numElements * sizeof(unsigned char));
    // TODO - if composited == NULL, asplode!
}

Layer * Compositor::addLayer(unsigned char numPx, unsigned char *pxData){
	// NOTE: currLayer should never exceed NUM_LAYERS
	layers[currLayer].init(numPx, pxData, this->numPx);
	currLayer++;
	return &layers[currLayer-1];
};
Layer * Compositor::addLayer(unsigned char *pxData){
	// NOTE: currLayer should never exceed NUM_LAYERS
	layers[currLayer].init(pxData, this->numPx);
	currLayer++;
	return &layers[currLayer-1];
};

void Compositor::update(long currentTime, bool force){
	long diff = currentTime - lastTime;

	// if it should tick, update
	if(diff > tickRate){
		lastTime = currentTime;
		composite();

	// if it should force update, do it
	} else if(force){
		composite();
	}
};

void Compositor::composite(){

	// zero out results
	for(int i = 0; i < numElements; i++){
		composited[i] = 0;
	}


	for(int i = 0; i < currLayer; i++){

		layers[i].update();
		float opacity = layers[i].getOpacity();

		for(int j = 0; j < numElements; j += numSubPx){

			// if this pixel is off (transparent), don't do anything
			if( layers[i].results[j] == 0 &&
				layers[i].results[j+1] == 0 &&
				layers[i].results[j+2] == 0
			){
				continue;
			}

			unsigned char r = layers[i].results[j] * opacity;
			unsigned char g = layers[i].results[j+1] * opacity;
			unsigned char b = layers[i].results[j+2] * opacity;

			// TODO - better blending mode calculations go here
			composited[j] = r + (composited[j] * (1-opacity));
			composited[j+1] = g + (composited[j+1] * (1-opacity));
			composited[j+2] = b + (composited[j+2] * (1-opacity));
		}

	}

    // apply global opacity multiplier
    for(int k = 0; k < numElements; k++){
        composited[k] *= globalOpacity;
    }

}
