all:
	g++ -Wall -o graphics_springs graphics_springs.cpp graphics.h forces.h forces.cpp particle.h particle.cpp vector.h vector.cpp collision.cpp  -lglut -lGLU -lGL
clean:
	rm graphics *.o