CXX=g++
CXXFLAGS= -c `sdl2-config --cflags` -std=c++11
INCLUDES= -Iinclude -I/opt/homebrew/opt/glm/include
LFLAGS= `sdl2-config --libs` -lGLEW -framework OpenGL
BUILDDIR=build
SRCDIR=src
SRC=$(wildcard $(SRCDIR)/*.cpp)
_OBJ=$(SRC:.cpp=.o)
OBJ=$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(_OBJ))
TARGET=prac1
TARGETPATH=$(BUILDDIR)/$(TARGET)

build: $(OBJ) $(TARGET)

run:
	cd $(BUILDDIR); ./$(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGETPATH) $(LFLAGS)


$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(TARGETPATH)
	rm -f $(OBJ)

