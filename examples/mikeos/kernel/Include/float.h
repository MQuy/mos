#if _MSC_VER > 1000
#pragma once
#endif

#ifndef FLOAT_H
#define FLOAT_H

#define FLT_RADIX     2
#define FLT_ROUNDS    1
#define FLT_DIG       6
#define FLT_EPSILON   1.192092896e-07F
#define FLT_MANT_DIG  24
#define FLT_MAX       3.402823466e+38F
#define FLT_MAX_EXP   38
#define FLT_MIN       1.175494351e-38F
#define FLT_MIN_EXP   (-37)

#define DBL_DIG        15
#define DBL_EPSILON    2.2204460492503131e-016
#define DBL_MANT_DIG   53
#define DBL_MAX        1.7976931348623158e+308
#define DBL_MAX_EXP    308
#define DBL_MIN        2.2250738585072014e-308
#define DBL_MIN_EXP    (-307)

#ifdef  __cplusplus
extern "C" {
#endif

void _fpreset();

#ifdef  __cplusplus
}
#endif

#endif

