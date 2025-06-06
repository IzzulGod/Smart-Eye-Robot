#!/usr/bin/env python3
"""
Smart Eye Robot - LLM Bridge (Complete Version)
Connects ESP32 robot with Meta Llama via OpenRouter
Handles voice recognition, LLM processing, and text-to-speech
"""

import serial
import time
import json
import requests
import threading
import queue
import speech_recognition as sr
import pyttsx3
import pyaudio
import wave
import webrtcvad
import collections
import sys
import re
import os
from typing import Dict, List, Optional

class SmartEyeRobot:
    def __init__(self):
        # Configuration
        self.OPENROUTER_API_KEY = "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"
        self.MODEL = "meta-llama/llama-4-maverick:free"
        self.SERIAL_PORT = "COM3"  # Adjust for your system (Linux: /dev/ttyUSB0)
        self.BAUD_RATE = 115200
        
        # Initialize components
        self.serial_conn = None
        self.speech_recognizer = sr.Recognizer()
        self.microphone = sr.Microphone()
        self.tts_engine = pyttsx3.init()
        self.message_queue = queue.Queue()
        
        # Voice activity detection
        self.vad = webrtcvad.Vad(2)  # Aggressiveness level 0-3
        self.sample_rate = 16000
        self.frame_duration = 30  # ms
        self.chunk_size = int(self.sample_rate * self.frame_duration / 1000)
        
        # Conversation context
        self.conversation_history = []
        self.robot_personality = """You are a friendly, intelligent robot with expressive animated eyes on an OLED display. 
        You can show emotions through your eye expressions and animations. When responding, first decide on an emotion 
        based on the context of the conversation, then provide a helpful, engaging response. 
        
        Available emotions and animations:
        - happy: Joyful, excited, pleased
        - surprised: Amazed, shocked, curious
        - thinking: Processing, contemplating, analyzing
        - sleepy: Tired, drowsy, relaxed
        - focused: Concentrated, attentive, alert
        - listening: Attentive, ready to hear
        - neutral: Default, calm state
        
        IMPORTANT: Always format your response as:
        EMOTION: [emotion_name]
        TEXT: [your conversational response in Indonesian or English based on user language]
        
        Keep responses natural, friendly, and conversational. Show personality through your chosen emotions and words.
        You can understand both Indonesian and English. Respond in the same language the user uses."""
        
        # Setup TTS
        self.setup_tts()
        
        # Threading control
        self.running = True
        self.listening_active = False
        
        print("🤖 Smart Eye Robot initialized!")
        print("🔌 Connecting to ESP32...")
        
    def setup_tts(self):
        """Configure text-to-speech engine"""
        try:
            voices = self.tts_engine.getProperty('voices')
            # Try to find a pleasant voice
            for voice in voices:
                if 'female' in voice.name.lower() or 'zira' in voice.name.lower():
                    self.tts_engine.setProperty('voice', voice.id)
                    break
            
            self.tts_engine.setProperty('rate', 180)  # Speed
            self.tts_engine.setProperty('volume', 0.9)  # Volume
            
            # Test TTS
            print("🔊 TTS engine configured successfully")
        except Exception as e:
            print(f"⚠️ TTS setup warning: {e}")
        
    def connect_serial(self) -> bool:
        """Connect to ESP32 via serial"""
        try:
            self.serial_conn = serial.Serial(self.SERIAL_PORT, self.BAUD_RATE, timeout=1)
            time.sleep(2)  # Wait for connection
            print(f"✅ Connected to ESP32 on {self.SERIAL_PORT}")
            
            # Send test command
            self.send_to_esp32("E:happy")
            return True
        except serial.SerialException as e:
            print(f"❌ Failed to connect to ESP32: {e}")
            print("💡 Make sure ESP32 is connected and port is correct")
            print("💡 Try different ports: COM1, COM4, /dev/ttyUSB0, /dev/ttyACM0")
            return False
    
    def send_to_esp32(self, command: str):
        """Send command to ESP32"""
        if self.serial_conn and self.serial_conn.is_open:
            try:
                self.serial_conn.write(f"{command}\n".encode())
                print(f"📤 Sent to ESP32: {command}")
            except Exception as e:
                print(f"❌ Error sending to ESP32: {e}")
    
    def listen_for_voice(self) -> Optional[str]:
        """Listen for voice input with improved recognition"""
        print("🎤 Listening for voice...")
        self.send_to_esp32("E:listening")
        
        try:
            # Adjust for ambient noise
            with self.microphone as source:
                print("📊 Adjusting for ambient noise...")
                self.speech_recognizer.adjust_for_ambient_noise(source, duration=1.0)
            
            # Listen for voice with longer timeout
            print("👂 Ready to listen...")
            with self.microphone as source:
                audio = self.speech_recognizer.listen(
                    source, 
                    timeout=10,  # Wait up to 10 seconds for speech
                    phrase_time_limit=15  # Allow up to 15 seconds of speech
                )
            
            # Convert speech to text - try both languages
            try:
                # Try Indonesian first
                text = self.speech_recognizer.recognize_google(audio, language='id-ID')
                print(f"🗣️ User said (ID): {text}")
                return text
            except sr.UnknownValueError:
                try:
                    # Try English if Indonesian fails
                    text = self.speech_recognizer.recognize_google(audio, language='en-US')
                    print(f"🗣️ User said (EN): {text}")
                    return text
                except sr.UnknownValueError:
                    print("❓ Could not understand audio in either language")
                    self.send_to_esp32("E:confused")
                    self.speak("Maaf, saya tidak bisa mendengar dengan jelas. Coba ulangi lagi.")
                    return None
            except sr.RequestError as e:
                print(f"❌ Speech recognition error: {e}")
                self.speak("Ada masalah dengan pengenalan suara saya.")
                return None
                
        except sr.WaitTimeoutError:
            print("⏰ No speech detected within timeout")
            self.send_to_esp32("E:neutral")
            return None
    
    def call_llm(self, user_input: str) -> Dict:
        """Call LLM via OpenRouter API with improved error handling"""
        print("🧠 Thinking...")
        self.send_to_esp32("E:thinking")
        
        # Build conversation context
        messages = [{"role": "system", "content": self.robot_personality}]
        
        # Add recent conversation history (keep last 6 messages for context)
        for msg in self.conversation_history[-6:]:
            messages.append(msg)
        
        messages.append({"role": "user", "content": user_input})
        
        try:
            response = requests.post(
                "https://openrouter.ai/api/v1/chat/completions",
                headers={
                    "Authorization": f"Bearer {self.OPENROUTER_API_KEY}",
                    "HTTP-Referer": "http://localhost:3000",
                    "X-Title": "Smart Eye Robot",
                    "Content-Type": "application/json"
                },
                json={
                    "model": self.MODEL,
                    "messages": messages,
                    "max_tokens": 250,
                    "temperature": 0.7,
                    "top_p": 0.9,
                    "stream": False
                },
                timeout=30
            )
            
            if response.status_code == 200:
                result = response.json()
                llm_response = result['choices'][0]['message']['content']
                
                # Update conversation history
                self.conversation_history.append({"role": "user", "content": user_input})
                self.conversation_history.append({"role": "assistant", "content": llm_response})
                
                # Keep history manageable
                if len(self.conversation_history) > 12:
                    self.conversation_history = self.conversation_history[-8:]
                
                return self.parse_llm_response(llm_response)
            else:
                print(f"❌ LLM API Error: {response.status_code} - {response.text}")
                return {
                    "emotion": "confused",
                    "text": "Maaf, saya mengalami masalah teknis. Coba lagi ya!"
                }
                
        except requests.exceptions.Timeout:
            print("⏰ LLM request timeout")
            return {
                "emotion": "sleepy",
                "text": "Hmm, saya agak lambat hari ini. Bisa ulangi pertanyaannya?"
            }
        except requests.exceptions.ConnectionError:
            print("🌐 No internet connection")
            return {
                "emotion": "confused",
                "text": "Sepertinya koneksi internet saya bermasalah."
            }
        except Exception as e:
            print(f"❌ LLM Error: {e}")
            return {
                "emotion": "confused",
                "text": "Ada yang tidak beres dengan otak saya. Mohon bersabar ya!"
            }
    
    def parse_llm_response(self, response: str) -> Dict:
        """Parse LLM response for emotion and text"""
        # Look for EMOTION: and TEXT: patterns
        emotion_match = re.search(r'EMOTION:\s*(\w+)', response, re.IGNORECASE)
        text_match = re.search(r'TEXT:\s*(.*)', response, re.IGNORECASE | re.DOTALL)
        
        emotion = emotion_match.group(1).lower() if emotion_match else "neutral"
        text = text_match.group(1).strip() if text_match else response.strip()
        
        # Clean up text - remove any remaining formatting
        text = re.sub(r'EMOTION:.*?TEXT:\s*', '', text, flags=re.IGNORECASE | re.DOTALL)
        text = text.strip()
        
        # If text is empty, use the whole response
        if not text:
            text = response.strip()
        
        # Validate emotion
        valid_emotions = ['happy', 'surprised', 'thinking', 'sleepy', 'focused', 'listening', 'neutral', 'confused']
        if emotion not in valid_emotions:
            emotion = 'neutral'
        
        return {"emotion": emotion, "text": text}
    
    def speak(self, text: str):
        """Convert text to speech and send emotion to ESP32"""
        print(f"🗣️ Robot says: {text}")
        
        try:
            # Send speaking animation to ESP32
            self.send_to_esp32("E:speaking")
            
            # Speak the text
            self.tts_engine.say(text)
            self.tts_engine.runAndWait()
            
            # Return to neutral after speaking
            time.sleep(0.5)
            self.send_to_esp32("E:neutral")
            
        except Exception as e:
            print(f"❌ TTS Error: {e}")
    
    def handle_esp32_messages(self):
        """Handle incoming messages from ESP32 in separate thread"""
        while self.running:
            try:
                if self.serial_conn and self.serial_conn.in_waiting > 0:
                    message = self.serial_conn.readline().decode().strip()
                    if message:
                        print(f"📥 ESP32: {message}")
                        
                        # Handle different message types from ESP32
                        if message.startswith("LLM_REQUEST:"):
                            user_input = message[12:]  # Remove "LLM_REQUEST:" prefix
                            self.process_user_input(user_input)
                        elif message == "Listening...":
                            print("🎤 ESP32 started listening")
                        elif message.startswith("Voice detected"):
                            print("👂 ESP32 detected voice")
                            
            except Exception as e:
                if self.running:  # Only print error if we're still supposed to be running
                    print(f"❌ Error reading from ESP32: {e}")
            
            time.sleep(0.1)
    
    def process_user_input(self, user_input: str):
        """Process user input and generate response"""
        if not user_input.strip():
            return
        
        # Get LLM response
        llm_result = self.call_llm(user_input)
        
        # Send emotion to ESP32
        self.send_to_esp32(f"E:{llm_result['emotion']}")
        
        # Wait a moment for animation
        time.sleep(0.5)
        
        # Speak the response
        self.speak(llm_result['text'])
    
    def interactive_mode(self):
        """Interactive mode for voice conversation"""
        print("\n🎙️ Interactive Voice Mode")
        print("Press Ctrl+C to exit")
        print("Say something to start conversation...")
        
        try:
            while self.running:
                # Listen for voice input
                user_input = self.listen_for_voice()
                
                if user_input:
                    # Process the input
                    self.process_user_input(user_input)
                else:
                    # Return to idle after failed listening
                    self.send_to_esp32("E:neutral")
                
                # Small delay between listening cycles
                time.sleep(1)
                
        except KeyboardInterrupt:
            print("\n👋 Exiting interactive mode...")
            
    def test_animations(self):
        """Test all available animations"""
        print("\n🎭 Testing animations...")
        animations = ['happy', 'surprised', 'thinking', 'sleepy', 'focused', 'listening', 'neutral']
        
        for emotion in animations:
            print(f"Testing {emotion}...")
            self.send_to_esp32(f"E:{emotion}")
            time.sleep(2)
        
        print("Animation test complete!")
    
    def run(self):
        """Main run method"""
        # Connect to ESP32
        if not self.connect_serial():
            print("❌ Cannot continue without ESP32 connection")
            return
        
        # Start ESP32 message handler thread
        esp32_thread = threading.Thread(target=self.handle_esp32_messages, daemon=True)
        esp32_thread.start()
        
        print("\n🤖 Smart Eye Robot is ready!")
        print("\nAvailable commands:")
        print("1. 'test' - Test all animations")
        print("2. 'voice' - Start voice interaction mode")
        print("3. 'chat' - Text chat mode")
        print("4. 'quit' - Exit program")
        
        try:
            while self.running:
                command = input("\n> ").strip().lower()
                
                if command == 'quit' or command == 'exit':
                    break
                elif command == 'test':
                    self.test_animations()
                elif command == 'voice':
                    self.interactive_mode()
                elif command == 'chat':
                    self.text_chat_mode()
                elif command.startswith('say '):
                    text = command[4:]
                    self.speak(text)
                elif command.startswith('emotion '):
                    emotion = command[8:]
                    self.send_to_esp32(f"E:{emotion}")
                else:
                    # Treat as conversation input
                    if command:
                        self.process_user_input(command)
        
        except KeyboardInterrupt:
            print("\n👋 Goodbye!")
        
        finally:
            self.running = False
            if self.serial_conn:
                self.serial_conn.close()
    
    def text_chat_mode(self):
        """Text-based chat mode"""
        print("\n💬 Text Chat Mode")
        print("Type 'exit' to return to main menu")
        
        while True:
            try:
                user_input = input("\nYou: ").strip()
                
                if user_input.lower() in ['exit', 'quit', 'back']:
                    break
                
                if user_input:
                    self.process_user_input(user_input)
                    
            except KeyboardInterrupt:
                break
        
        print("Returning to main menu...")

def main():
    """Main function"""
    print("🤖 Smart Eye Robot - LLM Bridge")
    print("=" * 40)
    
    # Create robot instance
    robot = SmartEyeRobot()
    
    # Run the robot
    robot.run()

if __name__ == "__main__":
    main()