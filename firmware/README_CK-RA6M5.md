## 📁 Firmware Directory Structure (CK-RA6M5)

All embedded source code and hardware configurations for the microcontroller are located in the `CK-RA6M5` directory. This project is pre-configured with standard settings and can be directly imported into the IDE for immediate use.

### 🚀 e2 studio Import Guide
The `CK-RA6M5` directory contains all necessary hidden configuration files (`.project`, `.cproject`) and system configurations (`configuration.xml`). You can open and compile the source code by following these steps:
1. Open the **Renesas e2 studio** IDE.
2. Select **File** -> **Import...** -> **General** -> **Existing Projects into Workspace**.
3. Browse and select the path to this `CK-RA6M5` directory.
4. Click **Generate Project Content** (inside the `configuration.xml` file) and proceed to **Build** the project.

### 🏗️ Source Code Architecture Layout

The source code is clearly separated into auto-generated system configurations (FSP), hardware sensor drivers, and the machine learning library.

CK-RA6M5/
 ├── configuration.xml          # Core FSP configuration file (Pins, Clocks, I2C, UART...)
 ├── .project / .cproject       # Eclipse/e2 studio project recognition files
 ├── ra/ , ra_cfg/ , ra_gen/    # Auto-generated HAL/Driver configuration code by e2 studio
 ├── script/                    # Linker script files (.ld) for microcontroller memory configuration
 └── src/                       # 🌟 Main application source code directory
      │
      ├── Include/              # Centralized directory for all header files (.h)
      │    ├── hs3001.h         # Header for the temperature/humidity sensor
      │    ├── zmod4410.h       # Header for the IAQ gas sensor
      │    └── SEGGER_RTT...    # Header for the RTT debug library
      │
      ├── ML_model/             # Isolated directory for AI libraries and models
      │    ├── edge-impulse-sdk/# Core C++ library for DSP signal processing and inference
      │    ├── model-parameters/# Contains feature extraction parameters of the model
      │    └── tflite-model/    # Neural network model compiled into TensorFlow Lite Micro
      │
      ├── hal_entry.c           # The main() function, managing initialization logic and the main loop
      ├── hs3001.c              # I2C driver for reading data from the HS3001 sensor
      ├── zmod4410.c            # I2C driver for controlling and reading data from the ZMOD4410 sensor
      ├── ei_main.cpp           # C++ wrapper connecting raw sensor data with the AI inference block
      ├── ei_classifier_...cpp  # Hardware-specific porting configurations for Edge Impulse
      └── SEGGER_RTT.c/.printf.c# High-speed logging library via J-Link instead of traditional UART