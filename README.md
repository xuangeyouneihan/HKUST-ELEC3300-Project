# HKUST-ELEC3300-Project

[English](README_en.md)

![预览图](preview.jpg)

香港科技大学 ELEC3300 嵌入式开发的课设，只是一个玩具。选定的内容是一个写字机器人，使用野火指南者（STM32F103VET6）、A4988 芯片、42 步进电机、大鱼写字机器人的 DIY 硬件结构开发

⚠屎山警告⚠：我们在开发此项目时曾多次更改方案，留下了一大堆没用的函数及功能，请谨慎使用

## 功能

按下 K1 画一个有点瘆人的福字，按下 K2 画一个圆和一个正五边形，使用 [pc 文件夹](pc)下的电脑端 UI 并通过 USB CDC 操控机器人写字（只支持特定字体）

## 编译和使用方法

本项目的硬件部分基于[大鱼DIY](https://space.bilibili.com/488684813)的写字机器人，组装方法建议直接看[他的视频](https://www.bilibili.com/video/BV1cK411X7aa)（但写字速度要比原版慢得多，功能也少得多，毕竟这只是一个课设）。3 块 A4988 分别插在 CNC Shield 的 X、Y、Z 插槽上。X 连接右上角的电机，Y 连接左上角的电机，Z 连接控制抬笔落笔的电机。CNC Shield 上针脚和 STM32 上针脚的对应关系如下：

| CNC Shield | STM32 |
| ---------- | ----- |
| 5V         | 3V3   |
| GND        | GND   |
| EN         | C7    |
| X.STEP     | A3    |
| X.DIR      | B9    |
| Y.STEP     | A2    |
| Y.DIR      | B10   |
| Z.STEP     | A8    |
| Z.DIR      | B12   |

本项目的 STM32 部分位于 [stm32 文件夹](stm32)，是一个 CMake 项目。开发时未使用 CubeIDE，而是使用的 CubeMX、CubeCLT 和 VS Code（安装 STM32 插件），使用的 CMake 配置为 Debug，STM32 的固件版本为 1.8.4。如果使用其他 IDE 请考虑用 CubeMX 或 CubeIDE 打开 [HKUST-ELEC3300-Project.ioc](stm32/HKUST-ELEC3300-Project.ioc)，更改 Toolchain / IDE 选项为你使用的 IDE，并重新生成代码。如果还是会遇到找不到各种文件或各种不同IDE直接的问题，请将 [HKUST-ELEC3300-Project.ioc](stm32/HKUST-ELEC3300-Project.ioc) 复制到一个新的空文件夹，打开复制的 IOC 文件，重新用 CubeMX 或 CubeIDE 生成代码，然后复制原 CMake 项目里的 [Core/Src/main.c](stm32/Core/Src/main.c)、[Core/Inc/main.h](stm32/Core/Inc/main.h)、[Core/Src/cJSON.c](stm32/Core/Src/cJSON.c)、[Core/Inc/cJSON.h](stm32/Core/Inc/cJSON.h)、[USB_DEVICE/App/usbd_cdc_if.c](stm32/USB_DEVICE/App/usbd_cdc_if.c)、[USB_DEVICE/App/usbd_cdc_if.h](stm32/USB_DEVICE/App/usbd_cdc_if.h) 到对应的位置（cJSON.c 和 cJSON.h 也可以直接从 [cJSON](https://github.com/DaveGamble/cJSON) 那里获取），用 IDE 打开新的项目文件夹，添加复制后（或下载下来）的 cJSON.c 和 cJSON.h 到项目里（或在项目根目录下 CMakeLists.txt 的“Add user sources here”和“Add user defined include paths”那里添加 cJSON.c 和 cJSON.h），然后再编译并刷入到 STM32 里

本项目的 PC 端配套程序在 [pc 文件夹](pc)下，使用 Python 编写，因此无需编译，只需先安装 Python（我们使用的是 3.13 版本），在该文件夹下打开命令行运行 `pip install -r requirements.txt` 和 `python ui.py`，在弹出的 UI 里调整选项并输入要写的文字，将 STM32 连接到电脑（STM32 侧插入 USB DEVICE 插口），点击“使用 CDC 输出”，选择写字机器人的端口并确定即可使用。若想使用骨架化功能，则需要取消注释 [requirements.txt](pc/requirements.txt) 里面被注释的内容并再次在 [pc 文件夹](pc)下再次运行 `pip install -r requirements.txt`（虽然效果挺惨不忍睹的）。[ui.py](pc/ui.py) 里的“导出 JSON”按钮和 [test.py](pc/test.py) 是测试用的，一般用不着。[example.json](pc/example.json) 是一个示例文件，在 [test.py](pc/test.py) 里打开它能看到一些测试用的文字。最好不要更改字体，原因的话使用别的字体在 [ui.py](pc/ui.py) 里输入，点击“导出 JSON”导出 JSON 数据然后在 [test.py](pc/test.py) 里打开导出的 JSON 文件就会明白——我们最开始处理的就是 TTF 字体，而 TTF 几乎都是轮廓字体很少有单线字体。当我们发现 Hershey 字体这种东西时又已经离 Final Demo 很近了，没时间再写一套解析 Hershey 字体的逻辑

## 作者

[xuangeyouneihan (吃花椒的驴)](https://github.com/xuangeyouneihan)：主要负责数据结构、CDC 传输与 JSON 解析部分

[definothatock](https://github.com/definothatock)：主要负责电机操控及写字机器人的移动算法部分

## 特别鸣谢

[大鱼DIY](https://space.bilibili.com/488684813)：没有他设计的写字机器人硬件结构就没有这个课设

[cJSON](https://github.com/DaveGamble/cJSON)：提供了一个轻量、便捷的 JSON 解析库，方便我们使用 CDC 传输完 JSON 数据后在 STM32 解析

香港科技大学负责 ELEC3300 的教授和 TA 们