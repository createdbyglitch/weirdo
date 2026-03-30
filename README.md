# Weirdo AI

A philosophical AI assistant that combines local LLM capabilities with real-time contextual awareness.

## Overview

Weirdo AI is a C++ command-line application that integrates with Ollama's TinyLlama model to provide deeply philosophical, elaborately structured responses to user queries. The system enriches interactions by incorporating real-time weather data and temporal context, creating a unique conversational experience characterized by intellectual rigor and existential depth.

## Features

- **Local LLM Integration**: Uses Ollama's TinyLlama model for completely local AI processing
- **Real-time Context Awareness**: Fetches current weather conditions and time/date information
- **Philosophical Response Style**: Generates responses with:
  - Multi-perspective analysis (mathematical, philosophical, historical, metaphysical)
  - Numbered sections with compelling titles
  - Exploration of counterarguments
  - Existential and transcendent conclusions
- **Rich Terminal Interface**: Color-coded output for enhanced readability
- **Continuous Conversation**: Maintains context throughout the session with updated environmental data

## Prerequisites

- **Operating System**: Linux, macOS, or Windows (with appropriate build tools)
- **Ollama**: Must be installed and running locally
- **TinyLlama Model**: Required model for AI responses
- **libcurl**: For HTTP requests to Ollama API and weather service

## Installation

### 1. Install Dependencies

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install libcurl4-openssl-dev g++ make
```

#### macOS

```bash
brew install curl gcc
```

#### Windows

- Install Cygwin or MSYS2
- Install curl development libraries through your package manager

2. Install Ollama

```bash
curl -fsSL https://ollama.com/install.sh | sh
```

3. Pull TinyLlama Model

```bash
ollama pull tinyllama:1.1b
```

4. Start Ollama Service

```bash
ollama serve
```

5. Compile Weirdo AI

```bash
curl -O https://github.com/createdbyglitch/weirdo/blob/main/weirdo.cc
g++ -o weirdo weirdo.cc -lcurl -std=c++17
```

Usage

1. Ensure Ollama is running:
   ```bash
   ollama serve
   ```
2. Run Weirdo AI:
   ```bash
   ./weirdo
   ```
3. Start conversing with the AI. Commands:
   · Type your question or statement and press Enter
   · Type exit, quit, or q to end the session

Example Interaction

```
╔════════════════════════════════════════════════════════════════╗
║                    W E I R D O   A I   S Y S T E M                  ║
╚════════════════════════════════════════════════════════════════╝
┌─ Contextual Awareness ──────────────────────────────────────────┐
│ Weather: Partly cloudy +22°C 15 km/h
│ Date & Time: Tuesday, November 12, 2024 at 02:35:47 PM
│ Temporal Anchor: 2024, November, day 12
└──────────────────────────────────────────────────────────────────┘
Weirdo AI: Greetings, seeker of knowledge. I am prepared to explore the depths of your inquiry...

You: What is consciousness?
Weirdo AI: [Elaborate philosophical response analyzing consciousness from multiple perspectives...]
```

#### Architecture

- WeirdoAI Class: Core application logic
  - getWeather(): Fetches current weather from wttr.in
  - getDateTime() / getYearMonthDay(): Retrieves temporal context
  - queryOllama(): Interfaces with Ollama API
  - generatePhilosophicalResponse(): Constructs system prompts with contextual data
- MemoryStruct: Custom callback structure for libcurl data handling
- JSON Parsing: Simple extraction methods for Ollama API responses

#### Configuration

The AI uses the following Ollama model settings (modifiable in queryOllama()):

- Model: tinyllama:1.1b
- Temperature: 0.7
- Top P: 0.9
- Top K: 40
- Repeat Penalty: 1.1

## Troubleshooting

Issue Solution
FATAL ERROR: libcurl initialization failed Install libcurl development libraries
ERROR: Unable to connect to Ollama API Ensure ollama serve is running
Weather data unavailable Check internet connection; API fallback is graceful
Slow responses TinyLlama model requires adequate RAM; consider using smaller models

#### Dependencies

- **libcurl**: HTTP client library
- **C++ Standard Library**: iostream, string, memory, array, sstream, ctime, regex, iomanip, algorithm
- Ollama: Local LLM service
- **wttr.in**: Free weather service API

## License

This project is licensed under the GNU General Public License v3.0. See the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for:

- Additional philosophical frameworks
- Support for more Ollama models
- Enhanced contextual awareness
- Performance optimizations
- GUI implementations