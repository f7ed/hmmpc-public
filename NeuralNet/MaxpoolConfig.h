#pragma once
#include "NeuralNet/LayerConfig.h"
#include "NeuralNet/globals.h"

using namespace std;
namespace hmmpc
{

class MaxpoolConfig: public LayerConfig
{
public:
    size_t imageHeight = 0;
    size_t imageWidth = 0;
    size_t features = 0;
    size_t poolSize = 0;
    size_t stride = 0;
    size_t batchSize = 0;

    MaxpoolConfig(size_t _imageHeight, size_t _imageWidth, size_t _features, 
				  size_t _poolSize, size_t _stride, size_t _batchSize)
	:imageHeight(_imageHeight),
	 imageWidth(_imageWidth),
	 features(_features),
	 poolSize(_poolSize),
	 stride(_stride),
	 batchSize(_batchSize),
	 LayerConfig("Maxpool")
	{
		assert((imageWidth - poolSize)%stride == 0 && "Maxpool layer parameters incorrect");
		assert((imageHeight - poolSize)%stride == 0 && "Maxpool layer parameters incorrect");
	};
};
}