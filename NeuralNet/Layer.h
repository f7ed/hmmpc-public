#pragma once
#include "Types/sfixMatrix.h"
namespace hmmpc
{
class Layer
{
public:
    int layerNum = 0;
    Layer(int _layerNum):layerNum(_layerNum){}

    virtual void printLayer(){};
    virtual void forward(const sfixMatrix &inputActivations) = 0;
    virtual void forwardOnly(const sfixMatrix &inputActivations) = 0;
    virtual void computeDelta(sfixMatrix &prevDelta) = 0;
    virtual void updateEquations(const sfixMatrix &prevActivations)=0;

    virtual sfixMatrix& getActivation() = 0;
    virtual sfixMatrix& getDelta() = 0;

};

class LayerClear
{
public:
    int layerNum = 0;
    LayerClear(int _layerNum):layerNum(_layerNum){}

    virtual void printLayer(){};
    virtual void forward(const RowMatrixXd &inputActivations) = 0;
    virtual void computeDelta(RowMatrixXd &prevDelta) = 0;
    virtual void updateEquations(const RowMatrixXd &prevActivations) = 0;

    virtual RowMatrixXd& getActivation() = 0;
    virtual RowMatrixXd& getDelta() = 0;
};
} // namespace hmmpc
