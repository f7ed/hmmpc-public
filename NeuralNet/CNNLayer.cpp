#pragma once
#include "NeuralNet/CNNLayer.h"
#include "Types/wrapper.h"
#include "NeuralNet/tools.h"
using namespace std;

namespace hmmpc
{

CNNLayer::CNNLayer(CNNConfig* conf, int _layerNum)
:Layer(_layerNum),
conf(conf->imageHeight, conf->imageWidth, conf->inputFeatures, 
	  conf->filters, conf->filterSize, conf->stride, 
	  conf->padding, conf->batchSize),
weights(conf->filterSize * conf->filterSize * conf->inputFeatures, conf->filters),
biases(1, conf->filters),
activations(conf->batchSize, conf->filters * 
            (((conf->imageWidth - conf->filterSize + 2*conf->padding)/conf->stride) + 1) * 
 		    (((conf->imageHeight - conf->filterSize + 2*conf->padding)/conf->stride) + 1)),
deltas(conf->batchSize, conf->filters * 
            (((conf->imageWidth - conf->filterSize + 2*conf->padding)/conf->stride) + 1) * 
 		    (((conf->imageHeight - conf->filterSize + 2*conf->padding)/conf->stride) + 1))
{
    initialize();
}

void CNNLayer::initialize()
{

}

void CNNLayer::printLayer()
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") CNN Layer\t\t  " << conf.imageHeight << " x " << conf.imageWidth 
		 << " x " << conf.inputFeatures << endl << "\t\t\t  " 
		 << conf.filterSize << " x " << conf.filterSize << "  \t(Filter Size)" << endl << "\t\t\t  " 
		 << conf.stride << " , " << conf.padding << " \t(Stride, padding)" << endl << "\t\t\t  " 
		 << conf.batchSize << "\t\t(Batch Size)" << endl << "\t\t\t  " 
		 << (((conf.imageWidth - conf.filterSize + 2*conf.padding)/conf.stride) + 1) << " x " 
		 << (((conf.imageHeight - conf.filterSize + 2*conf.padding)/conf.stride) + 1) << " x " 
		 << conf.filters << " \t(Output)" << endl;
}

// Each row stores the feature1, feature2, ..., of the image.
// TODO: There is some bug when the batch_size > 1. 
void CNNLayer::forward(const sfixMatrix &inputActivations)
{
	log_print("CNN.forward");
	
    size_t &B 	= conf.batchSize;
	size_t &iw 	= conf.imageWidth;
	size_t &ih 	= conf.imageHeight;
	size_t &f 	= conf.filterSize;
	size_t &Din 	= conf.inputFeatures;
	size_t &Dout = conf.filters;
	size_t &P 	= conf.padding;
	size_t &S 	= conf.stride;
	size_t ow 	= (((iw-f+2*P)/S)+1);
	size_t oh	= (((ih-f+2*P)/S)+1);

    gfpMatrix paddedInput(B, (iw+2*P)*(ih+2*P)*Din);
    zeroPad(inputActivations.share(), paddedInput, iw, ih, P, Din, B);

    sfixMatrix extendInput(B*oh*ow, f*f*Din);
    convolExtend(paddedInput, extendInput.share(), iw, ih, ow, oh, Din, S, f, B);

	// activations(B, oh*ow*Dout)
    if (FUNCTION_TIME)
		cout << "funcConvMatMul: " << funcTime(funcConvMatMul, extendInput, weights, biases, activations, B, oh, ow, Dout) << endl;
	else
		funcConvMatMul(extendInput, weights, biases, activations, B, oh, ow, Dout);
}

void CNNLayer::forwardOnly(const sfixMatrix &inputActivations)
{
	forward(inputActivations);
}

void CNNLayer::computeDelta(sfixMatrix &prevDelta)
{

}

void CNNLayer::updateEquations(const sfixMatrix &preactivations)
{

}
}
