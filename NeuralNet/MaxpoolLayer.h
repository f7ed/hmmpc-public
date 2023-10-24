#pragma once
#include "NeuralNet/MaxpoolConfig.h"
#include "NeuralNet/globals.h"
#include "NeuralNet/Layer.h"

using namespace std;

namespace hmmpc
{

class MaxpoolLayer: public Layer
{

private:
    MaxpoolConfig conf;
    sfixMatrix activations;
    sfixMatrix deltas;
    sintMatrix maxPrime;

public:
    MaxpoolLayer(MaxpoolConfig* conf, int _layerNum);

    void printLayer() override;
    void forward(const sfixMatrix &inputActivations) override;
    void forwardOnly(const sfixMatrix &inputActivations)override;
    void computeDelta(sfixMatrix &prevDelta)override;
    void updateEquations(const sfixMatrix &prevActivations)override;

    sfixMatrix& getActivation(){sfixMatrix &ref = activations; return ref;}
    sfixMatrix& getDelta(){sfixMatrix &ref = deltas; return ref;}
};
}