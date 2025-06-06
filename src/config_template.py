#!/usr/bin/env python3
"""
Smart Eye Robot - Configuration Template
Copy this file to 'config.py' and fill in your settings
"""

# OpenRouter API Configuration
OPENROUTER_API_KEY = "your-openrouter-api-key-here"
MODEL = "your-openrouter-models-here"

# Serial Configuration
SERIAL_PORT = "COM3"  # Windows: COM3, COM4, etc. | Linux: /dev/ttyUSB0, /dev/ttyACM0
BAUD_RATE = 115200

# Voice Recognition Settings
VOICE_LANGUAGE_PRIMARY = "id-ID"    # Indonesian
VOICE_LANGUAGE_FALLBACK = "en-US"   # English
LISTEN_TIMEOUT = 10                 # seconds
PHRASE_TIME_LIMIT = 15              # seconds

# Text-to-Speech Settings
TTS_RATE = 180      # Speech rate (words per minute)
TTS_VOLUME = 0.9    # Volume (0.0 to 1.0)

# LLM Settings
MAX_TOKENS = 250
TEMPERATURE = 0.7
TOP_P = 0.9
REQUEST_TIMEOUT = 30  # seconds

# Conversation Settings
MAX_HISTORY_MESSAGES = 12  # Keep conversation history
IDLE_TIMEOUT = 10000       # milliseconds before idle animations

# Robot Personality (optional customization)
ROBOT_PERSONALITY = """You are a friendly, intelligent robot with expressive animated eyes on an OLED display. 
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
