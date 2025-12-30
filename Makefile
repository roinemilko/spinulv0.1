D_DIR = dependencies/imgui
RAYLIB_DIR = dependencies/raylib/src
BRIDGE_DIR = dependencies/rlImGui
INCLUDES = -I$(D_DIR) -I$(D_DIR)/backends -I$(RAYLIB_DIR) -I$(BRIDGE_DIR)
RAYLIB_PCK = lib/libraylib.a

SOURCES = main.cpp utils.cpp physics.cpp \
	$(D_DIR)/imgui.cpp \
	$(D_DIR)/imgui_draw.cpp \
	$(D_DIR)/imgui_widgets.cpp \
	$(D_DIR)/imgui_tables.cpp \
	$(BRIDGE_DIR)/rlImGui.cpp

OBJECTS = $(SOURCES:.cpp=.o)
LIBS = -lglfw -lGL -lGLEW -pthread $(RAYLIB_PCK)

all: main run

main: $(OBJECTS)
	g++ $(OBJECTS) -o main $(LIBS)

%.o: %.cpp
	g++ -c $< -o $@ $(INCLUDES)

clean:
	rm -f $(OBJECTS) main

run:
	./main
