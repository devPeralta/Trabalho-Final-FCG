EXECUTABLE = ./bin/Linux/main
$(EXECUTABLE): src/main.cpp src/glad.c src/textrendering.cpp src/maze.cpp src/collisions.cpp include/matrices.h include/utils.h include/dejavufont.h
	mkdir -p bin/Linux
	g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o $(EXECUTABLE) src/main.cpp src/glad.c src/textrendering.cpp src/collisions.cpp src/maze.cpp src/tiny_obj_loader.cpp ./lib-linux/libglfw3.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor  
#                                                                                                                                                                          ^^^^^^^^^^^^^ ADICIONE AQUI

.PHONY: clean run exec

clean:
	rm -f $(EXECUTABLE)

run: $(EXECUTABLE)
	cd bin/Linux && ./main

exec: $(EXECUTABLE)
	@echo "Executando: $(EXECUTABLE) $(ARG)"
	$(EXECUTABLE) $(ARG)
