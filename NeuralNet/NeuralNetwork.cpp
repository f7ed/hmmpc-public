#pragma once
#include "NeuralNet/NeuralNetwork.h"
#include "NeuralNet/FCLayer.h"
#include "NeuralNet/ReLULayer.h"
#include "NeuralNet/CNNLayer.h"
#include "NeuralNet/MaxpoolLayer.h"
#include "NeuralNet/NeuralNetConfig.h"
#include "NeuralNet/tools.h"
#include "Types/wrapper.h"

extern size_t INPUT_SIZE;
extern size_t LAST_LAYER_SIZE;
extern size_t NUM_LAYERS;
extern size_t MINI_BATCH_SIZE;
using namespace std;
namespace hmmpc
{
NeuralNetwork::NeuralNetwork(NeuralNetConfig *config)
:inputData(MINI_BATCH_SIZE, INPUT_SIZE), outputData(MINI_BATCH_SIZE, LAST_LAYER_SIZE)
{
    for(size_t i = 0; i < NUM_LAYERS; i++){
        if(config->layerConf[i]->type.compare("FC")==0){
            FCConfig *cfg = static_cast<FCConfig*>(config->layerConf[i]);
            layers.push_back(new FCLayer(cfg, i));
        }else if(config->layerConf[i]->type.compare("ReLU")==0){
            ReLUConfig *cfg = static_cast<ReLUConfig*>(config->layerConf[i]);
            layers.push_back(new ReLULayer(cfg, i));
        }else if(config->layerConf[i]->type.compare("Maxpool")==0){
            MaxpoolConfig *cfg = static_cast<MaxpoolConfig *>(config->layerConf[i]);
            layers.push_back(new MaxpoolLayer(cfg, i));
        }else if(config->layerConf[i]->type.compare("CNN")==0){
            CNNConfig *cfg = static_cast<CNNConfig*>(config->layerConf[i]);
            layers.push_back(new CNNLayer(cfg, i));
        }
	}
}

NeuralNetwork::~NeuralNetwork()
{
	for (vector<Layer*>::iterator it = layers.begin() ; it != layers.end(); ++it)
		delete (*it);

	layers.clear();
}

void NeuralNetwork::forward()
{
    log_print("NN.forward");

#ifdef DEBUG_NN
    cout << "----------------------------------------------" << endl;
	cout << "DEBUG: forward() at NeuralNetwork.cpp" << endl;
    cout<<"inputData: "<<inputData.reveal(784)<<endl;
#endif

    layers[0]->forward(inputData);

    for(size_t i = 1; i < NUM_LAYERS; i++){
        layers[i]->forward(layers[i-1]->getActivation());
    }
}

void NeuralNetwork::forwardOnly()
{
    log_print("NN.forwardOnly");

    layers[0]->forwardOnly(inputData);

    for(size_t i = 1; i < NUM_LAYERS; i++){
        layers[i]->forwardOnly(layers[i-1]->getActivation());
    }
}

void NeuralNetwork::backward()
{
    log_print("NN.backward");
    computeDelta();
    updateEquations();
}

void NeuralNetwork::computeDelta()
{
    log_print("NN.computeDelta");
    sfixMatrix rowSum(MINI_BATCH_SIZE, 1);

#ifdef DEBUG_NN    
    cout << "----------------------------------------------" << endl;
	cout << "DEBUG: computeDelta() at NeuralNetwork.cpp" << endl;
#endif
    sfixMatrix activations(MINI_BATCH_SIZE, LAST_LAYER_SIZE);
    if (FUNCTION_TIME)
        cout<<"funcOnlyReLU: "<<funcTime(funcOnlyReLU, layers[NUM_LAYERS-1]->getActivation(), activations)<<endl;
    else
        funcOnlyReLU(layers[NUM_LAYERS-1]->getActivation(), activations);

    layers[NUM_LAYERS-1]->getActivation().share() = activations.share();
    
    rowSum.share() = activations.share().rowwise().sum();

    // sfixMatrix softmaxOutput = divideRowwise(activations, rowSum);
    sfixMatrix softmaxOutput(MINI_BATCH_SIZE, LAST_LAYER_SIZE);
    if (FUNCTION_TIME)
        cout<<"funcDivision: "<<funcTime(funcDivision, activations, rowSum, softmaxOutput)<<endl;
    else
        funcDivision(activations, rowSum, softmaxOutput);
    // cout<<"softmax: "<<softmaxOutput.reveal(10*3)<<endl;

    layers[NUM_LAYERS-1]->getDelta() = softmaxOutput - outputData;
    
    // cout<<"last layer delta: "<<layers[NUM_LAYERS-1]->getDelta().reveal(10*3)<<endl;

    for(size_t i = NUM_LAYERS-1; i > 0; --i){
        layers[i]->computeDelta(layers[i-1]->getDelta());
    }
}

void NeuralNetwork::updateEquations()
{
    log_print("NN.updateEquations");

    for(size_t i = NUM_LAYERS-1; i > 0; --i){
        layers[i]->updateEquations(layers[i-1]->getActivation());
    }

    layers[0]->updateEquations(inputData);
}

void NeuralNetwork::predict(sintMatrix &maxIndex)
{
    log_print("NN.predict");
    // cout<<layers[NUM_LAYERS-1]->getActivation().reveal()<<endl;
    layers[NUM_LAYERS-1]->getActivation().MaxpoolPrime(maxIndex);
}

void NeuralNetwork::getAccuracy(sintMatrix &maxIndex, vector<size_t>&counter)
{
    log_print("NN.getAccuracy");
    gfpMatrix prediction = maxIndex.reveal().values;
    // cout<<prediction<<endl;
    gfpMatrix groundTruth = outputData.reveal().values;
    // cout<<groundTruth<<endl;
    gfpMatrix diff = (prediction.array() * groundTruth.array()).rowwise().sum();
    
    for(size_t i = 0; i < MINI_BATCH_SIZE; i++){
        if(diff(i).get_value()){
            counter[0]++;
        }
    }
    counter[1]+=MINI_BATCH_SIZE;
    cout << "Rolling accuracy: " << counter[0] << " out of " 
		 << counter[1] << " (" << (counter[0]*100/counter[1]) << " %)" << endl;
}

// NeuralNetworkClear

NeuralNetworkClear::NeuralNetworkClear(NeuralNetConfig *config)
:inputData(MINI_BATCH_SIZE, INPUT_SIZE), outputData(MINI_BATCH_SIZE, LAST_LAYER_SIZE)
{
    for(size_t i = 0; i < NUM_LAYERS; i++){
        if(config->layerConf[i]->type.compare("FC")==0){
            FCConfig *cfg = static_cast<FCConfig*>(config->layerConf[i]);
            layers.push_back(new FCLayerClear(cfg, i));
        }else if(config->layerConf[i]->type.compare("ReLU")==0){
            ReLUConfig *cfg = static_cast<ReLUConfig*>(config->layerConf[i]);
            layers.push_back(new ReLULayerClear(cfg, i));
        }
    }
}

NeuralNetworkClear::~NeuralNetworkClear()
{
	for (vector<LayerClear*>::iterator it = layers.begin() ; it != layers.end(); ++it)
		delete (*it);

	layers.clear();
}

void NeuralNetworkClear::forward()
{
    log_print("NN.forward");

#ifdef DEBUG_NN
    cout << "----------------------------------------------" << endl;
	cout << "DEBUG: forward() at NeuralNetwork.cpp" << endl;
#endif

    print_oneline(inputData, "inputData: ");
    // print_oneline(((FCLayerClear*)layers[0])->getWeights(), "w0: ");
    // print_oneline(layers[0]->getActivation(), "a0: ");

    layers[0]->forward(inputData);
    for(size_t i = 1; i < NUM_LAYERS; i++){
        layers[i]->forward(layers[i-1]->getActivation());
    }
}

void NeuralNetworkClear::backward()
{
    log_print("NN.backward");
    computeDelta();
    updateEquations();
}

void NeuralNetworkClear::computeDelta()
{
    log_print("NN.computeDelta");

#ifdef DEBUG_NN
    cout << "----------------------------------------------" << endl;
	cout << "DEBUG: computeDelta() at NeuralNetwork.cpp" << endl;
#endif

	layers[NUM_LAYERS-1]->getActivation() = (layers[NUM_LAYERS-1]->getActivation().array() > 0).select(layers[NUM_LAYERS-1]->getActivation(), 0);

    // print_oneline(layers[NUM_LAYERS-1]->getActivation(), "y_hat: ");
    RowMatrixXd rowSum = layers[NUM_LAYERS-1]->getActivation().rowwise().sum();

    // print_oneline(rowSum, "rowSum: ");

    RowMatrixXd softmaxOutput(MINI_BATCH_SIZE, LAST_LAYER_SIZE);
    for(size_t i = 0; i < MINI_BATCH_SIZE; i++)if(rowSum(i)){
        softmaxOutput.row(i) = layers[NUM_LAYERS-1]->getActivation().row(i).array() / rowSum(i);
    }

    // print_oneline(softmaxOutput, "softmax: ");

    layers[NUM_LAYERS-1]->getDelta() = softmaxOutput - outputData;

    // print_oneline(layers[NUM_LAYERS-1]->getDelta(), "last layer delta: ");

    for(size_t i = NUM_LAYERS-1; i>0; i--){
        layers[i]->computeDelta(layers[i-1]->getDelta());
    }
}

void NeuralNetworkClear::updateEquations()
{
    log_print("NN.updateEquations");

#ifdef DEBUG_NN
    cout << "----------------------------------------------" << endl;
	cout << "DEBUG: updateEquations() at FCLayer.cpp" << endl;
#endif
    for(size_t i = NUM_LAYERS-1; i > 0; --i){
        layers[i]->updateEquations(layers[i-1]->getActivation());
    }

    layers[0]->updateEquations(inputData);
}

void NeuralNetworkClear::predict(RowMatrixXd &maxIndex)
{
    log_print("NN.predict");
    RowMatrixXd maxCoeff = layers[NUM_LAYERS-1]->getActivation().rowwise().maxCoeff();
    RowMatrixXd constOne(1, LAST_LAYER_SIZE);
    constOne.setConstant(1);
    for(size_t i = 0; i < MINI_BATCH_SIZE; i++){
        maxIndex.row(i) = (layers[NUM_LAYERS-1]->getActivation().row(i).array() >= maxCoeff(i)).select(constOne, 0);
    }
}

void NeuralNetworkClear::getAccuracy(RowMatrixXd &maxIndex, vector<size_t> &counter)
{
    RowMatrixXd diff = (outputData.array() * maxIndex.array()).rowwise().sum();
    for(size_t i = 0; i < MINI_BATCH_SIZE; i++){
        if(diff(i)){
            counter[0]++;
        }
    }
    counter[1]+=MINI_BATCH_SIZE;
    cout << "Rolling accuracy: " << counter[0] << " out of " 
		 << counter[1] << " (" << (counter[0]*100/counter[1]) << " %)" << endl;
}

}