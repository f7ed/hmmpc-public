#pragma once
#include "NeuralNet/MaxpoolLayer.h"
#include "Types/wrapper.h"
#include "NeuralNet/tools.h"

using namespace std;

namespace hmmpc
{
MaxpoolLayer::MaxpoolLayer(MaxpoolConfig* conf, int _layerNum)
:Layer(_layerNum),
conf(conf->imageHeight, conf->imageWidth, conf->features, 
	  conf->poolSize, conf->stride, conf->batchSize),
activations(conf->batchSize, conf->features*
            (((conf->imageWidth - conf->poolSize)/conf->stride) + 1) * 
 		    (((conf->imageHeight - conf->poolSize)/conf->stride) + 1)),
deltas(conf->batchSize, conf->features *
            (((conf->imageWidth - conf->poolSize)/conf->stride) + 1) * 
 		    (((conf->imageHeight - conf->poolSize)/conf->stride) + 1)),
maxPrime(conf->batchSize, conf->features * conf->poolSize * conf->poolSize * 
            (((conf->imageWidth - conf->poolSize)/conf->stride) + 1) * 
 		    (((conf->imageHeight - conf->poolSize)/conf->stride) + 1))
{}

void MaxpoolLayer::printLayer()
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") Maxpool Layer\t  " << conf.imageHeight << " x " << conf.imageWidth 
		 << " x " << conf.features << endl << "\t\t\t  " 
		 << conf.poolSize << "  \t\t(Pooling Size)" << endl << "\t\t\t  " 
		 << conf.stride << " \t\t(Stride)" << endl << "\t\t\t  " 
		 << conf.batchSize << "\t\t(Batch Size)" << endl;
}

void MaxpoolLayer::forward(const sfixMatrix &inputActivations)
{
    log_print("Maxpool.forward");

    size_t B 	= conf.batchSize;
	size_t iw 	= conf.imageWidth;
	size_t ih 	= conf.imageHeight;
	size_t f 	= conf.poolSize;
	size_t Din 	= conf.features;
	size_t S 	= conf.stride;
	size_t ow 	= (((iw-f)/S)+1);
	size_t oh	= (((ih-f)/S)+1);

    sfixMatrix extendInput(B*ow*oh*Din, f*f);
    maxpoolExtend(inputActivations.share(), extendInput.share(), iw, ih, ow, oh, Din, S, f, B);

    if (FUNCTION_TIME)
		cout << "funcMaxpool: " << funcTime(funcMaxpool,extendInput, maxPrime, activations, B, Din, oh, ow, f) << endl;
	else
		funcMaxpool(extendInput, maxPrime, activations, B, Din, oh, ow, f);

}

void MaxpoolLayer::forwardOnly(const sfixMatrix &inputActivations)
{
    log_print("Maxpool.forward");

    size_t B 	= conf.batchSize;
	size_t iw 	= conf.imageWidth;
	size_t ih 	= conf.imageHeight;
	size_t f 	= conf.poolSize;
	size_t Din 	= conf.features;
	size_t S 	= conf.stride;
	size_t ow 	= (((iw-f)/S)+1);
	size_t oh	= (((ih-f)/S)+1);

    sfixMatrix extendInput(B*ow*oh*Din, f*f);
    maxpoolExtend(inputActivations.share(), extendInput.share(), iw, ih, ow, oh, Din, S, f, B);

    if (FUNCTION_TIME)
		cout << "funcOnlyMaxpool: " << funcTime(funcOnlyMaxpool, extendInput, activations, B, Din, oh, ow) << endl;
	else
        funcOnlyMaxpool(extendInput, activations, B, Din, oh, ow);
}

void MaxpoolLayer::computeDelta(sfixMatrix &prevDelta)
{

}

void MaxpoolLayer::updateEquations(const sfixMatrix &prevActivations)
{

}
}