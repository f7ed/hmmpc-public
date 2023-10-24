#pragma once

#include "NeuralNet/NeuralNetwork.h"
#include "NeuralNet/NeuralNetConfig.h"
#include "Protocols/PhaseConfig.h"

namespace hmmpc
{

void train(NeuralNetwork *net);
void test(NeuralNetwork *net);

void train(NeuralNetworkClear *net);
void test(NeuralNetworkClear *net);

void preload_netwok(bool PRELOADING, string network, NeuralNetwork *net);
void loadData(string net, string dataset, size_t test_data_size);
void loadPlainData(string net, string dataset);
void readMiniBatch(NeuralNetwork* net, string phase);
void readPlainMiniBatch(NeuralNetworkClear* net, string phase);

void printNetwork(NeuralNetwork* net);
void selectNetwork(string network, string dataset, NeuralNetConfig*config);
void runOnly(NeuralNetwork *net, size_t l, string what, string&network);

}