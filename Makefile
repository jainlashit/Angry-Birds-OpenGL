sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGLEW -lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl -lftgl -lSOIL -I/usr/local/include -I/usr/include/freetype2 -L/usr/local/lib

clean:
	rm sample2D
