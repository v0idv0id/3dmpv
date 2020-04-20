# 3dmpv

- [3dmpv](#3dmpv)
  - [How it works](#how-it-works)
  - [Dependencies](#dependencies)
    - [Optional mpv-build](#optional-mpv-build)
  - [Installation / Compile](#installation--compile)
    - [Keyboard and Mouse](#keyboard-and-mouse)
    - [If compile fails but you want to know how it looks like](#if-compile-fails-but-you-want-to-know-how-it-looks-like)
  - [References](#references)
  - [License and Author](#license-and-author)

This is an example for how to use libmpv (mpv) for creating a video texture and modify the perspective using a non-affine projective interpolation. The problem with quads and linear UV interpolation is that it only arbitrary affine transforms are allowed as long as quad stays a parallelogram. To solve this problem projective interpolation is needed (UVQ).

This is a combination of the examples **MPVideoCube** and **PerspectiveTexture** which can be found on my GitHub repository: https://github.com/v0idv0id/



Click on the image below to see the screen capture video of the program on youtube:

[![3dmpv - Example for how to use libmpv (mpv) for creating video textures](http://img.youtube.com/vi/dA8J241K9dw/0.jpg)](https://www.youtube.com/watch?v=dA8J241K9dwg "3dmpv - Example for how to use libmpv (mpv) for creating video textures")

## How it works
* GLFW is used for the the window creation, event handling and  OpenGL context creation.
* libmpv is used to create a render context - this is actually the "Video to Texture" transfer/method. HW-accel is enabled, if it works depends on system and hardware.
  

## Dependencies
* Main dependencies: 
  * libmpv-dev  (at least version 0.30 and with opengl enabled)
  * libglfw3-dev
  * youtube-dl

### Optional mpv-build 
Please do not do this unless you know what you are doing. 

I recommend compile mpv from scratch using https://github.com/mpv-player/mpv-build.git but this is not for the faint hearted. See the "MPVideoCube" example for details on this.

## Installation / Compile
* make sure you do fullfill the requirements: 
  * ``` sudo apt-get install libmpv-dev libglfw3-dev build-essential ```
* To compile the demonstration just use:
  * ``` make  ```
* To run it, use the demonstration commands or your own video file:
  * ``` ./rundemo-video-2160p-60p.sh ``` (a 4K video @ 60fps)
  * ``` ./rundemo-video-1080p-60p.sh ``` (a FullHD video  @ 60fps)
  * ``` ./rundemo-youtube.sh ```  (This only works if youtube-dl is installed and working!)
  * ``` ./3dmpv myvideofile.mp4 ``` (try with your own video files)

### Keyboard and Mouse
* [O] Toggle Grid and Corner overlay
* [V] Increase vignette in non-overlay mode
* [B] Decrease vignette in non-overlay mode
* [A] Toggle animation
* [F] Reset video to fill-window
* [ESC] Quit

Corners of the video can be click-and-dragged with the mouse.



### If compile fails but you want to know how it looks like

If you want to know how the result looks like but you have problems to compile the program then take a look at the YouTube video linked at the top of this file. This is is a screen capture of the program running.

## References
* The main OpenGL concept is based on the examples from https://www.learnopengl.com
* The libmpv usage is roughly based on the mpv-examples https://github.com/mpv-player/mpv-examples/tree/master/libmpv
* The video clips test-1080p-60fps.m4v and test-2160p-60fps.mp4 are (c) copyright 2008, Blender Foundation / www.bigbuckbunny.org
* MPVideoCube - https://github.com/v0idv0id/MPVideoCube
* PerspectiveTexture - https://github.com/v0idv0id/PerspectiveTexture
   
## License and Author
GNU GPLv3 - Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com
