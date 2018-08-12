all:  my2D
my2D: my.cpp glad.c
	g++ -O2 -o my2D my.cpp glad.c  -L/usr/local/lib -lGLEW -lGLU -lGL -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lglfw -lrt -lm -ldl -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -lXext -lXrender -lXfixes -lX11 -lpthread -lxcb -lXau -lXdmcp  
clean:
	rm my2D
