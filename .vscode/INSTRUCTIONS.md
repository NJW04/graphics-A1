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

sudo apt install build-essential libsdl2-dev libglew-dev libglm-dev  libgl1-mesa-dev libglu1-mesa-dev
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
C - Continually Press to Speed Up Earths Rotation Speed  
Z - Continually Press to Slow Down Earths Rotation Speed  
X - Reset Earths Rotation Speed back to Default  
→ - Continually Press to Speed Up Moons Rotation Speed  
← - Continually Press to Slow Down Moons Rotation Speed  
↓ - Reset Moons Rotation Speed back to Default
esc - Stop the Rendering
W - Tilts the camera upward  
A - Rotates the camera to the left around the vertical (Y) axis  
S - Tilts the camera downward  
D - Rotates the camera to the right around the vertical (Y) axis  
Q - Rolls the camera counter-clockwise around its forward axis  
E - Rolls the camera clockwise around its forward axis

## Additional Features

### Additional Planets

- **Mars**, **Jupiter** and **Venus** have been added, each with its own texture.
- Planets are scaled and spaced out so that they don’t overlap, and each has a fixed rotation speed around the Sun.

### Animated Cloud Texture

- Earth now has a dynamic cloud layer driven by a second RGBA texture (`cloud_texture.png`).
- In the fragment shader I rotate and drift the cloud UVs over time, then blend them over the base Earth texture with configurable strength and brightness.
- I can also add the cloud texture onto any other planet because I made the texture able to be toggled off or on.
- If it isn't clear that the clouds are moving you can use the **P** key press to see it clearly.
