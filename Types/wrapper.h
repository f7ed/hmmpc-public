#pragma once
#include "Types/sfixMatrix.h"
#include "Types/sintMatrix.h"

namespace hmmpc
{

void funcMatMul(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix &res, 
                            bool a_transpose, bool b_transpose, size_t precision=FIXED_PRECISION);
void funcCwiseMul(const sfixMatrix&a, const sintMatrix &b, sfixMatrix &res);
void funcDivision(const sfixMatrix&a, const sfixMatrix &b, sfixMatrix &res);
void funcTrunc(sfixMatrix &res, size_t precision);

void funcReLU(const sfixMatrix &input, sintMatrix &reluPrime, sfixMatrix &activations);
void funcOnlyReLU(const sfixMatrix &input, sfixMatrix &activations);

void funcConvMatMul(const sfixMatrix &a, const sfixMatrix &b, const sfixMatrix &biases, sfixMatrix &res,
                     size_t B, size_t oh, size_t ow, size_t Dout);

void funcMaxpool(const sfixMatrix &input, sintMatrix &maxPrime, sfixMatrix &activations, 
                size_t B, size_t Din, size_t oh, size_t ow, size_t f);
void funcOnlyMaxpool(const sfixMatrix &input, sfixMatrix &activations,
                 size_t B, size_t Din, size_t oh, size_t ow);
}