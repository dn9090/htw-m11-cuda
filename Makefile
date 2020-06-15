GCC=g++
NVCC=nvcc
CUDA_ARCH=compute_50
LD=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

pre-build:
	mkdir -p ./bin

no_parallism: pre-build
	$(GCC) -x c++ src/no_parallism.cpp src/main.cpp $(LD) -o bin/image_modifier_no_parallism

no_parallism_cimg: pre-build
	$(GCC) -x c++ src/no_parallism.cpp src/main_cimg.cpp -lpng -ljpeg -o bin/image_modifier_no_parallism

cuda: pre-build
	$(NVCC) -arch=$(CUDA_ARCH) -x cu src/cuda.cu src/main.cpp $(LD) -o bin/image_modifier_cuda

cuda_cimg: pre-build
	$(NVCC) -arch=$(CUDA_ARCH) -x cu src/cuda.cu src/main_cimg.cpp -lpng -ljpeg -o bin/image_modifier_cuda

all: no_parallism cuda

clean: pre-build
	rm -rf ./bin/*