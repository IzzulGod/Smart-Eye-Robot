//intellar.ca 
//2025 - Enhanced Smart Eye Robot
//ESP32 with OLED Display, Microphone, Speaker, and LLM Integration

#include <SPI.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <SPIFFS.h>

// Pin definitions
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
#define MIC_PIN 34
#define SPEAKER_PIN 26
#define BUTTON_PIN 0

// Display setup
U8G2_SH1107_PIMORONI_128X128_F_HW_I2C u8g2(U8G2_R0);

// Constants
const int COLOR_WHITE = 1;
const int COLOR_BLACK = 0;
const int SCREEN_WIDTH = 128; 
const int SCREEN_HEIGHT = 128;

// Eye parameters
struct EyeState {
  int left_eye_x, left_eye_y, left_eye_width, left_eye_height;
  int right_eye_x, right_eye_y, right_eye_width, right_eye_height;
  int corner_radius;
  bool is_blinking;
  unsigned long last_blink;
  unsigned long last_idle_move;
  int idle_direction_x, idle_direction_y;
};

EyeState eyes;

// Reference values
const int REF_LEFT_EYE = 32;
const int REF_EYE_HEIGHT = 40;
const int REF_EYE_WIDTH = 40;
const int REF_SPACE_BETWEEN_EYE = 10;
const int REF_CORNER_RADIUS = 10;

// Communication and AI
String conversation_context = "";
bool is_listening = false;
bool is_speaking = false;
unsigned long last_interaction = 0;
const unsigned long IDLE_TIMEOUT = 10000; // 10 seconds

// Animation states
enum AnimationState {
  IDLE,
  LISTENING,
  THINKING,
  SPEAKING,
  HAPPY,
  SURPRISED,
  SLEEPY,
  FOCUSED
};

AnimationState current_state = IDLE;

// Audio buffer
const int BUFFER_SIZE = 1024;
int16_t audio_buffer[BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  
  // Initialize SPIFFS for audio storage
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }
  
  // Initialize display
  u8g2.setI2CAddress(0x78);
  u8g2.setDisplayRotation(U8G2_R0);
  u8g2.begin();
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(MIC_PIN, INPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  
  // Initialize I2S for audio
  setupI2S();
  
  // Initialize eyes
  reset_eyes();
  
  // Startup animation
  startup_sequence();
  
  Serial.println("Smart Eye Robot Ready!");
  Serial.println("Commands:");
  Serial.println("- Press button to start voice interaction");
  Serial.println("- Send 'Ax' for animation x");
  Serial.println("- Send 'T:text' for text-to-speech");
  Serial.println("- Send 'E:emotion' for emotion (happy/sad/surprised/angry/sleepy)");
}

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void startup_sequence() {
  display_clearDisplay();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(20, 40, "Hello!");
  u8g2.drawStr(15, 60, "I'm awake");
  display_display();
  delay(2000);
  
  wakeup_animation();
}

void display_clearDisplay() {
  u8g2.clearBuffer();   
}

void display_fillRoundRect(int x, int y, int w, int h, int r, int color) {
  u8g2.setDrawColor(color);
  
  if(w < 2*(r+1)) r = (w/2)-1;
  if(h < 2*(r+1)) r = (h/2)-1;
  
  u8g2.drawRBox(x, y, w<1?1:w, h<1?1:h, r);
}

void display_display() {
  u8g2.sendBuffer();
}

void display_fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color) {
  u8g2.setDrawColor(color);
  u8g2.drawTriangle(x0, y0, x1, y1, x2, y2);
}

void draw_eyes(bool update = true) {
  display_clearDisplay();
  
  // Draw left eye
  int x = eyes.left_eye_x - eyes.left_eye_width/2;
  int y = eyes.left_eye_y - eyes.left_eye_height/2;
  display_fillRoundRect(x, y, eyes.left_eye_width, eyes.left_eye_height, eyes.corner_radius, COLOR_WHITE);
  
  // Draw right eye
  x = eyes.right_eye_x - eyes.right_eye_width/2;
  y = eyes.right_eye_y - eyes.right_eye_height/2;
  display_fillRoundRect(x, y, eyes.right_eye_width, eyes.right_eye_height, eyes.corner_radius, COLOR_WHITE);
  
  // Add pupils for more expression
  if(current_state == FOCUSED || current_state == THINKING) {
    draw_pupils();
  }
  
  if(update) {
    display_display();
  }
}

void draw_pupils() {
  // Small pupils for focused look
  int pupil_size = 8;
  display_fillRoundRect(eyes.left_eye_x - pupil_size/2, eyes.left_eye_y - pupil_size/2, 
                       pupil_size, pupil_size, pupil_size/2, COLOR_BLACK);
  display_fillRoundRect(eyes.right_eye_x - pupil_size/2, eyes.right_eye_y - pupil_size/2, 
                       pupil_size, pupil_size, pupil_size/2, COLOR_BLACK);
}

void reset_eyes() {
  eyes.left_eye_height = REF_EYE_HEIGHT;
  eyes.left_eye_width = REF_EYE_WIDTH;
  eyes.right_eye_height = REF_EYE_HEIGHT;
  eyes.right_eye_width = REF_EYE_WIDTH;
  
  eyes.left_eye_x = SCREEN_WIDTH/2 - REF_EYE_WIDTH/2 - REF_SPACE_BETWEEN_EYE/2;
  eyes.left_eye_y = SCREEN_HEIGHT/2;
  eyes.right_eye_x = SCREEN_WIDTH/2 + REF_EYE_WIDTH/2 + REF_SPACE_BETWEEN_EYE/2;
  eyes.right_eye_y = SCREEN_HEIGHT/2;
  
  eyes.corner_radius = REF_CORNER_RADIUS;
  eyes.is_blinking = false;
  eyes.last_blink = millis();
  eyes.last_idle_move = millis();
}

// Enhanced animations
void wakeup_animation() {
  // Eyes closed
  eyes.left_eye_height = 2;
  eyes.right_eye_height = 2;
  eyes.corner_radius = 0;
  draw_eyes();
  delay(500);
  
  // Gradual opening
  for(int h = 2; h <= REF_EYE_HEIGHT; h += 3) {
    eyes.left_eye_height = h;
    eyes.right_eye_height = h;
    eyes.corner_radius = min(h/2, REF_CORNER_RADIUS);
    draw_eyes();
    delay(50);
  }
  
  reset_eyes();
  draw_eyes();
}

void blink_animation(int intensity = 1) {
  int blink_speed = 12 * intensity;
  int original_height = eyes.left_eye_height;
  
  // Close eyes
  for(int i = 0; i < 3; i++) {
    eyes.left_eye_height -= blink_speed;
    eyes.right_eye_height -= blink_speed;
    eyes.left_eye_width += 2;
    eyes.right_eye_width += 2;
    draw_eyes();
    delay(20);
  }
  
  // Open eyes
  for(int i = 0; i < 3; i++) {
    eyes.left_eye_height += blink_speed;
    eyes.right_eye_height += blink_speed;
    eyes.left_eye_width -= 2;
    eyes.right_eye_width -= 2;
    draw_eyes();
    delay(20);
  }
  
  eyes.last_blink = millis();
}

void happy_animation() {
  current_state = HAPPY;
  reset_eyes();
  draw_eyes();
  
  // Draw smile triangles
  int offset = REF_EYE_HEIGHT/2;
  for(int i = 0; i < 15; i++) {
    display_fillTriangle(
      eyes.left_eye_x - eyes.left_eye_width/2 - 1, 
      eyes.left_eye_y + offset, 
      eyes.left_eye_x + eyes.left_eye_width/2 + 1, 
      eyes.left_eye_y + 8 + offset, 
      eyes.left_eye_x - eyes.left_eye_width/2 - 1,
      eyes.left_eye_y + eyes.left_eye_height + offset,
      COLOR_BLACK
    );
    
    display_fillTriangle(
      eyes.right_eye_x + eyes.right_eye_width/2 + 1, 
      eyes.right_eye_y + offset, 
      eyes.right_eye_x - eyes.right_eye_width/2 - 1, 
      eyes.right_eye_y + 8 + offset, 
      eyes.right_eye_x + eyes.right_eye_width/2 + 1,
      eyes.right_eye_y + eyes.right_eye_height + offset,
      COLOR_BLACK
    );
    
    offset -= 1;
    display_display();
    delay(30);
  }
  
  delay(1500);
  current_state = IDLE;
}

void surprised_animation() {
  current_state = SURPRISED;
  
  // Make eyes bigger
  for(int i = 0; i < 10; i++) {
    eyes.left_eye_width += 2;
    eyes.right_eye_width += 2;
    eyes.left_eye_height += 2;
    eyes.right_eye_height += 2;
    eyes.corner_radius += 1;
    draw_eyes();
    delay(30);
  }
  
  delay(1000);
  
  // Return to normal
  reset_eyes();
  draw_eyes();
  current_state = IDLE;
}

void thinking_animation() {
  current_state = THINKING;
  
  // Look up and to the side
  for(int i = 0; i < 5; i++) {
    eyes.left_eye_y -= 2;
    eyes.right_eye_y -= 2;
    eyes.left_eye_x -= 1;
    eyes.right_eye_x -= 1;
    draw_eyes();
    delay(100);
  }
  
  delay(500);
  
  // Small movements while thinking
  for(int i = 0; i < 10; i++) {
    eyes.left_eye_x += (i % 2 == 0) ? 1 : -1;
    eyes.right_eye_x += (i % 2 == 0) ? 1 : -1;
    draw_eyes();
    delay(200);
  }
  
  reset_eyes();
  current_state = IDLE;
}

void sleepy_animation() {
  current_state = SLEEPY;
  
  // Droopy eyes
  for(int i = 0; i < 8; i++) {
    eyes.left_eye_height -= 3;
    eyes.right_eye_height -= 3;
    eyes.left_eye_y += 1;
    eyes.right_eye_y += 1;
    draw_eyes();
    delay(100);
  }
  
  // Slow blink
  blink_animation(2);
  delay(500);
  blink_animation(3);
  
  current_state = IDLE;
}

void listening_animation() {
  current_state = LISTENING;
  
  // Attentive look - slightly wider eyes
  eyes.left_eye_width += 5;
  eyes.right_eye_width += 5;
  eyes.left_eye_height += 3;
  eyes.right_eye_height += 3;
  
  draw_eyes();
}

void speaking_animation() {
  current_state = SPEAKING;
  
  // Animate while speaking
  for(int i = 0; i < 5; i++) {
    eyes.left_eye_height += (i % 2 == 0) ? 2 : -2;
    eyes.right_eye_height += (i % 2 == 0) ? 2 : -2;
    draw_eyes();
    delay(150);
  }
  
  current_state = IDLE;
  reset_eyes();
}

void idle_animation() {
  unsigned long now = millis();
  
  // Natural blinking
  if(now - eyes.last_blink > random(3000, 8000)) {
    blink_animation();
  }
  
  // Subtle eye movements
  if(now - eyes.last_idle_move > random(5000, 12000)) {
    int move_x = random(-3, 4);
    int move_y = random(-2, 3);
    
    eyes.left_eye_x += move_x;
    eyes.right_eye_x += move_x;
    eyes.left_eye_y += move_y;
    eyes.right_eye_y += move_y;
    
    draw_eyes();
    delay(500);
    
    // Return gradually
    for(int i = 0; i < 3; i++) {
      eyes.left_eye_x -= move_x/3;
      eyes.right_eye_x -= move_x/3;
      eyes.left_eye_y -= move_y/3;
      eyes.right_eye_y -= move_y/3;
      draw_eyes();
      delay(100);
    }
    
    eyes.last_idle_move = now;
  }
  
  // Pupil size variation
  if(random(0, 100) < 2) {
    int size_change = random(-2, 3);
    eyes.left_eye_width += size_change;
    eyes.right_eye_width += size_change;
    draw_eyes();
    delay(200);
    eyes.left_eye_width -= size_change;
    eyes.right_eye_width -= size_change;
    draw_eyes();
  }
}

// Audio processing
bool detectVoiceActivity() {
  size_t bytes_read;
  i2s_read(I2S_NUM_0, audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);
  
  // Simple voice activity detection
  long sum = 0;
  for(int i = 0; i < BUFFER_SIZE; i++) {
    sum += abs(audio_buffer[i]);
  }
  
  int average = sum / BUFFER_SIZE;
  return average > 1000; // Threshold for voice detection
}

// Communication with Python script
void sendToLLM(String user_input) {
  // Send to Python script via Serial
  Serial.println("LLM_REQUEST:" + user_input);
  
  // Show thinking animation
  thinking_animation();
}

void processLLMResponse(String response) {
  // Parse LLM response for emotion and text
  int emotion_start = response.indexOf("EMOTION:");
  int text_start = response.indexOf("TEXT:");
  
  if(emotion_start != -1) {
    String emotion = response.substring(emotion_start + 8, text_start);
    emotion.trim();
    
    if(emotion == "happy") happy_animation();
    else if(emotion == "surprised") surprised_animation();
    else if(emotion == "thinking") thinking_animation();
    else if(emotion == "sleepy") sleepy_animation();
  }
  
  if(text_start != -1) {
    String text = response.substring(text_start + 5);
    text.trim();
    
    // Simulate speaking
    speaking_animation();
    
    // Send to TTS
    Serial.println("TTS:" + text);
  }
}

void loop() {
  unsigned long now = millis();
  
  // Check for button press (start voice interaction)
  if(digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    if(digitalRead(BUTTON_PIN) == LOW) {
      listening_animation();
      is_listening = true;
      Serial.println("Listening...");
      
      // Wait for voice or timeout
      unsigned long listen_start = millis();
      while(millis() - listen_start < 5000) {
        if(detectVoiceActivity()) {
          Serial.println("Voice detected, processing...");
          // In real implementation, record and send audio
          sendToLLM("User spoke something"); // Placeholder
          break;
        }
        delay(10);
      }
      
      is_listening = false;
      reset_eyes();
      draw_eyes();
    }
  }
  
  // Check for serial commands
  if(Serial.available()) {
    String data = Serial.readString();
    data.trim();
    
    if(data.startsWith("A")) {
      // Animation command
      int anim = data.substring(1).toInt();
      launchAnimation(anim);
    }
    else if(data.startsWith("E:")) {
      // Emotion command
      String emotion = data.substring(2);
      if(emotion == "happy") happy_animation();
      else if(emotion == "surprised") surprised_animation();
      else if(emotion == "thinking") thinking_animation();
      else if(emotion == "sleepy") sleepy_animation();
    }
    else if(data.startsWith("LLM_RESPONSE:")) {
      // Response from LLM
      String response = data.substring(13);
      processLLMResponse(response);
    }
    else if(data.startsWith("T:")) {
      // Text to speech
      String text = data.substring(2);
      speaking_animation();
    }
  }
  
  // Idle animations when not interacting
  if(current_state == IDLE && now - last_interaction > IDLE_TIMEOUT) {
    idle_animation();
  }
  
  delay(10);
}

void launchAnimation(int index) {
  last_interaction = millis();
  
  switch(index) {
    case 0: wakeup_animation(); break;
    case 1: reset_eyes(); draw_eyes(); break;
    case 2: blink_animation(); break;
    case 3: happy_animation(); break;
    case 4: surprised_animation(); break;
    case 5: thinking_animation(); break;
    case 6: sleepy_animation(); break;
    case 7: listening_animation(); break;
    case 8: speaking_animation(); break;
    default: idle_animation(); break;
  }
}