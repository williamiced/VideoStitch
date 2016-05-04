#ifndef CUDA_ACCELERATOR_H
#define CUDA_ACCELERATOR_H

#include <vector>

using namespace std;

#define CHECK {\
	auto e = cudaDeviceSynchronize();\
	if (e != cudaSuccess) {\
		printf("At " __FILE__ ":%d, %s\n", __LINE__, cudaGetErrorString(e));\
		abort();\
	}\
}
	extern unsigned char* cudaProjMapX;
	extern unsigned char* cudaProjMapY;
	extern unsigned char* cudaFinalBlendingMap;
	extern unsigned char* cudaFrames;
	extern unsigned char* cudaOut;
	extern unsigned char* cudaSaliency;

	void helloWorld();
	void registerRefMap(vector<unsigned char*> mapXArr, vector<unsigned char*> mapYArr, vector<unsigned char*> blendMapArr, int viewCount, int W, int H);
	void copyFrames(vector<unsigned char*> frames, int viewCount, int channel, int fW, int fH);
	void renderSaliencyAreaCuda(int viewCount, int vW, int vH, int vChannels, int sW, int sH, int gridSize, int renderDiameter, int cX, int cY, int oW, int oH, int oChannels, 
								unsigned char* saliencyInfo, unsigned char* out);
#endif
