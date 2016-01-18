# C++ Compiler
CC=g++ -std=c++14

# Using Debug flags
CFLAGS=-Wall -O3

# Using for clearer error finding
BONUS=2>&1 | grep -E --color=always 'error|warning|$$'

# For OpenCV support
CFLAGS+=`pkg-config --cflags opencv`
LDFLAGS+=`pkg-config --libs opencv` -L/usr/local/cuda-7.5/lib64

# Paths
SRC=src
OBJ=obj
SRC_FILES = $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/*/*.cpp)
DEPD = $(wildcard $(SRC)/*.h) $(wildcard $(SRC)/*/*.h)
BIN=bin
DATA=data/mp4
INC=-I/usr/local/cuda-7.5/extras/CUPTI/include -Isrc -I/usr/local/cuda-7.5/targets/x86_64-linux/include/ 

# All files
OBJ_FILES := $(addprefix obj/,$(notdir $(SRC_FILES:.cpp=.o)))

# Used for debug
cccyan=$(shell echo "\033[0;36m")
ccend=$(shell echo "\033[0m")

all: VideoStitch CameraCalibrator

BUILD_PRINT = \e[1;34mBuilding $<\e[0m

$(OBJ)/%.o: src/%.cpp
	@mkdir -p $(OBJ)
	@echo "$(cccyan)[Run OBJ $@ compile]$(ccend)"
	$(CC) $(CFLAGS) $(INC) -c -o $@ $< 

$(OBJ)/%.o: src/*/%.cpp
	@mkdir -p $(OBJ)
	@echo "$(cccyan)[Run OBJ $@ compile]$(ccend)"
	$(CC) $(CFLAGS) $(INC) -c -o $@ $< 

VideoStitch: $(OBJ_FILES)
	@echo "$(cccyan)[Run Link compile]$(ccend)"
	$(CC) $? -o $(BIN)/$@ $(LDFLAGS)

CameraCalibrator:
	@echo "$(cccyan)[Ganerate camera calibrator]$(ccend)"
	$(CC) -o $(BIN)/$@ tools/calibration/CameraCalibrator.cpp $(LDFLAGS)

run:
	$(BIN)/VideoStitch --input data/Cut15/inputVideo.txt --calibration data/Cut15/Calibration.txt --pto data/Cut15/15.pto --duration 10 --output StitchResult.avi

calibrator:
	$(BIN)/CameraCalibrator data/CalibrationImages2/input_config.xml

clean:
	-rm -r $(BIN)/VideoStitch
	-rm -rf obj/*.o

CC_FLAGS += -MMD
-include $(OBJFILES:.o=.d)
