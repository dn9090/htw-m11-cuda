## Overview

This repository consists of three projects.
The first one, called *image modifier no parallism*, and the second one, called *image modifier cuda*
share a lot of the same codebase.
The third project *Image Modifier ImageSharp* is written in C# and builds on top of the ImageSharp library.

## Requirements

### image modifier no parallism
* OpenCV2 with core, imgproc, imgcodecs, highgui

### image modifier cuda
* OpenCV2 with core, imgproc, imgcodecs, highgui
* Nvidia CUDA

### Image Modifier ImageSharp
* dotnet core 3
* ImageSharp 1

## Building

The Makefile contains the recipe *no_parallism* for the first and *cuda* for the second project.
If you want to build and test the first project run the shell script *run_no_parallism.sh*.

The third project is build via dotnet core run.

## Documentation
Can be found [here](docs/Documentation.md).