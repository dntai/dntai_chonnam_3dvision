# 3-D VISION FINAL PROJECT
Chonnam National University, Gwangju, South Korea<br/>
2017 3-D Vision Class  
Professor: **Chilwoo Lee**
Student: Do Nhu Tai
**Target: Building a application for argument reality video**

## Aruco Create Board
![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/aruco_create_board.png)


## Argument Marker Detection
![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/aruco_marker_detection.png)


## Calibration Camera
![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/calibration_camera.png)


![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/aruco_calibration_camera.png)


## Argument Reality 2D Camera
![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/argument_reality_2d_camera.png)


## Requirements
1. Windows 10 64 Bits
2. Visual Sutdio 2015
3. OpenCV 3.3 with built-in modules: Aruco, OpenGL Support, xfeature2d, feature2d, calib3d, viz
4. VTK 8.0
5. MFC


## Project Layout
![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/modules.png)


### Main:
1. CalibrateCamera: Using exporting calibration camera parameters
2. DetectChessboard: Checking dection chess board
3. Vision3D (GUI): Main Application

![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/main_gui.png)


### Examples: References from OpenCV witj Aruco, Calib3D Modules, and OpenCV3 Cookbook for Camera Pose, Matching Camera
1. ArucoCalibrateCamera
2. ArucoCreateBoard
3. ArucoCreateMarker
4. ArucoDetectBoard
5. ArucoDetectMarkers
6. CameraPose
7. MatchingCamera


### Others: Testing projects
1. CamCalibEx
2. Vision3DPrj


## How to run source code
**Project has already pre-configure with *.dll in libraries.**
1. Download project from github
2. Run *.bat in install directories


## How to build source code
**Project has already pre-configure with *.dll in libraries.**
1. Download project from github
2. Open Visual Sutdio 2015
3. Build every project in solution
4. Modify libraries in properti sheet of every project (project includes necessary libraries and headers)
![alt text](https://raw.githubusercontent.com/dntai/dntai_chonnam_3dvision/master/images/property_sheet.png)


### References
[1] Laganière, Robert. "OpenCV 3 Computer Vision Application Programming Cookbook". Packt Publishing Ltd, 2017.

[2] Ma, Y., Soatto, S., Kosecka, J., & Sastry, S. S. (2012). "An invitation to 3-d vision: from images to geometric models". Springer Science & Business Media.


### Personal information
**Do Nhu Tai**
Supervisor: Professor **Kim Soo-Hyung**
Pattern Recognition Lab  
**Chonnam National University, Korea**
E-mail: donhutai@gmail.com