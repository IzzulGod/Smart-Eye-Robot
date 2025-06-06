# 🤖 Smart Eye Robot

Smart Eye Robot is an interactive voice-enabled assistant featuring expressive animated eyes, emotional responses, and real-time communication capabilities. Powered by open-source Large Language Models (LLMs) via OpenRouter, this project seamlessly combines natural conversation, dynamic emotional animations on an OLED display, and robust integration between hardware and software components.

Whether you're a hobbyist, researcher, or educator, this project provides a flexible and highly customizable foundation for building your own AI-powered companion.

## ✨ Key Features

### 🎭 Expressive Eye Animations
- Over 8 unique emotional expressions including idle states, listening modes, and contextual reactions
- Smooth animation transitions with natural blinking and pupil movement

### 🗣️ Voice Interaction
- Multi-language support for voice input (Indonesian and English)
- Real-time speech recognition with ambient noise filtering

### 🧠 LLM Integration via OpenRouter
- Seamless integration with customizable open-source LLMs including LLaMA, Mistral, and Gemma
- Configurable model parameters for optimal performance

### 🔊 Natural Text-to-Speech
- High-quality AI response conversion to spoken audio
- Adjustable voice parameters and language selection

### 📡 Real-Time Serial Communication
- Efficient bidirectional Python ↔ ESP32 messaging protocol
- Low-latency command processing and response handling

### 💬 Multi-Mode Interface
- Voice conversation mode for natural interaction
- Text-based chat interface for debugging
- Animation testing suite for development

## 🧩 Hardware Requirements

### ESP32 Setup
- **Microcontroller**: ESP32-WROOM-32 Development Board (recommended)
- **Display**: 128x128 OLED Display (SH1107, Pimoroni compatible)
- **Audio Input**: I2S MEMS Microphone (optional for voice detection)
- **Audio Output**: Speaker (optional for audio playback)
- **Controls**: Push Button for manual triggering

### Wiring Diagram
```
ESP32 Pin    →    Component
GPIO 21      →    OLED SDA
GPIO 22      →    OLED SCL
GPIO 25      →    I2S WS
GPIO 33      →    I2S SD
GPIO 32      →    I2S SCK
GPIO 34      →    Analog Microphone (optional)
GPIO 26      →    Speaker PWM
GPIO 0       →    Button (with pull-up resistor)
```

## 💻 Software Requirements

### System Requirements
- Python 3.7 or later
- Arduino IDE (for firmware upload)
- Git (for repository management)
- Active internet connection (for LLM API access)
- System microphone and speakers (for interaction)

### Python Dependencies
Install all required packages using:
```bash
pip install -r requirements.txt
```

## 🚀 Quick Start Guide

### 1. Hardware Setup
1. Connect ESP32 to OLED display, microphone, speaker, and button according to wiring diagram
2. Upload `esp32_smart_eye.ino` firmware using Arduino IDE
3. Connect ESP32 to your computer via USB cable

### 2. Software Installation
```bash
git clone https://github.com/yourusername/smart-eye-robot.git
cd smart-eye-robot
pip install -r requirements.txt
```

### 3. Configure Serial Port
Open `python_llm_bridge.py` and update the serial port configuration:
```python
self.SERIAL_PORT = "COM3"          # For Windows systems
# or
self.SERIAL_PORT = "/dev/ttyUSB0"  # For Linux/Mac systems
```

### 4. Add OpenRouter API Key
1. Obtain an API key from [openrouter.ai](https://openrouter.ai)
2. Configure in the Python script:
```python
self.OPENROUTER_API_KEY = "your-api-key-here"
self.MODEL = "meta-llama/llama-4-maverick:free"  # or any available model
```

### 5. Launch the Robot
```bash
python python_llm_bridge.py
```

## 🧠 System Architecture

### Core Components
- **ESP32**: Handles eye animations and receives AI responses via serial communication
- **Python Bridge**: Manages voice input processing, AI API interactions, and TTS output
- **LLM Backend**: Processes natural language requests through OpenRouter API
- **Emotion Engine**: Matches contextual emotions to conversation flow for immersive interaction

### Data Flow
1. Voice input captured by microphone
2. Speech-to-text conversion in Python
3. Text sent to LLM via OpenRouter API
4. AI response processed and emotion determined
5. Response sent to ESP32 for eye animation
6. Text-to-speech playback of AI response

## 🎮 Available Commands

| Command | Description |
|---------|-------------|
| `voice` | Start voice interaction mode |
| `chat` | Enter text-based conversation |
| `test` | Run all eye animation sequences |
| `emotion [name]` | Trigger specific eye emotion |
| `say [text]` | Direct text-to-speech playback |
| `quit` | Exit the program |

## 🎭 Emotion System

### Supported Eye Expressions
- **happy** – Excited, joyful eyes with raised pupils
- **surprised** – Wide, amazed look with enlarged pupils
- **thinking** – Focused and contemplative with side glances
- **sleepy** – Tired expression with slow blinking
- **focused** – Alert and attentive with steady gaze
- **listening** – Ready to receive input with active pupils
- **neutral** – Calm baseline expression
- **confused** – Unsure or puzzled with tilted perspective

### Animation Features
- Natural blinking patterns with randomized timing
- Smooth emotion transitions with interpolation
- Dynamic pupil movement and size variation
- Reactive animations during speech activity
- Contextual emotion matching based on conversation tone

## 🔧 Configuration Reference

### Voice Recognition Settings
```python
language='id-ID'  # For Indonesian language
language='en-US'  # For English language
```

### Serial Communication Parameters
```python
self.SERIAL_PORT = "COM3"
self.BAUD_RATE = 115200
```

### LLM Configuration
```python
self.MODEL = "meta-llama/llama-4-maverick:free"
self.TEMPERATURE = 0.7
self.MAX_TOKENS = 150
```

## 📡 Communication Protocol

### Python → ESP32 Commands
- `E:[emotion]` — Trigger specific emotion animation
- `A:[index]` — Trigger animation by numerical index
- `T:[text]` — Send text to display (planned feature)

### ESP32 → Python Messages
- `LLM_REQUEST:[text]` — Send user input to LLM processing
- `Listening...` — ESP32 is in active listening state
- `Voice detected` — Microphone has detected speech input

## 🐛 Troubleshooting

### Common Issues & Solutions

#### ESP32 Not Detected
- Verify USB cable connection and port availability
- Check correct COM port selection in configuration
- Ensure ESP32 drivers are properly installed

#### Voice Recognition Issues
- Confirm microphone permissions in system settings
- Reduce ambient noise in environment
- Verify microphone hardware connection
- Test with different speech clarity and volume

#### LLM API Errors
- Verify OpenRouter API key validity
- Check network connectivity and firewall settings
- Monitor API usage limits and quotas
- Try alternative LLM models if current model is unavailable

#### Display Problems
- Double-check I2C wiring connections
- Verify OLED display I2C address (default: 0x78)
- Ensure display compatibility with SH1107 controller
- Test with basic display examples first

## 🧩 Extend & Customize

### Adding New Eye Animations
1. Define new animation function in `esp32_smart_eye.ino`
2. Add corresponding logic to `launchAnimation()` switch statement
3. Map new emotion in Python emotion dictionary
4. Test using `emotion [name]` command

### Customizing AI Behavior
- Modify system prompt or persona template for different personalities
- Switch to different LLM models via OpenRouter for varied responses
- Adjust temperature, max_tokens, and response style parameters
- Implement memory or context chaining for conversational continuity

### Hardware Extensions
- Add additional sensors (temperature, humidity, motion)
- Integrate RGB LEDs for enhanced visual feedback
- Implement servo motors for physical movement
- Add LCD display for text output

## 🤝 Contributing

We welcome contributions from the community! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Contribution Guidelines
- Follow existing code style and conventions
- Add appropriate documentation for new features
- Include test cases where applicable
- Update README.md if adding new functionality

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for complete details.

## 🙏 Credits & Acknowledgments

- **OpenRouter** for providing unified access to open-source LLMs
- **Open-source LLM Community**: Meta (LLaMA), Mistral AI, Google (Gemma), and other contributors
- **Arduino & ESP32 Development Community** for extensive hardware support
- **Python Voice/TTS Libraries** contributors for speech processing capabilities
- **U8g2 Graphics Library** for OLED display functionality

## 📞 Support & Help

### Getting Help
- Open an issue on [GitHub Issues](https://github.com/yourusername/smart-eye-robot/issues)
- Refer to the `docs/` folder for detailed setup guides
- Review troubleshooting section above for common problems
- Check the [Wiki](https://github.com/yourusername/smart-eye-robot/wiki) for additional resources

### Community
- Join our [Discord](https://discord.gg/your-server) for real-time support
- Follow development updates on [Twitter](https://twitter.com/your-handle)
- Subscribe to our [YouTube channel](https://youtube.com/your-channel) for tutorials

---

**Built with ❤️ for open-source hardware and AI enthusiasts.**

*Smart Eye Robot - Bringing AI to life through expressive robotics.*
