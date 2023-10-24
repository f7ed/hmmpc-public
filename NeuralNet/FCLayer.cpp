#pragma once
#include "NeuralNet/FCLayer.h"
#include "NeuralNet/tools.h"
#include "Types/wrapper.h"
using namespace std;
namespace hmmpc
{

FCLayer::FCLayer(FCConfig* conf, int _layerNum)
:Layer(_layerNum),
conf(conf->inputDim, conf->batchSize, conf->outputDim),
activations(conf->batchSize, conf->outputDim),
deltas(conf->batchSize, conf->outputDim),
weights(conf->inputDim, conf->outputDim),
biases(conf->outputDim, 1)
{
    initialize();
}

void FCLayer::initialize()
{
    SeededPRNG prng;
    TYPE lower = 30, higher = 50, decimation = 10000;
    // if(partyNum==0){
    //     for(size_t i = 0; i < weights.size(); i++){
    //         float tmp = (float) (prng.get_uchar() % (higher-lower) + lower)/decimation;
    //         // float tmp = (float) (40 % (higher-lower) + lower)/decimation;
    //         weights.secret()(i) = map_float_to_gfp(tmp);
    //     }
    // }
    // weights.input_from_party(0);

    // for(size_t i = 0; i < weights.size(); i++){
    //     float tmp = (float) (ShareBase::PRNG_agreed.get_uchar() % (higher-lower) + lower)/decimation;
    //     weights.secret()(i) = map_float_to_gfp(tmp);
    // }
    // biases.share().setConstant(0);
}

void FCLayer::printLayer()
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") FC Layer\t\t  " << conf.inputDim << " x " << conf.outputDim << endl << "\t\t\t  "
		 << conf.batchSize << "\t\t (Batch Size)" << endl;
}

void FCLayer::forward(const sfixMatrix &inputActivations)
{
    log_print("FC.forward");

#ifdef DEBUG_NN
    cout << "----------------------------------------------" << endl;
	cout << "DEBUG: forward() at FCLayer.cpp" << endl;
#endif

    if (FUNCTION_TIME)
        cout << "funcMatMul: "<< funcTime(funcMatMul, inputActivations, weights, activations, 0, 0, FIXED_PRECISION) <<endl;
    else
        funcMatMul(inputActivations, weights, activations, 0, 0, FIXED_PRECISION);
    
    // activations = inputActivations * weights;
    for(size_t i = 0; i < conf.outputDim; i++){
        activations.share().col(i) = activations.share().col(i).array() + biases.share()(i);
    }

#ifdef DEBUG_NN
    cout<<"w: "<<weights.reveal(conf.outputDim)<<endl;
    cout<<"y: "<<activations.reveal(conf.outputDim)<<endl;
#endif
}

void FCLayer::forwardOnly(const sfixMatrix &inputActivations)
{
    forward(inputActivations);
}

void FCLayer::computeDelta(sfixMatrix &prevDelta)
{
    if (FUNCTION_TIME)
        cout << "funcMatMul: "<< funcTime(funcMatMul, deltas, weights, prevDelta, 0, 1, FIXED_PRECISION) <<endl;
    else
        funcMatMul(deltas, weights, prevDelta, 0, 1, FIXED_PRECISION);

#ifdef DEBUG_NN
    cout<<"w: "<<weights.reveal(conf.outputDim)<<endl;
    std::string str = "layer " + to_string(layerNum) + " delta: ";
    cout<<str<<prevDelta.reveal(prevDelta.cols())<<endl;
#endif
}

void FCLayer::updateEquations(const sfixMatrix &prevActivations)
{
    log_print("FC.updateEquations");

    sfixMatrix batchBiases(conf.outputDim, 1);
    batchBiases.share() = (deltas.share().colwise().sum()).reshaped<Eigen::RowMajor>();

    // batchBiases.truncate(LOG_LEARNING_RATE+LOG_MINI_BATCH);
    if (FUNCTION_TIME)
        cout << "funcT: "<< funcTime(funcTrunc, batchBiases, LOG_LEARNING_RATE+LOG_MINI_BATCH) <<endl;
    else
        funcTrunc(batchBiases, LOG_LEARNING_RATE+LOG_MINI_BATCH);


    biases.share() -= batchBiases.share();

    // Update Weights
    sfixMatrix deltaWeights(conf.inputDim, conf.outputDim);
    // deltaWeights.share() = prevActivations.share().transpose() * deltas.share();
    // deltaWeights.reduce_truncate(LOG_LEARNING_RATE, LOG_MINI_BATCH);
    if (FUNCTION_TIME)
        cout << "funcMatMul: "<< funcTime(funcMatMul, prevActivations, deltas, deltaWeights, 1, 0, FIXED_PRECISION+LOG_LEARNING_RATE+LOG_MINI_BATCH) <<endl;
    else
        funcMatMul(prevActivations, deltas, deltaWeights, 1, 0, FIXED_PRECISION+LOG_LEARNING_RATE+LOG_MINI_BATCH);
    
    weights.share() -= deltaWeights.share();
}

//*************FCLayerClear
FCLayerClear::FCLayerClear(FCConfig*conf, int _layerNum)
:LayerClear(_layerNum),
conf(conf->inputDim, conf->batchSize, conf->outputDim),
activations(conf->batchSize, conf->outputDim),
deltas(conf->batchSize, conf->outputDim),
weights(conf->inputDim, conf->outputDim),
biases(conf->outputDim, 1)
{
    initalize();
}

void FCLayerClear::initalize()
{
    SeededPRNG prng;
    TYPE lower = 30, higher = 50, decimation = 10000;
    for(size_t i = 0; i < weights.size(); i++){
        weights(i) = (float) (prng.get_uchar() % (higher-lower) + lower)/decimation;
    }
    biases.setConstant(0);
}

void FCLayerClear::printLayer()
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") FC Layer\t\t  " << conf.inputDim << " x " << conf.outputDim << endl << "\t\t\t  "
		 << conf.batchSize << "\t\t (Batch Size)" << endl;
}

void FCLayerClear::forward(const RowMatrixXd &inputActivations)
{
    log_print("FC.forward");

#ifdef DEBUG_NN
    cout << "----------------------------------------------" << endl;
	cout << "DEBUG: forward() at FCLayer.cpp" << endl;
#endif

    activations = inputActivations * weights;
    
    for(size_t i = 0; i < conf.outputDim; i++){
        activations.col(i) = activations.col(i).array() + biases(i);
    }

    // print_oneline(weights, "w: ");
    // print_oneline(activations, "y: ");

    // New
    // zetas = inputActivations * weights;
    
    // for(size_t i = 0; i < conf.outputDim; i++){
    //     zetas.col(i) = zetas.col(i).array() + biases(i);
    // }

    // RowMatrixXd constOne(MINI_BATCH_SIZE, conf.outputDim);
	// constOne.setConstant(1);
	// reluPrime = (zetas.array() > 0).select(constOne, 0);
	// activations = (zetas.array() > 0).select(zetas, 0);

    // print_oneline(weights, "w: ");
    // print_oneline(zetas, "zeta: ");
    // print_oneline(reluPrime, "reluP: ");
    // print_oneline(activations, "activations: ");
}

void FCLayerClear::computeDelta(RowMatrixXd &prevDelta)
{
    prevDelta = deltas * weights.transpose();

    // print_oneline(weights, "w: ");
    // std::string str = "layer " + to_string(layerNum) + " delta: ";
    // print_oneline(prevDelta, str);

}

void FCLayerClear::updateEquations(const RowMatrixXd &prevActivations)
{
    log_print("FC.updateEquations");

#ifdef DEBUG_NN
    cout << "Layer" <<layerNum<< endl;
#endif
    // print_oneline(deltas, "layer delta: ");
    // print_oneline(biases, "biases: ");
    RowMatrixXd batchDeltaBaises = (deltas.colwise().sum()).reshaped<Eigen::RowMajor>();
    
    // print_oneline(batchDeltaBaises, "baiseDelta: ");
    biases = biases.array() - batchDeltaBaises.array() / (1<<(LOG_LEARNING_RATE+LOG_MINI_BATCH));
    // print_oneline(biases, "baise: ");

    // print_oneline(prevActivations, "prevActivations: ");
    // print_oneline(deltas, "deltas: ");
    RowMatrixXd batchDeltaWeights = prevActivations.transpose() * deltas;
    // print_oneline(batchDeltaWeights, "weightsDelta: ");

    weights = weights.array() - batchDeltaWeights.array() / (1<<(LOG_LEARNING_RATE+LOG_MINI_BATCH));
    // print_oneline(weights, "w: ");
}

void FCLayerClear::printWeight(std::string fn)
{
    ofstream out(fn);
    for(size_t i = 0; i < conf.inputDim; i++){
        for(size_t j = 0; j < conf.outputDim; j++){
            out<<weights(i, j)<<" ";
        }
        out<<endl;
    }
}

void FCLayerClear::printBias(std::string fn)
{
    ofstream out(fn);
    for(size_t i = 0; i < conf.outputDim; i++){
        out<<biases(i)<<endl;
    }
}
}