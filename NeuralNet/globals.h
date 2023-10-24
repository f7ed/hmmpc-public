#ifndef NEUTALNET_GLOBALS_H_
#define NEUTALNET_GLOBALS_H_
#include <iostream>
#include <assert.h>
#include <Types/sfixMatrix.h>
#pragma once
using namespace std;
namespace hmmpc
{

#define FUNCTION_TIME false
#define LOG_DEBUG_NN false

static const size_t LOG_MINI_BATCH = 7;
static size_t LOG_LEARNING_RATE = 3;
static const size_t NUM_ITERATIONS = 1;
static size_t TRAINING_DATA_SIZE = 0;
static size_t TEST_DATA_SIZE = 0;
static size_t TRAIN_ITERATIONS;
static size_t TEST_ITERATIONS;

inline void log_print(string str)
{
#if (LOG_DEBUG_NN)
	cout << "----------------------------" << endl;
	cout << "Started " << str << endl;
	cout << "----------------------------" << endl;	
#endif
}

inline void check_overflow(sfixMatrix &matrix)
{
	matrix.reveal();
	for(size_t i = 0; i < matrix.size(); i++){
		float tmp = map_gfp_to_float(matrix.secret()(i));
		if(tmp >= 1e4 || tmp <= -1e4){
			assert(false && "Overflow");
		}
	}
}
}
#endif