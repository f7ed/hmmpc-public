#pragma once

#include "NeuralNet/NeuralNetConfig.h"
#include "NeuralNet/globals.h"
#include "NeuralNet/Layer.h"

namespace hmmpc
{

class NeuralNetwork
{
public: 
    sfixMatrix inputData;
    sfixMatrix outputData;
    vector<Layer*> layers;

    NeuralNetwork(NeuralNetConfig*config);
    ~NeuralNetwork();

    void forward();
    void forwardOnly();//for inference
    void backward();
    void computeDelta();
    void updateEquations();
    void predict(sintMatrix &maxIndex);
    void getAccuracy(sintMatrix &maxIndex, vector<size_t> &counter);
};

class NeuralNetworkClear
{
public: 
    RowMatrixXd inputData;
    RowMatrixXd outputData;
    vector<LayerClear*> layers;

    NeuralNetworkClear(NeuralNetConfig*config);
    ~NeuralNetworkClear();

    void forward();
    void backward();
    void computeDelta();
    void updateEquations();
    void predict(RowMatrixXd &maxIndex);
    void getAccuracy(RowMatrixXd &maxIndex, vector<size_t> &counter);
};
}