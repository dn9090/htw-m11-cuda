#!/bin/bash

make no_parallism_lodepng
mkdir -p export/

bin/image_modifier_no_parallism grey examples/example_image1_small.png export/example_image1_small_grey.png
bin/image_modifier_no_parallism blur examples/example_image1_small.png export/example_image1_small_blur.png
bin/image_modifier_no_parallism hsv examples/example_image1_small.png export/example_image1_small_hsv.png


make cuda_lodepng

bin/image_modifier_cuda grey examples/example_image1_small.png export/example_image1_small_grey.png
bin/image_modifier_cuda blur examples/example_image1_small.png export/example_image1_small_blur.png
bin/image_modifier_cuda hsv examples/example_image1_small.png export/example_image1_small_hsv.png

