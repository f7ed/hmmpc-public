#pragma once

#include <iostream>
#include "NeuralNet/globals.h"
namespace hmmpc
{
class LayerConfig
{
private:
    /* data */
public:
    std::string type;
    LayerConfig(std::string _type):type(_type){}
};

}
