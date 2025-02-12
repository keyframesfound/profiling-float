# ESP8266 Stepper Motor Serial Commands

Send these commands through the Serial Monitor at 9600 baud rate.

## Available Commands

### Set Direction (x)
```
x0    // Counter-clockwise rotation
x1    // Clockwise rotation
```

### Set Motor Speed (d)
```
d1000    // Set step delay to 1000 microseconds (slower)
d500     // Set step delay to 500 microseconds (faster)
d100     // Set step delay to 100 microseconds (very fast)
```
Speed limits: 20-5000 microseconds (lower = faster)

### Move Steps (z)
```
z200     // Move 200 steps
z400     // Move 400 steps
z1600    // Move 1600 steps (8 full rotations for a 200-step motor)
```
Maximum steps per command: 3200

### Emergency Stop
```
!        // Send '!' character to immediately stop motor
```