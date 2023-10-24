#pragma once
#include "NeuralNet/FCConfig.h"
#include "NeuralNet/Layer.h"

extern int partyNum;
namespace hmmpc
{

class FCLayer : public Layer
{
private:
    FCConfig conf;
    sfixMatrix activations;
    sfixMatrix deltas;
    sfixMatrix weights;
    sfixMatrix biases;

public:
    FCLayer(FCConfig* conf, int _layerNum);
    void initialize();

    void printLayer() override;
    void forward(const sfixMatrix& inputActivations) override;
    void forwardOnly(const sfixMatrix &inputActivations) override;
    void computeDelta(sfixMatrix &prevDelta)override;
    void updateEquations(const sfixMatrix &prevActivations)override;

    sfixMatrix& getActivation(){sfixMatrix &ref = activations; return ref;}
    sfixMatrix& getDelta(){sfixMatrix &ref = deltas; return ref;}
    sfixMatrix& getWeights(){sfixMatrix &ref = weights; return ref;}
    sfixMatrix& getBias(){sfixMatrix &ref = biases; return ref;}
};

class FCLayerClear: public LayerClear
{
private:
    FCConfig conf;
    RowMatrixXd activations;
    RowMatrixXd deltas;
    RowMatrixXd weights;
    RowMatrixXd biases;

public:
    FCLayerClear(FCConfig*conf, int _layerNum);

    void initalize();
    void printLayer() override;
    void forward(const RowMatrixXd& inputActivations) override;
    void computeDelta(RowMatrixXd &prevDelta)override;
    void updateEquations(const RowMatrixXd &prevActivations)override;

    RowMatrixXd& getActivation(){RowMatrixXd &ref = activations; return ref;}
    RowMatrixXd& getDelta(){RowMatrixXd &ref = deltas; return ref;}
    RowMatrixXd& getWeights(){RowMatrixXd &ref = weights; return ref;}
    RowMatrixXd& getBias(){RowMatrixXd &ref = biases; return ref;}
    
    void printWeight(std::string fn);
    void printBias(std::string fn);
};
}