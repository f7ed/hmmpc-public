#pragma once
#include "Types/wrapper.h"

namespace hmmpc
{
void funcMatMul(const sfixMatrix&a, const sfixMatrix&b, sfixMatrix &res, bool a_transpose, bool b_transpose, size_t precision)
{
    if(!a_transpose && !b_transpose){res.share() = a.share() * b.share();}
    else if(!a_transpose && b_transpose){res.share() = a.share() * b.share().transpose();}
    else if(a_transpose && !b_transpose){res.share() = a.share().transpose() * b.share();}
    else {res.share() = a.share().transpose() * b.share().transpose();}
    if(precision==FIXED_PRECISION)
        res.reduce_truncate();
    else
        res.reduce_truncate(precision);
}

void funcCwiseMul(const sfixMatrix&a, const sintMatrix &b, sfixMatrix &res)
{
    res.share() = a.share().array() * b.share().array();
    res.reduce_degree();
}

void funcDivision(const sfixMatrix&a, const sfixMatrix &b, sfixMatrix &res)
{
    res = divideRowwise(a, b);
}

void funcTrunc(sfixMatrix &res, size_t precision)
{
    res.truncate(precision);
}

void funcReLU(const sfixMatrix &input, sintMatrix &reluPrime, sfixMatrix &activations)
{
    // input.ReLU(reluPrime, activations);
    input.ReLU_opt(reluPrime, activations);
}

void funcOnlyReLU(const sfixMatrix &input, sfixMatrix &activations)
{
    // input.ReLU(activations);
    input.ReLU_opt(activations);
}

void funcConvMatMul(const sfixMatrix &a, const sfixMatrix &b, const sfixMatrix &biases, sfixMatrix &res,
                     size_t B, size_t oh, size_t ow, size_t Dout)
{
    // a: (B*ow*oh, f*f*Din)
    // b: (f*f*Din, Dout)

    // tmp = B*ow*oh, Dout
    gfpMatrix tmp = (a.share() * b.share()).array() + gfpMatrix(biases.share().colwise().replicate(a.rows())).array();
    // res = B, ow*oh*Dout
    size_t nRow=ow*oh;
    for(size_t i = 0; i < B; i++){
        size_t startRow=i*nRow;
        // BUG LOG: The channel is stored sequentially in one row.
        res.share().row(i) = tmp.middleRows(startRow, nRow).reshaped().transpose();
    }
    // res.share() = ( (a.share() * b.share()).array() + gfpMatrix(biases.share().colwise().replicate(a.rows())).array() ).reshaped(B, oh*ow*Dout);//colmajor
    res.reduce_truncate();
}

void funcMaxpool(const sfixMatrix &input, sintMatrix &maxPrime, sfixMatrix &activations, 
                size_t B, size_t Din, size_t oh, size_t ow, size_t f)
{
    // input: (B*ow*oh*Din, f*f)
    // activations: (B, ow*oh*Din)
    // maxPrime: (B, ow*oh*Din*f*f)

    sfixMatrix tmpActivations(B*ow*oh*Din, 1);
    sintMatrix tmpPrime(B*ow*oh*Din, f*f);
    tmpPrime.share().setConstant(1);
    // input.Maxpool(tmpPrime, tmpActivations);
    input.Maxpool_opt(tmpPrime, tmpActivations);
    activations.share() = tmpActivations.share().reshaped(B, ow*oh*Din);
    maxPrime.share() = tmpPrime.share().reshaped<RowMajor>(B, ow*oh*Din*f*f);
}

void funcOnlyMaxpool(const sfixMatrix &input, sfixMatrix &activations,
                 size_t B, size_t Din, size_t oh, size_t ow)
{
    // input: (B*ow*oh*Din, f*f)
    // activations: (B, ow*oh*Din)
    sfixMatrix tmpActivations(B*ow*oh*Din, 1);
    // input.Maxpool(tmpActivations);
    input.Maxpool_opt(tmpActivations);
    activations.share() = tmpActivations.share().reshaped(B, ow*oh*Din);
}
}