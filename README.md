# Sonic feedback for ultrasonic echo-location
See report.pdf for the paper.
License: GPL3

## Build and run
### Laptop part
The path to the arduino serial TTY needs to be changed in `serialreader.c`:
    #define ARDUINO_TTY "/dev/arduinomicro"
    
#### To build and run:
cd code/laptop
    gcc  `pkg-config --libs alsa` -lpthread -lm main.c serialreader.c sinegen.c  -std=gnu11 -o main
    ./main
Observe that the TTY device needs to be available when the program is started.

### Arduino part
Open the `arduino` folder with the Arduino software and Compile & Upload it.

