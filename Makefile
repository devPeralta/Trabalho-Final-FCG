EXECUTABLE = ./bin/Linux/main

$(EXECUTABLE): src/main.cpp src/glad.c src/textrendering.cpp src/collisions.cpp include/matrices.h include/utils.h include/dejavufont.h
	mkdir -p bin/Linux
	g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o $(EXECUTABLE) src/main.cpp src/glad.c src/textrendering.cpp src/collisions.cpp ./lib-linux/libglfw3.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor

.PHONY: clean run exec
clean:
	rm -f $(EXECUTABLE)

run: $(EXECUTABLE)
	cd bin/Linux && ./main

# Target 'exec': Compila se necessário (depende de $(EXECUTABLE)) e executa.
# Passa a variável $(ARG) para o executável.
exec: $(EXECUTABLE)
	@echo "Executando: $(EXECUTABLE) $(ARG)"
	$(EXECUTABLE) $(ARG)
