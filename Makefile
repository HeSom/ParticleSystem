
INCLUDES = -Iinclude
LIBRARIES = -Ldependencies

LDFLAGS = $(LIBRARIES) -lglfw3 -lopengl32 -lglu32 -lgdi32 -lcudart -lcuda
NVCCFLAGS ?= $(INCLUDES) -arch compute_30 -Xcompiler -Wall,-Wextra,-fPIC

main: main.cpp app.o glad.o glslShader.o kernels.o particleSystem.o renderer.o
	nvcc $(NVCCFLAGS) $^ -o $@

app.o: app.cpp app.h globals.h
	nvcc -c $(NVCCFLAGS) $< -o $@

glad.o: glad.c globals.h
	nvcc -c $(NVCCFLAGS) $< -o $@

glslShader.o: glslShader.cpp glslShader.h globals.h
	nvcc -c $(NVCCFLAGS) $< -o $@

kernels.o: kernels.cu kernels.h globals.h
	nvcc -c $(NVCCFLAGS) $< -o $@

particleSystem.o: particleSystem.cpp particleSystem.h globals.h
	nvcc -c $(NVCCFLAGS) $< -o $@

renderer.o: renderer.cpp renderer.h globals.h
	nvcc -c $(NVCCFLAGS) $< -o $@
