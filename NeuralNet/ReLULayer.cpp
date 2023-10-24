#pragma once

#include "NeuralNet/ReLULayer.h"
#include "NeuralNet/tools.h"
#include "Types/wrapper.h"
using namespace std;

namespace hmmpc
{

ReLULayer::ReLULayer(ReLUConfig *conf, int _layerNum)
:Layer(_layerNum),
conf(conf->inputDim, conf->batchSize),
activations(conf->batchSize, conf->inputDim),
deltas(conf->batchSize, conf->inputDim),
reluPrime(conf->batchSize, conf->inputDim)
{}

void ReLULayer::printLayer()
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") ReLU Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
}

void ReLULayer::forward(const sfixMatrix& inputActivations)
{
	log_print("ReLU.forward");
    // inputActivations.ReLU(reluPrime, activations);
	if (FUNCTION_TIME)
        cout << "funcReLU: "<< funcTime(funcReLU, inputActivations, reluPrime, activations) <<endl;
    else
        funcReLU(inputActivations, reluPrime, activations);

#ifdef DEBUG_NN
	cout<<"reluPrime: "<<reluPrime.reveal(conf.inputDim)<<endl;
	cout<<"activations: "<<activations.reveal(conf.inputDim)<<endl;
#endif
}

void ReLULayer::forwardOnly(const sfixMatrix&inputActivations)
{
	log_print("ReLU.forward");
	if (FUNCTION_TIME)
		cout<<"funcReLU: "<<funcTime(funcOnlyReLU, inputActivations, activations)<<endl;
	else
		funcOnlyReLU(inputActivations, activations);
}

void ReLULayer::computeDelta(sfixMatrix &prevDelta)
{
    // prevDelta.share() = deltas.share().array() * reluPrime.share().array();
    // prevDelta.reduce_degree();
	if (FUNCTION_TIME)
        cout << "funcCwiseMul: "<< funcTime(funcCwiseMul, deltas, reluPrime, prevDelta) <<endl;
    else
        funcCwiseMul(deltas, reluPrime, prevDelta);

#ifdef DEBUG_NN
	cout<<"reluP: "<<reluPrime.reveal(reluPrime.cols())<<endl;
	std::string str = "layer " + to_string(layerNum) + " delta: ";
	cout<<str<<prevDelta.reveal(prevDelta.cols())<<endl;
#endif
}

void ReLULayer::updateEquations(const sfixMatrix& prevActivations)
{
	log_print("ReLU.updateEquations");
}


// ReLULayerClear
ReLULayerClear::ReLULayerClear(ReLUConfig *conf, int _layerNum)
:LayerClear(_layerNum),
conf(conf->inputDim, conf->batchSize),
activations(conf->batchSize, conf->inputDim),
deltas(conf->batchSize, conf->inputDim),
reluPrime(conf->batchSize, conf->inputDim)
{}

void ReLULayerClear::printLayer()
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") ReLU Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
}

void ReLULayerClear::forward(const RowMatrixXd& inputActivations)
{
	log_print("ReLU.forward");
    
	RowMatrixXd constOne(inputActivations.rows(), inputActivations.cols());
	constOne.setConstant(1);
	reluPrime = (inputActivations.array() > 0).select(constOne, 0);
	activations = (inputActivations.array() > 0).select(inputActivations, 0);

#ifdef DEBUG_NN
	print_oneline(reluPrime, "reluPrime: ");
	print_oneline(activations, "activations: ");
#endif
}

void ReLULayerClear::computeDelta(RowMatrixXd &prevDelta)
{
    prevDelta = deltas.array() * reluPrime.array();
#ifdef DEBUG_NN
	print_oneline(reluPrime, "reluP: ");
	std::string str = "layer " + to_string(layerNum) + " delta: ";
    print_oneline(prevDelta, str);
#endif
}

void ReLULayerClear::updateEquations(const RowMatrixXd& prevActivations)
{
	log_print("ReLU.updateEquations");
}
} // namespace hmmpc
