#include <cstdio>
#include <cstdlib>
#include "SyncedMemory.h"
#include "Accelerator.h"

unsigned char* cudaProjMapX;
unsigned char* cudaProjMapY;
unsigned char* cudaFinalBlendingMap;
unsigned char* cudaFrames = nullptr;
unsigned char* cudaOut = nullptr;
unsigned char* cudaSaliency = nullptr;

__device__ __host__ int CeilDiv(int a, int b) { return (a-1)/b + 1; }
__device__ __host__ int CeilAlign(int a, int b) { return CeilDiv(a, b) * b; }

void helloWorld() {
	printf("Hello World\n");
}

void registerRefMap(vector<unsigned char*> mapXArr, vector<unsigned char*> mapYArr, vector<unsigned char*> blendMapArr, int viewCount, int W, int H) {
	int elementCount = W * H;

	cudaMalloc(&cudaProjMapX, elementCount * viewCount * sizeof(unsigned int));
	cudaMalloc(&cudaProjMapY, elementCount * viewCount * sizeof(unsigned int));
	cudaMalloc(&cudaFinalBlendingMap, elementCount * viewCount * sizeof(float));

	for (int v=0; v<viewCount; v++) {
		cudaMemcpy(cudaProjMapX + elementCount * v * sizeof(unsigned int) / sizeof(unsigned char), mapXArr[v], elementCount * sizeof(unsigned int), cudaMemcpyHostToDevice);
		cudaMemcpy(cudaProjMapY + elementCount * v * sizeof(unsigned int) / sizeof(unsigned char), mapYArr[v], elementCount * sizeof(unsigned int), cudaMemcpyHostToDevice);
		cudaMemcpy(cudaFinalBlendingMap + elementCount * v * sizeof(float) / sizeof(unsigned char), blendMapArr[v], elementCount * sizeof(float), cudaMemcpyHostToDevice);	
	}	
}

void copyFrames(vector<unsigned char*> frames, int viewCount, int channel, int fW, int fH) {
	int elementCount = fW * fH;
	if (cudaFrames == nullptr)
		cudaMalloc(&cudaFrames, elementCount * viewCount * channel * sizeof(unsigned char));

	for (int v=0; v<viewCount; v++) 
		cudaMemcpy(cudaFrames + elementCount * channel * v, frames[v], elementCount * channel * sizeof(unsigned char), cudaMemcpyHostToDevice);	
}

__global__ void runRenderSaliencyAreaCuda(int viewCount, int vW, int vH, int vChannels, int sW, int sH, int gridSize, int renderDiameter, int cX, int cY, int oW, int oH, int oChannels, 
										unsigned char* mapX, unsigned char* mapY, unsigned char* bMap, unsigned char* frames, unsigned char* saliency, unsigned char* out) {
	const int y = blockIdx.y * blockDim.y + threadIdx.y;
	const int x = blockIdx.x * blockDim.x + threadIdx.x;
	const int sX = x / gridSize;
	const int sY = y / gridSize;
	const int curS = sW * sY + sX;

	if (x < oW and y < oH and saliency[curS] > 0) {
		// For diameter
		int tmpX = cX - sX;
		tmpX = tmpX > 0 ? tmpX : tmpX * (-1);
		tmpX = tmpX < sW/2 ? tmpX : sW-tmpX;
		int tmpY = cY - sY;
		
		if ( sqrtf(tmpX*tmpX + tmpY*tmpY)*gridSize > renderDiameter)
			return;
		
		int curO = (y * oW + x);
		int curO3 = curO * oChannels;
		int elementCount = oW * oH;
		int frameElementCount = vW * vH * vChannels;

		//outImg.at<Vec3b>(y0, x0) = Vec3b(0, 0, 0);
		int oriR = *(out + curO3);
		int oriG = *(out + curO3 + 1);
		int oriB = *(out + curO3 + 2);

		*(out + curO3) = 0;
		*(out + curO3 + 1) = 0;
		*(out + curO3 + 2) = 0;

		unsigned int* mX = (unsigned int*) mapX;
		unsigned int* mY = (unsigned int*) mapY;
		float* b = (float*) bMap;
		int tmp;

		for (int v=0; v<viewCount; v++) {
			int pX = *(mX + elementCount * v + curO);
			int pY = *(mY + elementCount * v + curO);
			if ( !(pY < 0 || pX < 0 || pX >= vW || pY >= vH) ) {
				int pOffset = (pY * vW + pX) * vChannels;
				tmp = *(out + curO3) + *(frames + frameElementCount * v + pOffset) * *(b + elementCount * v + curO);
				*(out + curO3) = tmp > 255 ? 255 : tmp;

				tmp = *(out + curO3 + 1) + *(frames + frameElementCount * v + pOffset + 1) * *(b + elementCount * v + curO + 1);
				*(out + curO3 + 1) = tmp > 255 ? 255 : tmp;

				tmp = *(out + curO3 + 2) + *(frames + frameElementCount * v + pOffset + 2) * *(b + elementCount * v + curO + 2);
				*(out + curO3 + 2) = tmp > 255 ? 255 : tmp;
			}
		}

		// Blending
		int cCanvasX = (cX-0.5) * gridSize;
		int cCanvasY = (cY-0.5) * gridSize;
		float diffX = cCanvasX - x;
		float diffY = cCanvasY - y;
		float dist = sqrtf(diffX * diffX + diffY * diffY);
		float xr = (dist - renderDiameter)/(0 - renderDiameter); 
    	xr = xr > 1.0 ? 1.0 : xr;
    	xr = xr < 0.0 ? 0.0 : xr;
    	float blendRatio = xr*xr*(3 - 2*xr);
    	tmp = (int)(blendRatio * *(out + curO3) + (1-blendRatio) * oriR);
    	*(out + curO3) = tmp > 255 ? 255 : tmp;
    	tmp = (int)(blendRatio * *(out + curO3 + 1) + (1-blendRatio) * oriG);
    	*(out + curO3 + 1) = tmp > 255 ? 255 : tmp;
    	tmp = (int)(blendRatio * *(out + curO3 + 2) + (1-blendRatio) * oriB);
    	*(out + curO3 + 2) = tmp > 255 ? 255 : tmp;
	}
}

void renderSaliencyAreaCuda(int viewCount, int vW, int vH, int vChannels, int sW, int sH, int gridSize, int renderDiameter, int cX, int cY, int oW, int oH, int oChannels, 
							unsigned char* saliencyInfo, unsigned char* out) {
	dim3 gdim(CeilDiv(oW,32), CeilDiv(oH,16)), bdim(32,16);

	if (cudaOut == nullptr)	
		cudaMalloc(&cudaOut, oW * oH * oChannels * sizeof(unsigned char));
	if (cudaSaliency == nullptr)	
		cudaMalloc(&cudaSaliency, sW * sH * sizeof(unsigned char));

	// To copy the small size canvas
	cudaMemcpy(cudaOut, out, oW * oH * oChannels * sizeof(unsigned char), cudaMemcpyHostToDevice);
	cudaMemcpy(cudaSaliency, saliencyInfo, sW * sH * sizeof(unsigned char), cudaMemcpyHostToDevice);

	runRenderSaliencyAreaCuda<<<gdim, bdim>>>(viewCount, vW, vH, vChannels, sW, sH, gridSize, renderDiameter, cX, cY, oW, oH, oChannels, cudaProjMapX, cudaProjMapY, cudaFinalBlendingMap, cudaFrames, cudaSaliency, cudaOut);
	
	CHECK

	cudaMemcpy(out, cudaOut, oW * oH * oChannels * sizeof(unsigned char), cudaMemcpyDeviceToHost);
}