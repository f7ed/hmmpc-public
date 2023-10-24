#pragma once
#include "NeuralNet/CNNConfig.h"
#include "NeuralNet/Layer.h"
#include "NeuralNet/globals.h"

using namespace std;

namespace hmmpc
{

class CNNLayer: public Layer
{
private:
    CNNConfig conf;
    sfixMatrix activations;
    sfixMatrix deltas;
    sfixMatrix weights;
    sfixMatrix biases;
public:

    //Constructor and initializer
	CNNLayer(CNNConfig* conf, int _layerNum);
	void initialize();

	//Functions
	void printLayer() override;
	void forward(const sfixMatrix& inputActivation) override;
    void forwardOnly(const sfixMatrix &inputActivations) override;
	void computeDelta(sfixMatrix& prevDelta) override;
	void updateEquations(const sfixMatrix& prevActivations) override;

    sfixMatrix& getActivation(){sfixMatrix &ref = activations; return ref;}
    sfixMatrix& getDelta(){sfixMatrix &ref = deltas; return ref;}
    sfixMatrix& getWeights(){sfixMatrix &ref = weights; return ref;}
    sfixMatrix& getBias(){sfixMatrix &ref = biases; return ref;}
};
}