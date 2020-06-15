#!/bin/bash

make cuda_lodepng

bin/image_modifier_cuda grey examples/example_image1_small.png export/example_image1_small_grey.png
bin/image_modifier_cuda blur examples/example_image1_small.png export/example_image1_small_blur.png
bin/image_modifier_cuda hsv examples/example_image1_small.png export/example_image1_small_hsv.png


