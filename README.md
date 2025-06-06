---

🤖 Smart Eye Robot

Smart Eye Robot is an interactive voice-enabled assistant featuring expressive animated eyes, emotional responses, and real-time communication. Powered by open-source LLMs via OpenRouter, this project combines natural conversation, emotional animations on an OLED display, and smooth integration between hardware and software.

Whether you're a hobbyist, researcher, or educator, this project offers a flexible and customizable foundation to build your own AI-powered companion.


---

✨ Key Features

🎭 Expressive Eye Animations
Over 8 unique emotional expressions, including idle states, listening, and reactions.

🗣 Voice Interaction
Supports voice input in both Indonesian and English.

🧠 LLM Integration via OpenRouter
Uses customizable open-source LLMs such as LLaMA, Mistral, or Gemma.

🔊 Natural Text-to-Speech
Converts AI replies into spoken responses.

📱 Real-Time Serial Communication
Efficient Python ↔ ESP32 messaging.

💬 Multi-Mode Interface
Voice conversation, text mode, and animation testing.



---

🧩 Hardware Requirements

ESP32 Setup

ESP32-WROOM-32 Development Board (recommended)

OLED Display: 128x128 (SH1107, Pimoroni compatible)

I2S MEMS Microphone (optional for voice detection)

Speaker (optional for audio playback)

Push Button (for manual triggering)


Wiring Diagram

ESP32        Component
GPIO 21   -> OLED SDA
GPIO 22   -> OLED SCL
GPIO 25   -> I2S WS
GPIO 33   -> I2S SD
GPIO 32   -> I2S SCK
GPIO 34   -> Analog mic (optional)
GPIO 26   -> Speaker PWM
GPIO 0    -> Button (pull-up)


---

💻 Software Requirements

Python 3.7 or later

Arduino IDE (for uploading firmware)

Git (to clone the repository)

Internet connection (for LLM API)

Microphone and speaker (for interaction)


Python Dependencies

Install using:

pip install -r requirements.txt


---

🚀 Quick Start Guide

1. Hardware Setup

Connect ESP32 to the OLED display, microphone, speaker, and button.

Upload esp32_smart_eye.ino using Arduino IDE.

Connect ESP32 to your PC via USB.


2. Software Setup

git clone https://github.com/yourusername/smart-eye-robot.git
cd smart-eye-robot
pip install -r requirements.txt

3. Configure Serial Port

Open python_llm_bridge.py and update:

self.SERIAL_PORT = "COM3"        # For Windows
# or
self.SERIAL_PORT = "/dev/ttyUSB0"  # For Linux/Mac

4. Add Your OpenRouter API Key

Get a key from openrouter.ai and configure:

self.OPENROUTER_API_KEY = "your-api-key-here"
self.MODEL = "meta-llama/llama-4-maverick:free"  # or any available model

5. Run the Robot

python python_llm_bridge.py


---

🧠 How It Works

ESP32 handles eye animations and receives AI responses via serial.

Python script bridges voice input, AI processing, and TTS output.

LLM requests are sent via OpenRouter API to an open-source LLM backend.

Emotions are matched to conversation context for immersive interaction.



---

🎮 Available Commands

Command	Description

voice	Start voice interaction mode
chat	Text-based conversation
test	Run all eye animations
emotion [name]	Trigger a specific eye emotion
say [text]	Direct text-to-speech playback
quit	Exit the program



---

🎭 Emotion System

Supported Eye Expressions

happy – Excited, joyful eyes

surprised – Wide, amazed look

thinking – Focused and contemplative

sleepy – Tired and blinking

focused – Alert and attentive

listening – Ready to receive input

neutral – Calm baseline

confused – Unsure or puzzled


Animation Highlights

Natural blinking and eye movement

Smooth emotion transitions

Pupil movement and size variation

Reactivity during speech activity



---

🔧 Configuration Reference

Voice Recognition Language

language='id-ID'  # For Indonesian
language='en-US'  # For English

Serial Communication

self.SERIAL_PORT = "COM3"
self.BAUD_RATE = 115200


---

📡 Communication Protocol

Python → ESP32

E:[emotion] — Trigger emotion

A:[index] — Trigger animation by index

T:[text] — (Planned) Send text to display


ESP32 → Python

LLM_REQUEST:[text] — Send user input to LLM

Listening... — ESP32 is in listening state

Voice detected — Microphone detected speech



---

🐛 Troubleshooting

Common Issues & Fixes

ESP32 not detected

> Check cables, USB port, and correct COM port.



Voice not recognized

> Confirm mic permissions, reduce ambient noise, check microphone connection.



LLM API errors

> Verify API key, check network, monitor usage limits, try a different model.



Display not working

> Recheck wiring and I2C address (default: 0x78), ensure display compatibility.

---

🧩 Extend & Customize

Add New Eye Animations

1. Define a new function in esp32_smart_eye.ino


2. Add logic to launchAnimation()


3. Map new emotion in Python


4. Use test or emotion [name] to try it



Tweak AI Behavior

Modify prompt or persona template

Switch to a different LLM model via OpenRouter

Adjust temperature, max_tokens, and response style

Add memory or context chaining features



---

🤝 Contributing

We welcome contributions!

1. Fork the repo


2. Create a feature branch


3. Commit your changes


4. Submit a Pull Request




---

📄 License

This project is licensed under the MIT License. See the LICENSE file for more details.


---

🙏 Credits & Thanks

OpenRouter for providing unified access to open-source LLMs

The open-source LLM community: Meta, Mistral, Google, and others

Arduino & ESP32 development community

Contributors to Python voice/TTS libraries

U8g2 OLED graphics library



---

📞 Need Help?

Open an issue on GitHub

Refer to the docs folder for setup guides

Review the troubleshooting tips above



---

Built with ❤ for open-source hardware and AI enthusiasts.


---
