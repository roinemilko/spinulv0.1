SRC_DIR = src
IMGUI = dependencies/imgui
RAYLIB = dependencies/raylib/src
BRIDGE = dependencies/rlImGui
HEADERS = include

BUILT_LIBS = $(wildcard lib/*.a)

BUILD_DIR = build


INCLUDES = -I$(HEADERS) -I$(SRC_DIR) -I$(IMGUI) -I$(RAYLIB) -I$(BRIDGE)

SOURCES = $(wildcard $(SRC_DIR)/*.cpp) \
        $(IMGUI)/imgui.cpp \
        $(IMGUI)/imgui_draw.cpp \
        $(IMGUI)/imgui_widgets.cpp \
        $(IMGUI)/imgui_tables.cpp \
        $(BRIDGE)/rlImGui.cpp

OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.cpp=.o))
LIBS = $(BUILT_LIBS) -lglfw -lGLEW -lGL -lm -ldl

TARGET = $(BUILD_DIR)/main

all: $(TARGET) run

$(TARGET): $(OBJECTS)
	g++ $(OBJECTS) -o $(TARGET) $(LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ -c $< -o $@ $(INCLUDES)

clean:
	rm -f $(OBJECTS) main *.o src/*.o

run:
	./$(BUILD_DIR)/main
