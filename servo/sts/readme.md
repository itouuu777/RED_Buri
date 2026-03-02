Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/ststm32/nucleo_f446re.html
PLATFORM: ST STM32 (19.4.0) > ST Nucleo F446RE
HARDWARE: STM32F446RET6 180MHz, 128KB RAM, 512KB Flash
DEBUG: Current (stlink) On-board (stlink) External (blackmagic, cmsis-dap, jlink)
PACKAGES: 
 - framework-stm32cubef4 @ 1.28.1 
 - tool-ldscripts-ststm32 @ 0.2.0 
 - toolchain-gccarmnoneeabi @ 1.70201.0 (7.2.1)
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 62 compatible libraries
Scanning dependencies...
Dependency Graph
|-- c620_can
|-- c620_control
|-- sts3215
Building in release mode
Compiling .pio/build/nucleo_f446re/lib560/sts3215/sts3215.o
lib/sts3215/sts3215.cpp: In member function 'int16_t STS3215::getPosition(uint32_t)':
lib/sts3215/sts3215.cpp:93:10: error: 'memcpy' is not a member of 'std'
     std::memcpy(frame,     hdr,  5);
          ^~~~~~
lib/sts3215/sts3215.cpp:94:10: error: 'memcpy' is not a member of 'std'
     std::memcpy(frame + 5, body, len);
          ^~~~~~
*** [.pio/build/nucleo_f446re/lib560/sts3215/sts3215.o] Error 1
