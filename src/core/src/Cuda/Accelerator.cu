#include <cstdio>
#include <cstdlib>
#include "SyncedMemory.h"
#include "Accelerator.h"

unsigned char* cudaProjMapX;
unsigned char* cudaProjMapY;
unsigned char* cudaFinalBlendingMap;
unsigned char* cudaFrames = nullptr;
unsigned char* cudaOut = nullptr;
unsigned char* cudaSmallOut = nullptr;
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
										unsigned char* mapX, unsigned char* mapY, unsigned char* bMap, unsigned char* frames, unsigned char* saliency, unsigned char* out, 
										int dW, int dH, unsigned char* smallOut) {
	const int y = blockIdx.y * blockDim.y + threadIdx.y;
	const int x = blockIdx.x * blockDim.x + threadIdx.x;

	if (x < oW and y < oH) {
		int curO = (y * oW + x);

		// Bilinear 
		int x0 = x / gridSize;
		int y0 = y / gridSize;
		int lt = y0 * sW + x0;

		int dx = x * dW / oW;
		int dy = y * dH / oH;
		int curD = (dy * dW + dx);

		int oriR = *(smallOut + curD * oChannels);
		int oriG = *(smallOut + curD * oChannels + 1);
		int oriB = *(smallOut + curD * oChannels + 2);

		if (saliency[lt] == 0 && saliency[lt+1] == 0 && saliency[lt+sW] == 0 && saliency[lt+sW+1] == 0) {
			*(out + curO * oChannels) = oriR;
			*(out + curO * oChannels + 1) = oriG;
			*(out + curO * oChannels + 2) = oriB;
			
			return;
		}

		float s = (x % gridSize) / (float)gridSize;
		float t = (y % gridSize) / (float)gridSize;

		int sVal = (1-s) * ( (1-t) * saliency[lt] + t * saliency[lt+sW] ) + s * ( (1-t) * saliency[lt+1] + t * saliency[lt+sW+1] );

		int curO3 = curO * oChannels;
		int elementCount = oW * oH;
		int frameElementCount = vW * vH * vChannels;

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

		if (sVal <= BLEND_THRESHOLD) {
			float blendRatio = sVal / 255.f;
			// Blending
	    	tmp = (int)(blendRatio * *(out + curO3) + (1-blendRatio) * oriR);
	    	*(out + curO3) = tmp > 255 ? 255 : tmp;
	    	tmp = (int)(blendRatio * *(out + curO3 + 1) + (1-blendRatio) * oriG);
	    	*(out + curO3 + 1) = tmp > 255 ? 255 : tmp;
	    	tmp = (int)(blendRatio * *(out + curO3 + 2) + (1-blendRatio) * oriB);
	    	*(out + curO3 + 2) = tmp > 255 ? 255 : tmp;
	    }
	}
}

__global__ void runRenderSmallSizePanoCuda(int viewCount, int vW, int vH, int vChannels, int dW, int dH, int dChannels, int oW, int oH,
										unsigned char* mapX, unsigned char* mapY, unsigned char* bMap, unsigned char* frames, unsigned char* out) {
	const int y = blockIdx.y * blockDim.y + threadIdx.y;
	const int x = blockIdx.x * blockDim.x + threadIdx.x;

	if (x < dW and y < dH) {
		int curD = (y * dW + x);
		int curO = ((y * oH / dH) * oW + (x * oW / dW));

		int curD3 = curD * dChannels;
		int elementCount = oW * oH ;
		int frameElementCount = vW * vH * vChannels;

		*(out + curD3) = 0;
		*(out + curD3 + 1) = 0;
		*(out + curD3 + 2) = 0;

		unsigned int* mX = (unsigned int*) mapX;
		unsigned int* mY = (unsigned int*) mapY;
		float* b = (float*) bMap;
		int tmp;

		for (int v=0; v<viewCount; v++) {
			int pX = *(mX + elementCount * v + curO);
			int pY = *(mY + elementCount * v + curO);
			if ( !(pY < 0 || pX < 0 || pX >= vW || pY >= vH) ) {
				int pOffset = (pY * vW + pX) * vChannels;

				tmp = *(out + curD3) + *(frames + frameElementCount * v + pOffset) * *(b + elementCount * v + curO);
				*(out + curD3) = tmp > 255 ? 255 : tmp;
				tmp = *(out + curD3 + 1) + *(frames + frameElementCount * v + pOffset + 1) * *(b + elementCount * v + curO + 1);
				*(out + curD3 + 1) = tmp > 255 ? 255 : tmp;
				tmp = *(out + curD3 + 2) + *(frames + frameElementCount * v + pOffset + 2) * *(b + elementCount * v + curO + 2);
				*(out + curD3 + 2) = tmp > 255 ? 255 : tmp;
			}
		}		
	}
}

void renderSaliencyAreaCuda(int viewCount, int vW, int vH, int vChannels, int sW, int sH, int gridSize, int renderDiameter, int cX, int cY, int oW, int oH, int oChannels, 
							unsigned char* blendSaliency, unsigned char* out, int dW, int dH) {
	dim3 gdim(CeilDiv(oW,32), CeilDiv(oH,16)), bdim(32,16);

	if (cudaOut == nullptr)	
		cudaMalloc(&cudaOut, oW * oH * oChannels * sizeof(unsigned char));
	if (cudaSaliency == nullptr)	
		cudaMalloc(&cudaSaliency, sW * sH * sizeof(unsigned char));

	// To copy the small size canvas
	//cudaMemcpy(cudaOut, out, oW * oH * oChannels * sizeof(unsigned char), cudaMemcpyHostToDevice);
	cudaMemcpy(cudaSaliency, blendSaliency, sW * sH * sizeof(unsigned char), cudaMemcpyHostToDevice);
	runRenderSaliencyAreaCuda<<<gdim, bdim>>>(viewCount, vW, vH, vChannels, sW, sH, gridSize, renderDiameter, cX, cY, oW, oH, oChannels, cudaProjMapX, cudaProjMapY, cudaFinalBlendingMap, cudaFrames, cudaSaliency, cudaOut, dW, dH, cudaSmallOut);
	
	CHECK

	cudaMemcpy(out, cudaOut, oW * oH * oChannels * sizeof(unsigned char), cudaMemcpyDeviceToHost);
}

void renderSmallSizePanoCuda(int viewCount, int vW, int vH, int vChannels, int dW, int dH, int dChannels, int oW, int oH, unsigned char* downOut) {
	dim3 gdim(CeilDiv(dW,32), CeilDiv(dH,16)), bdim(32,16);

	if (cudaSmallOut == nullptr)	
		cudaMalloc(&cudaSmallOut, dW * dH * dChannels * sizeof(unsigned char));
	cudaMemcpy(cudaSmallOut, downOut, dW * dH * dChannels * sizeof(unsigned char), cudaMemcpyHostToDevice);
	runRenderSmallSizePanoCuda<<<gdim, bdim>>>(viewCount, vW, vH, vChannels, dW, dH, dChannels, oW, oH, cudaProjMapX, cudaProjMapY, cudaFinalBlendingMap, cudaFrames, cudaSmallOut);
	
	CHECK

	cudaMemcpy(downOut, cudaSmallOut, dW * dH * dChannels * sizeof(unsigned char), cudaMemcpyDeviceToHost);
}