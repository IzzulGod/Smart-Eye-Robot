#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ANIMATION STATES
enum AnimationMode {
  MODE_IDLE = 0,
  MODE_HAPPY = 1,
  MODE_SURPRISED = 2,
  MODE_SLEEPY = 3,
  MODE_ANGRY = 4,
  MODE_CONFUSED = 5,
  MODE_FOCUSED = 6,
  MODE_WINK = 7
};

// EYE PARAMETERS
struct EyeState {
  int x, y;              // center position
  int width, height;     // current size
  int target_width, target_height;  // target size for smooth animation
  float scale_factor;    // for breathing effect
};

// Reference dimensions
const int REF_EYE_HEIGHT = 40;
const int REF_EYE_WIDTH = 40;
const int REF_SPACE_BETWEEN = 10;
const int REF_CORNER_RADIUS = 10;

// Eye states
EyeState left_eye, right_eye;

// Animation control
AnimationMode current_mode = MODE_IDLE;
unsigned long last_blink = 0;
unsigned long last_breath = 0;
unsigned long last_idle_change = 0;
unsigned long mode_start_time = 0;

// Timing constants
const unsigned long BLINK_INTERVAL_MIN = 2000;  // 2 seconds
const unsigned long BLINK_INTERVAL_MAX = 5000;  // 5 seconds
const unsigned long BREATH_CYCLE = 3000;        // 3 seconds
const unsigned long IDLE_CHANGE_INTERVAL = 8000; // 8 seconds

// Animation parameters
float breath_phase = 0;
int idle_pattern = 0;
boolean is_blinking = false;
int blink_progress = 0;

// CORE FUNCTIONS 

void initializeEyes() {
  // Set default eye positions (centered)
  left_eye.x = SCREEN_WIDTH/2 - REF_EYE_WIDTH/2 - REF_SPACE_BETWEEN/2;
  left_eye.y = SCREEN_HEIGHT/2;
  left_eye.width = left_eye.target_width = REF_EYE_WIDTH;
  left_eye.height = left_eye.target_height = REF_EYE_HEIGHT;
  left_eye.scale_factor = 1.0;
  
  right_eye.x = SCREEN_WIDTH/2 + REF_EYE_WIDTH/2 + REF_SPACE_BETWEEN/2;
  right_eye.y = SCREEN_HEIGHT/2;
  right_eye.width = right_eye.target_width = REF_EYE_WIDTH;
  right_eye.height = right_eye.target_height = REF_EYE_HEIGHT;
  right_eye.scale_factor = 1.0;
}

void drawEye(EyeState &eye, bool invert = false) {
  int x = eye.x - eye.width/2;
  int y = eye.y - eye.height/2;
  
  if (invert) {
    display.fillRoundRect(x, y, eye.width, eye.height, REF_CORNER_RADIUS, SSD1306_BLACK);
    display.drawRoundRect(x-1, y-1, eye.width+2, eye.height+2, REF_CORNER_RADIUS, SSD1306_WHITE);
  } else {
    display.fillRoundRect(x, y, eye.width, eye.height, REF_CORNER_RADIUS, SSD1306_WHITE);
  }
}

void updateDisplay() {
  display.clearDisplay();
  drawEye(left_eye);
  drawEye(right_eye);
  display.display();
}

void smoothTransition(EyeState &eye, float speed = 0.3) {
  eye.width += (eye.target_width - eye.width) * speed;
  eye.height += (eye.target_height - eye.height) * speed;
}

// IDLE ANIMATIONS 

void updateIdleBreathing() {
  unsigned long now = millis();
  
  if (now - last_breath > 50) { // Update every 50ms for smooth animation
    breath_phase += 0.1;
    if (breath_phase > 2 * PI) breath_phase = 0;
    
    float breath_scale = 1.0 + 0.05 * sin(breath_phase); // 5% size variation
    
    left_eye.target_width = REF_EYE_WIDTH * breath_scale;
    left_eye.target_height = REF_EYE_HEIGHT * breath_scale;
    right_eye.target_width = REF_EYE_WIDTH * breath_scale;
    right_eye.target_height = REF_EYE_HEIGHT * breath_scale;
    
    last_breath = now;
  }
}

void updateAlternatingPulse() {
  unsigned long now = millis();
  
  if (now - last_breath > 80) {
    breath_phase += 0.15;
    if (breath_phase > 4 * PI) breath_phase = 0;
    
    float left_sin = sin(breath_phase);
    float right_sin = sin(breath_phase + PI);
    float left_scale = 1.0 + 0.1 * (left_sin > 0 ? left_sin : 0);
    float right_scale = 1.0 + 0.1 * (right_sin > 0 ? right_sin : 0);
    
    left_eye.target_width = REF_EYE_WIDTH * left_scale;
    left_eye.target_height = REF_EYE_HEIGHT * left_scale;
    right_eye.target_width = REF_EYE_WIDTH * right_scale;
    right_eye.target_height = REF_EYE_HEIGHT * right_scale;
    
    last_breath = now;
  }
}

void updateRandomBlink() {
  unsigned long now = millis();
  
  if (is_blinking) {
    blink_progress++;
    if (blink_progress < 3) {
      left_eye.target_height = 3;
      right_eye.target_height = 3;
    } else if (blink_progress < 6) {
      left_eye.target_height = REF_EYE_HEIGHT;
      right_eye.target_height = REF_EYE_HEIGHT;
    } else {
      is_blinking = false;
      blink_progress = 0;
      last_blink = now + random(BLINK_INTERVAL_MIN, BLINK_INTERVAL_MAX);
    }
  } else if (now > last_blink) {
    is_blinking = true;
    blink_progress = 0;
  }
}

void updateIdlePattern() {
  unsigned long now = millis();
  
  if (now - last_idle_change > IDLE_CHANGE_INTERVAL) {
    idle_pattern = (idle_pattern + 1) % 2; // Alternate between breathing and pulsing
    last_idle_change = now;
  }
  
  if (idle_pattern == 0) {
    updateIdleBreathing();
  } else {
    updateAlternatingPulse();
  }
  
  updateRandomBlink();
}

// EXPRESSION ANIMATIONS 

void animateHappy() {
  // Wider eyes with upward curve effect
  left_eye.target_width = REF_EYE_WIDTH + 8;
  left_eye.target_height = REF_EYE_HEIGHT - 5;
  right_eye.target_width = REF_EYE_WIDTH + 8;
  right_eye.target_height = REF_EYE_HEIGHT - 5;
  
  // Add smile effect by drawing curves
  display.clearDisplay();
  drawEye(left_eye);
  drawEye(right_eye);
  
  // Draw smile curves under eyes
  int curve_y = left_eye.y + left_eye.height/2 + 5;
  for (int i = 0; i < left_eye.width; i++) {
    int curve_height = 3 * sin(PI * i / (float)left_eye.width);
    display.drawPixel(left_eye.x - left_eye.width/2 + i, curve_y + curve_height, SSD1306_BLACK);
  }
  for (int i = 0; i < right_eye.width; i++) {
    int curve_height = 3 * sin(PI * i / (float)right_eye.width);
    display.drawPixel(right_eye.x - right_eye.width/2 + i, curve_y + curve_height, SSD1306_BLACK);
  }
  
  display.display();
}

void animateSurprised() {
  // Very wide eyes
  left_eye.target_width = REF_EYE_WIDTH + 15;
  left_eye.target_height = REF_EYE_HEIGHT + 15;
  right_eye.target_width = REF_EYE_WIDTH + 15;
  right_eye.target_height = REF_EYE_HEIGHT + 15;
}

void animateSleepy() {
  // Half-closed eyes
  left_eye.target_width = REF_EYE_WIDTH;
  left_eye.target_height = REF_EYE_HEIGHT / 3;
  right_eye.target_width = REF_EYE_WIDTH;
  right_eye.target_height = REF_EYE_HEIGHT / 3;
}

void animateAngry() {
  // Narrowed eyes with angry brows
  left_eye.target_width = REF_EYE_WIDTH - 5;
  left_eye.target_height = REF_EYE_HEIGHT - 10;
  right_eye.target_width = REF_EYE_WIDTH - 5;
  right_eye.target_height = REF_EYE_HEIGHT - 10;
  
  display.clearDisplay();
  drawEye(left_eye);
  drawEye(right_eye);
  
  // Draw angry brows
  int brow_y = left_eye.y - left_eye.height/2 - 8;
  display.drawLine(left_eye.x - left_eye.width/2, brow_y + 3, 
                   left_eye.x + left_eye.width/2, brow_y, SSD1306_WHITE);
  display.drawLine(right_eye.x - right_eye.width/2, brow_y, 
                   right_eye.x + right_eye.width/2, brow_y + 3, SSD1306_WHITE);
  
  display.display();
}

void animateConfused() {
  // One eye slightly larger, slight head tilt effect
  left_eye.target_width = REF_EYE_WIDTH + 5;
  left_eye.target_height = REF_EYE_HEIGHT + 5;
  right_eye.target_width = REF_EYE_WIDTH - 3;
  right_eye.target_height = REF_EYE_HEIGHT - 3;
  
  display.clearDisplay();
  drawEye(left_eye);
  drawEye(right_eye);
  
  // Draw question mark effect
  display.drawCircle(SCREEN_WIDTH - 15, 15, 3, SSD1306_WHITE);
  display.drawPixel(SCREEN_WIDTH - 15, 22, SSD1306_WHITE);
  
  display.display();
}

void animateFocused() {
  // Slightly smaller, concentrated eyes
  left_eye.target_width = REF_EYE_WIDTH - 8;
  left_eye.target_height = REF_EYE_HEIGHT - 5;
  right_eye.target_width = REF_EYE_WIDTH - 8;
  right_eye.target_height = REF_EYE_HEIGHT - 5;
}

void animateWink() {
  // Close left eye, keep right eye normal
  left_eye.target_width = REF_EYE_WIDTH;
  left_eye.target_height = 2;
  right_eye.target_width = REF_EYE_WIDTH;
  right_eye.target_height = REF_EYE_HEIGHT;
}

// ANIMATION CONTROLLER 

void updateAnimation() {
  unsigned long now = millis();
  
  switch (current_mode) {
    case MODE_IDLE:
      updateIdlePattern();
      break;
      
    case MODE_HAPPY:
      animateHappy();
      if (now - mode_start_time > 3000) current_mode = MODE_IDLE;
      return; // Skip smooth transition for special drawing
      
    case MODE_SURPRISED:
      animateSurprised();
      if (now - mode_start_time > 2000) current_mode = MODE_IDLE;
      break;
      
    case MODE_SLEEPY:
      animateSleepy();
      if (now - mode_start_time > 5000) current_mode = MODE_IDLE;
      break;
      
    case MODE_ANGRY:
      animateAngry();
      if (now - mode_start_time > 3000) current_mode = MODE_IDLE;
      return; // Skip smooth transition for special drawing
      
    case MODE_CONFUSED:
      animateConfused();
      if (now - mode_start_time > 4000) current_mode = MODE_IDLE;
      return; // Skip smooth transition for special drawing
      
    case MODE_FOCUSED:
      animateFocused();
      if (now - mode_start_time > 3000) current_mode = MODE_IDLE;
      break;
      
    case MODE_WINK:
      animateWink();
      if (now - mode_start_time > 1000) current_mode = MODE_IDLE;
      break;
  }
  
  // Apply smooth transitions
  smoothTransition(left_eye);
  smoothTransition(right_eye);
  updateDisplay();
}

void setAnimationMode(AnimationMode mode) {
  current_mode = mode;
  mode_start_time = millis();
  
  // Reset to default position when switching modes
  if (mode == MODE_IDLE) {
    initializeEyes();
  }
}

// SERIAL COMMUNICATION 

void processSerialCommand() {
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command.startsWith("EXPR_")) {
      String expr = command.substring(5);
      
      if (expr == "HAPPY") setAnimationMode(MODE_HAPPY);
      else if (expr == "SURPRISED") setAnimationMode(MODE_SURPRISED);
      else if (expr == "SLEEPY") setAnimationMode(MODE_SLEEPY);
      else if (expr == "ANGRY") setAnimationMode(MODE_ANGRY);
      else if (expr == "CONFUSED") setAnimationMode(MODE_CONFUSED);
      else if (expr == "FOCUSED") setAnimationMode(MODE_FOCUSED);
      else if (expr == "WINK") setAnimationMode(MODE_WINK);
      else if (expr == "IDLE") setAnimationMode(MODE_IDLE);
      
      Serial.println("Expression set to: " + expr);
    }
    else if (command == "STATUS") {
      Serial.print("Current mode: ");
      Serial.println(current_mode);
    }
  }
}

// MAIN FUNCTIONS

void setup() {
  Serial.begin(115200);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("LLM Eye System"));
  display.println(F("Initializing..."));
  display.display();
  delay(2000);
  
  initializeEyes();
  setAnimationMode(MODE_IDLE);
  
  Serial.println("Eye animation system ready!");
  Serial.println("Commands:");
  Serial.println("EXPR_HAPPY, EXPR_SURPRISED, EXPR_SLEEPY");
  Serial.println("EXPR_ANGRY, EXPR_CONFUSED, EXPR_FOCUSED, EXPR_WINK");
  Serial.println("EXPR_IDLE, STATUS");
}

void loop() {
  processSerialCommand();
  updateAnimation();
  delay(20); // 50 FPS for smooth animation
}