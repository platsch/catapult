#include <Adafruit_NeoPixel.h>

#define STRIPE_PIN 2	 // input pin Neopixel is attached to
#define SHORT_STRIPE1_PIN 3
#define SHORT_STRIPE2_PIN 4
#define RING_PIN 5
#define BUTTON1_PIN 9
#define BUTTON2_PIN 10
#define POTI1_PIN A1
#define POTI2_PIN A0
#define STRIPPIXELS 69
#define DEMO_DELAY 30 // time in s until demo mode gets activated
#define DEMO_WINDOW 1000 // random variations of loading time in demo mode. 1000 means +-500ms variation.

// two led strips back to back for 360° "view". Comment out to have only one stripe.
#define DOUBLE_STRIPE

#ifdef DOUBLE_STRIPE
#define NUMPIXELS      300 // number of neopixels in main stripe
#define MAX_H 5.0 // maximum throwable height in meters
#else
#define NUMPIXELS      300 // number of neopixels in main stripe
#define MAX_H 10.0 // maximum throwable height in meters
#endif

enum states {
  IDLE, LOAD, THROW, HIT
};

Adafruit_NeoPixel stripe = Adafruit_NeoPixel(NUMPIXELS, STRIPE_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_l = Adafruit_NeoPixel(STRIPPIXELS, SHORT_STRIPE1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_r = Adafruit_NeoPixel(STRIPPIXELS, SHORT_STRIPE2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring = Adafruit_NeoPixel(16, RING_PIN, NEO_GRB + NEO_KHZ800);

// global settings and states
double brightness = 100;
double gravity = 9;
double max_v = 1;
int target = 0;
int bullet_size = 10;
unsigned int max_score = 10;
bool demo_mode = false;

// player related variables
states state[2] = {IDLE, IDLE};
int score[2] = {0, 0};
uint8_t color_p[2][3] = {{255, 40, 41}, {50, 60, 180}};//{106, 95, 219}};

// load
unsigned long load_t0[2] = {0, 0};

// throw
double throw_v0[2] = {-1, -1};
unsigned long throw_t0[2] = {0, 0};
int throw_last_h[2] = {0, 0};
bool throw_missed[2] = {false, false};

// hit
int hit_position[2] = {0, 0};
unsigned long hit_t0[2] = {0, 0};

// demo / idle
unsigned long demo_last_interaction = 0;
unsigned long demo_t0[2] = {0, 0};
unsigned long demo_load[2] = {0, 0};

void setup() {
  stripe.begin(); // Initializes the NeoPixel library.
  strip_l.begin();
  strip_r.begin();
  ring.begin();
  pinMode(BUTTON1_PIN, INPUT_PULLUP); // initialize input pin
  pinMode(BUTTON2_PIN, INPUT_PULLUP); // initialize input pin
#ifdef DOUBLE_STRIPE
  target = random(20, NUMPIXELS/2);
#else
  target = random(20, NUMPIXELS);
#endif
  //randomSeed(analogRead(0)); // initialize random seed
  Serial.begin(9600);

  stripe.clear();
  stripe.show();
  strip_l.clear();
  strip_l.show();
  strip_r.clear();
  strip_r.show();
}

bool init_sequence = true;

void loop() {
  // read brightness poti
  int poti = analogRead(POTI1_PIN);
  if(abs(brightness - poti/1023.0) > 0.01) {
    brightness = poti/1023.0;
  }

  // read gravity poti
  poti = analogRead(POTI2_PIN);
  double tmp_gravity = max(1.0, poti/1023.0 * 16.0);
  if(abs(tmp_gravity - gravity) > 0.05) {
    Serial.print("update gravity to: ");
    Serial.println(gravity);
    gravity = tmp_gravity;
    // compute new maximum velocity
    max_v = sqrt(2 * MAX_H * gravity);
  }
  updateRing();

  // this is only executed once, but after reading poti values, therefore not in setup()
  if(init_sequence) {
    initSequence();
    init_sequence = false;
  }

  //  clear current stripe status
  stripe.clear();
  
  // visualize target pixel
  stripe.setPixelColor(target, br_color(0, 255, 0));
#ifdef DOUBLE_STRIPE
  stripe.setPixelColor(NUMPIXELS-target-1, br_color(0, 255, 0));
#endif

  // visualize score
  updateScore();

  // update state machine
  for(int player=0; player <= 1; player++) {
    bool input = (player == 0) ? !digitalRead(BUTTON1_PIN) : !digitalRead(BUTTON2_PIN); // read push buttons

    // interrupt demo mode and reset timer if an interaction happens
    if(input) {
      if(demo_mode) {
        score[0] = 0;
        score[1] = 0;
      }
      demo_last_interaction = millis();
      demo_mode = false;
      demo_t0[0] = 0;
      demo_t0[1] = 0;
    }
    // demo mode?
    if(millis() - demo_last_interaction > DEMO_DELAY * 1000) {
      if(!demo_mode) {
        score[0] = 0;
        score[1] = 0;
      }
      demo_mode = true;
    }
    if(demo_mode) {
      switch(state[player]) {
      case states::LOAD: // don't understand why, but this needs to be earlier than states::IDLE, otherwise it will be skipped. Weird.
        input = true;
        if((millis() - demo_t0[player]) > demo_load[player]) {
          input = false; // <- trigger throw
        }
        break;
      case states::IDLE:
        // initiate demo throw
        demo_t0[player] = millis();
        // figure out a loading time (+- some  random) to hit the target
        // h = 1/2 * a * t^2 -> t = sqrt((2*h)/a) (time of flight, unit: s)
        double t = sqrt((2.0*target/30.0)/gravity);
        double v0 = t*gravity;
        demo_load[player] = v0/max_v * 2500; // max_v would be reached by holding the button for 2.5s. THIS IS PROBABLY NOT HOW IT ACTS IN REALITY, seems that max loading time is > 2.5s for some reason.
        demo_load[player] += random(0, DEMO_WINDOW) - DEMO_WINDOW*0.5; //some random variation demo load time
        input = true; // trigger state switch
        break;
      case states::THROW:
        break;
      case HIT:
        break;
      }
    }

    // local variables for switch/case
    uint8_t energy = 0;
    bool hit;

    switch(state[player]) {
      case states::IDLE:
        if(input) {
          state[player] = states::LOAD;
        }
        break;
      case states::LOAD:
        energy = load_pixel(player);
        if(!input) {
          state[player] = states::THROW;
          load_t0[player] = 0;
          throw_v0[player] = max_v/255.0 * energy;
        }
        break;
      case states::THROW:
        if(throw_pixel(player, hit)) {
          // reset throw state
          throw_t0[player] = 0; // reset to invalid
          throw_missed[player] = false;

          if(hit) {
            state[player] = states::HIT;
            score[player]++;

            // generate new target
            hit_position[player] = target;
#ifdef DOUBLE_STRIPE
            target = random(20, NUMPIXELS/2);
#else
            target = random(20, NUMPIXELS);
#endif
          } else {
            state[player] = states::IDLE;
          }
        }
        break;
      case HIT:
        if(visualizeHit(player)) {
          // won this round??
          if(score[player] >= max_score) {
            visualize_win(player);
            for(int i=0; i<=1; i++) {
              score[i] = 0;
              throw_t0[i] = 0;
              throw_missed[i] = false;
              state[i] = states::IDLE;
            }
          }
          state[player] = states::IDLE;
        }
        break;
    }
  }

  stripe.show();
  //delay(100);
}

// returns current energy as value between 0-255. 255 leads to max_v.
uint8_t load_pixel(int player) {
  uint8_t energy = 0;
  if(load_t0[player] == 0) load_t0[player] = millis();
  energy = min((millis() - load_t0[player]) * 0.1, 255);  // max loading time 2.5sec -> / 2500 * 255

  // visualize loading
  for(int i = 0; i <= energy/25; i++) {
    stripe.setPixelColor(i, stripe.getPixelColor(i) | br_color(color_p[player][0], color_p[player][1], color_p[player][2]));
#ifdef DOUBLE_STRIPE
    stripe.setPixelColor(NUMPIXELS-i-1, stripe.getPixelColor(i) | br_color(color_p[player][0], color_p[player][1], color_p[player][2]));
#endif
  }

  return energy;
}

// returns true if hit or returned to floor
bool throw_pixel(int player, bool& hit) {
  hit  = false; // default return value
  if(throw_t0[player] == 0) throw_t0[player] = millis();
  double t = (millis() - throw_t0[player]) / 1000.0;

  int h = ((throw_v0[player] * t) - (0.5 * gravity * t*t)) * 30.0; // 30 leds per meter

  for(int i = 0; i < bullet_size; i++) {
    int k = min(h+i, NUMPIXELS/2-1);
    uint32_t color = stripe.getPixelColor(k) | br_color(color_p[player][0], color_p[player][1], color_p[player][2]);
    stripe.setPixelColor(k, color);
#ifdef DOUBLE_STRIPE
    color = stripe.getPixelColor(NUMPIXELS-k-1) | br_color(color_p[player][0], color_p[player][1], color_p[player][2]);
    stripe.setPixelColor(NUMPIXELS-k-1, color);
#endif
  }

  // target hit?
  if(throw_last_h[player] > h && !throw_missed[player]) {
    // highest point
    if((throw_last_h[player] <= target) && (target <= (throw_last_h[player] + bullet_size))) {
      Serial.print("player ");
      Serial.print(player);
      Serial.println(" HIT!!");
      hit = true;
      throw_last_h[player] = 0;
      return true;
    }else{
      throw_missed[player] = true;
      Serial.print("player ");
      Serial.print(player);
      Serial.println(" missed...");
    }
  }
  
  if(h < 0) {
    throw_last_h[player] = 0;
    return true;
  }

  throw_last_h[player] =  h;
  return false;
}

bool visualizeHit(int player) {
  if(hit_t0[player] == 0) hit_t0[player] = millis();
  unsigned long t = millis() - hit_t0[player];

  if(t % 100 >= 50) {
    int viz_start = max(hit_position[player] - bullet_size/2, 0);
    for(int i = 0; i < bullet_size; i++) {
      int k = min(viz_start+i, NUMPIXELS/2-1);
      stripe.setPixelColor(k, br_color(0, 0, 255));
#ifdef DOUBLE_STRIPE
      stripe.setPixelColor(NUMPIXELS-k-1, br_color(0, 0, 255));
#endif
    }
  }

  if(t > 500) {
    hit_t0[player] = 0;
    return true;
  } else {
    return false;
  }
}

void updateScore() {
  int blocksize = 5;
  strip_l.clear();
  strip_r.clear();
  for(int i=0; i<score[0]*blocksize; i++) {
    if(i % blocksize == blocksize-1) {
      // skip pixel as seperator
    } else {
      strip_l.setPixelColor(i, br_color(color_p[0][0], color_p[0][1], color_p[0][2]));
    }
  }
  for(int i=0; i<score[1]*blocksize; i++) {
    if(i % blocksize == blocksize-1) {
      // skip pixel as seperator
    } else {
      strip_r.setPixelColor(i, br_color(color_p[1][0], color_p[1][1], color_p[1][2]));
    }
  }
  strip_l.show();
  strip_r.show();
}

void visualize_win(int player) {
  for(int k=0; k<=3; k++) {
    strip_l.clear();
    strip_r.clear();
    strip_l.show();
    strip_r.show();
    for(int i=0; i < STRIPPIXELS; i++) {
      if(player == 0) {
        //left
        strip_l.setPixelColor(i, br_color(color_p[0][0], color_p[0][1], color_p[0][2]));
        strip_l.show();
      } else {
        //right
        strip_r.setPixelColor(i, br_color(color_p[1][0], color_p[1][1], color_p[1][2]));
        strip_r.show();
      }
      delay(10);
    }
  }
}

void updateRing() {
  ring.clear();
  for(int i=0; i < gravity; i++) {
    ring.setPixelColor(15-i, br_color(50, 10, 0));
  }
  ring.show();
}

void initSequence() {
  // init sequence
  for(int i=0; i < STRIPPIXELS; i++) {
    strip_l.clear();
    strip_l.setPixelColor(i, br_color(0, 0, 255));
    strip_l.show();
    strip_r.clear();
    strip_r.setPixelColor(i, br_color(0, 0, 255));
    strip_r.show();
    delay(10);
  }
  for(int i=STRIPPIXELS; i >= 0; i--) {
    strip_l.clear();
    strip_l.setPixelColor(i, br_color(0, 0, 255));
    strip_l.show();
    strip_r.clear();
    strip_r.setPixelColor(i, br_color(0, 0, 255));
    strip_r.show();
    delay(10);
  }
  strip_l.clear();
  strip_l.show();
  strip_r.clear();
  strip_r.show();
}

// returns brightness adjustes pixel color value
uint32_t br_color(uint8_t r, uint8_t g, uint8_t b) {
  return Adafruit_NeoPixel::Color(r*brightness, g*brightness, b*brightness);
}

