#pragma once

#include "NeuralNet/ReLUConfig.h"
#include "NeuralNet/Layer.h"
#include "NeuralNet/globals.h"

namespace hmmpc 
{

class ReLULayer:public Layer
{
private:   
    ReLUConfig conf;
    sfixMatrix activations;
    sfixMatrix deltas;
    sintMatrix reluPrime;

public:
    ReLULayer(ReLUConfig* conf, int _layerNum);

    void printLayer() override;
    void forward(const sfixMatrix&inputActivations)override;
    void computeDelta(sfixMatrix &prevDelta)override;
    void updateEquations(const sfixMatrix& prevActivations)override;

    void forwardOnly(const sfixMatrix&inputActivations);// without calculating reluPrime

    sfixMatrix& getActivation(){sfixMatrix &ref = activations; return ref;}
    sfixMatrix& getDelta(){sfixMatrix &ref = deltas; return ref;}
};

class ReLULayerClear:public LayerClear
{
private:
    ReLUConfig conf;
    RowMatrixXd activations;
    RowMatrixXd deltas;
    RowMatrixXd reluPrime;

public:
    ReLULayerClear(ReLUConfig* conf, int _layerNum);
    void printLayer() override;
    void forward(const RowMatrixXd&inputActivations)override;
    void computeDelta(RowMatrixXd &prevDelta)override;
    void updateEquations(const RowMatrixXd& prevActivations)override;

    RowMatrixXd& getActivation(){RowMatrixXd &ref = activations; return ref;}
    RowMatrixXd& getDelta(){RowMatrixXd &ref = deltas; return ref;}
};
} // namespace hmmpc
