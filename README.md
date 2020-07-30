# Columbia Open Source Ultrasound Summer 2020
To run the program, pull from the master branch and run the main. <b>NOTE:</b> the program may not be able to run on Windows due to the limits of some necessary libraris.

<b> Live Rendering</b>
To test the live rendering code, you may need a Red Pitaya (or other device) as the client to send out the data

## config_file Folder
This folder contains some necessary files for the project, including
* The SSH config file (IP address, username, password);
* STL files for the probes and the 3D texts in GUI;
* Other fonts for GUI.

## data Folder
This folder contains the bytes files for rendering results. These files can be loaded by the program and directly render to some 2D or 3D images for viewing. If you want to see the rendering result of a bytes file, please move the file under this folder where GUI will automatically read the existed file here.

## extern Folder
This is the folder contains all the external libraries used for our project, including
* glad: an OpenGL loading library
* glfw: a library providing a simple API for creating windows, contexts and surfaces, receiving input and events
* glm: a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specification
* [iir1](https://github.com/berndporr/iir1): a library for generating filters based on the input parameters
* imgui: a library for creating the user interface
* Not in the folder, but please make sure your computer has installed [libssh](https://www.libssh.org/) before running our program!

## images Folder
This is the folder to save the screenshots or other pictures you may want to save during the experiment

## source Folder
This folder contains the majority of the CS code worked on this Summer 2020 semester. The folder is divided into 2 sections, DensityMap and Ultrasound


### DensityMap
The DensityMap folder contains the code for the image rendering process (the basic grid structure and the camera and shader classes)

#### DensityMap Class
DensityMap is a class that stores a 3D array of unsigned bytes between 0 and 255 (inclusive) and allows them to be displayed using OpenGL.

##### Methods

<b>DensityMap(int dim)</b>  
Initializes the DensityMap with a cubic array of side length dim.

<b>void clear(int value = 0)</b>  
Fills the whole array with a given value. Defaults to 0.

<b>void writeLine(glm::vec3 p1, glm::vec3 p2, std::vector&lt;unsigned char&gt; vals, WriteMode writeMode = DensityMap::WriteMode::Avg)</b>  
Adds a line of data to the array along the line segment defined by p1 and p2. The more values there are in vals, the smoother the line will be.  
The value written to the cell will be the weighted average of the new and old value. The coefficient used in this formula is determined by setUpdateCoefficient().

<b>void writeCell(unsigned int x, unsigned int y, unsigned int z, unsigned char value)</b>  
Writes to one cell of the buffer on the graphics card.

Multiple data values can fall into the same cell (especially if there is a lot of data), so there are multiple ways to combine them.  
If writeMode is equal to DensityMap::WriteMode::Avg, then all values in the same cell will be averaged, and that value will be written to the cell.  
If writeMode is equal to DensityMap::WriteMode::Max, then the maximum of all the values in the cell will be written (this can be better if your data is sparse).

<b>int getDim()</b>  
Returns the side length of the cube.

<b>void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model)</b>  
Draws the density map and a white box around it to the screen.

<b>void setThreshold(unsigned char value)</b>  
<b>unsigned char getThreshold()</b>  
These set and get the minimum value needed to draw a cell. The fewer cells are drawn, the faster your program will run.

<b>void setUpdateCoefficient(float value)</b>  
<b>float getUpdateCoefficient()</b>  
These set and get the update coefficient used for the weighted average in writeLine().  
If it is 1, then the new value completely overwrites the old value. If it is 0.5, then the mean of the new and old values is taken. If it is 0, then writing new values has no effect (not recommended for obvious reasons).

<b>void setBrightness(float value)</b>  
<b>float getBrightness()</b>  
<b>void setContrast(float value)</b>  
<b>float getContrast()</b>  

These set and get the image brightness and contrast.  
After the cell shades are mapped onto the closed interval [0, 1], the following formula is applied:

```
shade = contrast * (shade - 0.5) + 0.5 + brightness
```

When the brightness is 1, all cells will be white, and when the brightness is -1, all cells will be invisible.  
When the contrast is between 0 and 1, cells will appear less contrasted with their neighbors, and when it is more than 1, they will appear more contrasted with their neighbors.

<b>unsigned char readCell(int x, int y, int z)</b>  
Gets the value at a specific index in the array. Blocks when drawing is happening.

<b>unsigned char readCellInterpolated(float x, float y, float z)</b>  
Gets the interpolated value at a position in the cube. Blocks when drawing is happening.  
x, y, and z must all be on the half-open range [0, 1)

<b>void readLine(glm::vec3 p1, glm::vec3 p2, int numVals, unsigned char* vals)</b>  
Gets the interpolated values along the line between two points and writes them to a given array.  
Make sure at least `numVals` bytes of memory are allocated for `vals`.

##### Movement

There are two movement options, controlled by setting ROTATE_GRID at the top of main.cpp to either true or false.  

If ROTATE_GRID is false, then the camera can be moved around using WASD plus Q and E for upwards and downwards movement. C zooms in the camera.  

If ROTATE_GRID is true, then the camera is stationary, and the grid can be rotated by clicking the left mouse button and dragging. Press R to reset the orientation.  

![The image is in the images folder](https://github.com/ColumbiaOpenSourceUltrasound/Spring-2020-CS/blob/master/images/Density%20Map%20(24%20FPS).png "Code demo")


### Ultrasound Folder
This folder contains the work of the CS team. The folder is divided into 2 sections, the include folder and the source folder. The include folder contains the header files of each code in source folder, so we will only go through the source folder.

#### Data processing
All the data processing methods can be found in the data class, with methods to load and read data files based on the probe used to collect the data. Functions in data.cpp includes:
* Read bytes file to a vector
* Decode the data of format v0.6, v0.7 and v0.8 from the Red Pitaya
* Transfer raw parameters into a format can be drawn by the OpenGL in the 3D space
* Adding filters to the adc samples
* Live rendering server code based on UDP protocol
* Connect to the Red Pitaya through SSH and send any command
* Other helper functions needed in the functions above

##### Methods
<b>void readDataSubmarine(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate)</b>  
Read a data file whose format is v0.6 (the probe type should be the submarine) and render the result  
<b>void readDataWhitefin(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate, bool& error, std::string& errorMessage)</b>  
Read a data file whose format is v0.7. This is for the probe Whitefin used in spring 2020.
<b>void readDataTest(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate)</b>  
Read a data file whose format is v0.8. This is the newest format and is used in this semester (Summer 2020)  
<b>bool connectToProbe(DensityMap& grid, std::string probeIP, std::string username, std::string password, std::string compIP,
                    bool isSubmarine,
                    int lxRangeMin, int lxRangeMax, int lxRes, int servoRangeMin, int servoRangeMax, int servoRes,
                    std::string customCommand,
                    int connectionType, std::string& output, bool& connected, bool& error, std::string& errorMessage
)</b>  
Connect to the Red Pitaya through SSH, with parameters that wanted to send to the probe like the servo angle.  

#### Remote control
The class, socket, in remote.cpp is for the remote control of the Red Pitaya based on SSH. Functions in this class includes:
* Connect and disconnect with a device
* Save and load the configuration information in the config file
* Create an interactive shell with the device
* Send any command or a series of commands to the device

#### GUI
The GUI is made up of the following classes: GUI, Probe, Scale, Marker.

##### GUI Class
This class creates the two panels to the left and right of the screen. It contains widgets such as sliders and input buttons to load new file and change different parameter values. This class creates the scale and marker classes as features that the user can interact with through the GUI.

##### Scale Class
The scale class consists of drawing the scale and rendering the three axes on the screen. The class contains methods to set the positions of the scales and change the distance calculation.

##### Marker Class
The marker class creates two markers on the screen and returns the distance between the markers. The class consists of methods to draw the markers, change their positions, and check for intersections.

##### Probe Class
The probe class loads an STL file and renders it on the screen. It can also take in a file of rotation values for the probe and apply each rotation when drawing the object. 

#### Helper classes
The helper class and the rotation class contain methods needed throughout the program. They contain methods to read an STL file to display on the screen and methods to convert between euler angles and quaternions.


