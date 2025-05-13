# HKUST-ELEC3300-Project

[简体中文](README.md)

**Note: The original document is in Simplified Chinese, and this document is translated by DeepSeek from the Chinese document. In case of conflict, the Chinese document prevails.**

![Preview](preview.jpg)

Embedded development course project for ELEC3300 at The Hong Kong University of Science and Technology. This is a toy-level implementation of a writing robot using Embedfire 指南者 (STM32F103VET6) development board, A4988 drivers, NEMA 17 stepper motors, and hardware structure design from [大鱼DIY](https://space.bilibili.com/488684813) Writing Robot.

## Features

- Press K1 to draw a creepy Chinese "福" (Fu) character
- Press K2 to draw a circle and a regular pentagon
- Use the PC-side UI in [pc folder](pc) to control robot writing via USB CDC (supports specific fonts only)

## Compilation and Usage

### Hardware Setup
The hardware design is based on [大鱼DIY](https://space.bilibili.com/488684813) writing robot. Assembly instructions can be found in [his video](https://www.bilibili.com/video/BV1cK411X7aa) (Note: Our implementation has much slower writing speed and much fewer features compared to original design, since this is only a course project). Three A4988 drivers should be installed on X, Y, Z slots of CNC Shield:
- X: Right-top motor
- Y: Left-top motor
- Z: Pen lifting motor

CNC Shield to STM32 pin mapping:

| CNC Shield | STM32 |
|------------|-------|
| 5V         | 3V3   |
| GND        | GND   |
| EN         | C7    |
| X.STEP     | A3    |
| X.DIR      | B9    |
| Y.STEP     | A2    |
| Y.DIR      | B10   |
| Z.STEP     | A8    |
| Z.DIR      | B12   |

### STM32 Development
The STM32 project located in [stm32 folder](stm32) is a CMake project. Development environment:
- CubeMX + CubeCLT + VS Code (with STM32 plugin)
- CMake Debug configuration
- STM32 firmware v1.8.4

For other IDEs, if you encounter an issue:
1. Open [HKUST-ELEC3300-Project.ioc](stm32/HKUST-ELEC3300-Project.ioc) with CubeMX/CubeIDE
2. Regenerate code with your preferred Toolchain/IDE
3. Copy core files:
   - `main.c`, `main.h`
   - `cJSON.c`, `cJSON.h` (can also be obtained from [cJSON repo](https://github.com/DaveGamble/cJSON))
   - `usbd_cdc_if.c`, `usbd_cdc_if.h`
4. Add cJSON files to project via copied CMakeLists.txt or IDE

### PC Software

The PC program in [pc folder](pc) requires:
1. Python 3.13
2. Install dependencies:  
   ```bash
   pip install -r requirements.txt
   ```
3. Run UI:
   ```bash
   python ui.py
   ```
4. Connect STM32 via USB DEVICE port and select corresponding COM port

Note: 
- For skeletonization feature, uncomment dependencies in [requirements.txt](pc/requirements.txt) (though the feature is undesirable)
- Font changing is not recommended due to TTF contour processing limitations, and when we found Hershey fonts, it was no longer feasible to implement corresponding logic within the time constraints
- [test.py](pc/test.py) and [example.json](pc/example.json) are for testing purposes

## Authors

[xuangeyouneihan (吃花椒的驴)](https://github.com/xuangeyouneihan): Data structures, CDC communication, and JSON parsing

[definothatock](https://github.com/definothatock): Motor control algorithms and movement implementation

## Acknowledgements

- [大鱼DIY](https://space.bilibili.com/488684813): Original hardware design
- [cJSON](https://github.com/DaveGamble/cJSON): Lightweight JSON parser for STM32
- ELEC3300 teaching team at HKUST