SRC_DIR = src
IMGUI = dependencies/imgui
RAYLIB = dependencies/raylib/src
RAYLIB_PCK = lib/libraylib.a
BRIDGE = dependencies/rlImGui
HEADERS = include

BUILD_DIR = build


INCLUDES = -I$(HEADERS) -I$(SRC_DIR) -I$(IMGUI) -I$(RAYLIB) -I$(BRIDGE)

SOURCES = $(wildcard $(SRC_DIR)/*.cpp) \
        $(IMGUI)/imgui.cpp \
        $(IMGUI)/imgui_draw.cpp \
        $(IMGUI)/imgui_widgets.cpp \
        $(IMGUI)/imgui_tables.cpp \
        $(BRIDGE)/rlImGui.cpp

OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.cpp=.o))
LIBS = -lglfw -lGLEW -lGL -pthread $(RAYLIB_PCK)

all: main run

main: $(OBJECTS)
	g++ $(OBJECTS) -o main $(LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ -c $< -o $@ $(INCLUDES)

clean:
	rm -f $(OBJECTS) main *.o src/*.o

run:
	./main
