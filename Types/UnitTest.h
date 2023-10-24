#ifndef TYPES_UNITTEST_H_
#define TYPES_UNITTEST_H_
#include "Protocols/PhaseConfig.h"
namespace hmmpc
{
void testCint(PhaseConfig *phase);
void testSfix(PhaseConfig *phase);
void testSintMul(PhaseConfig *phase);
void testSfixMul(PhaseConfig *phase);
void debugSfixMatMul(PhaseConfig *phase);
void debugSintMatMul(PhaseConfig *phase);
void debugSfixMatMulCfix(PhaseConfig *phase);

void debugSfixDivide(PhaseConfig*phase);
}
#endif