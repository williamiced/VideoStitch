# C++ Compiler
CC=g++ -std=c++14

# Using Debug flags
CFLAGS=-Wall -O3

# Using for clearer error finding
BONUS=2>&1 | grep -E --color=always 'error|warning|$$'

# For OpenCV support
CFLAGS+=`pkg-config --cflags opencv` -fopenmp
LDFLAGS+=`pkg-config --libs opencv` -L/usr/local/cuda-7.5/lib64 -lboost_system -lboost_timer -lgomp

# Paths
SRC=src/core/src
OBJ=obj
SRC_FILES = $(wildcard $(SRC)/*.cpp)
DEPD = $(wildcard $(SRC)/header/.h)
BIN=bin
DATA=data/mp4
INC=-I/usr/local/cuda-7.5/extras/CUPTI/include -I$(SRC) -I/usr/local/cuda-7.5/targets/x86_64-linux/include/ 

# All files
OBJ_FILES := $(addprefix obj/,$(notdir $(SRC_FILES:.cpp=.o)))

# Used for debug
cccyan=$(shell echo "\033[0;36m")
ccend=$(shell echo "\033[0m")

all: VideoStitch

tools: VideoStitch CameraCalibrator ImagesDumper

BUILD_PRINT = \e[1;34mBuilding $<\e[0m

makeObj: $(OBJ_FILES)
	@echo "$(cccyan)[Obj files generated]$(ccend)"

$(OBJ)/%.o: $(SRC)/%.cpp $(SRC)/header/%.h
	@mkdir -p $(OBJ)
	@echo "$(cccyan)[Run OBJ $@ compile]$(ccend)"
	$(CC) $(CFLAGS) $(INC) -c -o $@ $< 

VideoStitch: $(OBJ_FILES) 
	@echo "$(cccyan)[Run Link compile]$(ccend)"
	$(CC) $? -I$(SRC) src/cmd/main.cpp -o $(BIN)/$@ $(LDFLAGS)

CameraCalibrator:
	@echo "$(cccyan)[Ganerate camera calibrator]$(ccend)"
	$(CC) -o $(BIN)/$@ tools/calibration/CameraCalibrator.cpp $(LDFLAGS)

ImagesDumper:
	@echo "$(cccyan)[Ganerate images dumper]$(ccend)"
	$(CC) obj/Usage.o -o $(BIN)/$@ tools/imagesDumper/ImagesDumper.cpp $(LDFLAGS)

run:
	#$(BIN)/VideoStitch --input data/gopro/inputVideo.txt --calibration data/MultiCalibration/calibrationResult.txt --pto data/Cut15/15.pto --duration 100 --output StitchResult.avi
	#$(BIN)/VideoStitch --input data/MultiCalibration/inputVideo.txt --calibration data/MultiCalibration/calibrationResult.txt --pto data/Cut15/15.pto --duration 100 --output StitchResult.avi
	$(BIN)/VideoStitch --input data/Cut15/inputVideo.txt --calibration data/Cut15/Calibration.txt --pto data/Cut15/15.pto --duration 200 --output StitchResult.avi

calibrator:
	$(BIN)/CameraCalibrator data/CalibrationImages2/input_config.xml

dumper:
	$(BIN)/ImagesDumper data/Library20160216/inputVideo2.txt data/Library20160216/pattern.png 0 1 1 data/Library20160216/dump4
	#$(BIN)/ImagesDumper data/Library20160216/inputVideo2.txt data/Library20160216/pattern.png 0 1 1 data/Library20160216/dump3

clean:
	-rm -r $(BIN)/VideoStitch
	-rm -rf obj/*.o

CC_FLAGS += -MMD
-include $(OBJFILES:.o=.d)
