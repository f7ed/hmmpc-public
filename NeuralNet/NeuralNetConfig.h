#pragma once
#include "NeuralNet/LayerConfig.h"
#include "NeuralNet/FCConfig.h"
#include "NeuralNet/ReLUConfig.h"
#include <vector>
#include <iostream>
using namespace std;
namespace hmmpc
{

extern size_t INPUT_SIZE;
extern size_t LAST_LAYER_SIZE;
extern size_t NUM_LAYERS;
extern size_t MINI_BATCH_SIZE;

class NeuralNetConfig
{
public:
    size_t numIterations = 0;
    size_t numLayers = 0;
    vector<LayerConfig*> layerConf;
    
    NeuralNetConfig(size_t _numIterations):numIterations(_numIterations){}

    void addLayer(LayerConfig *fcl){layerConf.push_back(fcl);}

};

    
}