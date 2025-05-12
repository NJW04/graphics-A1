Make sure you're running Ubuntu/Linux. All installation and execution is done via the terminal.

Navigate to Graphics-A1 Directorry.

```
cd graphics-A1
```

Install the necessary libraries using apt:

```
sudo apt update
```

```
sudo apt install build-essential libsdl2-dev libglew-dev libglm-dev
```

To compile the project, run:

```
make
```

Then to render the sun,earth and moon, run:

```
make run
```

### Key Presses

Whilst the rendering is running you can use these key presses:

P - Toggle Pause for the rotation of solar bodies  
D - Continually Press to Speed Up Earths Rotation Speed  
A - Continually Press to Slow Down Earths Rotation Speed  
S - Reset Earths Rotation Speed back to Default  
→ - Continually Press to Speed Up Moons Rotation Speed  
← - Continually Press to Slow Down Moons Rotation Speed  
↓ - Reset Moons Rotation Speed back to Default
esc - Stop the Rendering
