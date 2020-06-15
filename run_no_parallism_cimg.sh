#!/bin/bash

make no_parallism_cimg

bin/image_modifier_no_parallism grey examples/example_image1_small.png export/example_image1_small_grey.png
bin/image_modifier_no_parallism emboss examples/example_image1_small.png export/example_image1_small_emboss.png
bin/image_modifier_no_parallism blur examples/example_image1_small.png export/example_image1_small_blur.png
bin/image_modifier_no_parallism hsv examples/example_image1_small.png export/example_image1_small_hsv.png