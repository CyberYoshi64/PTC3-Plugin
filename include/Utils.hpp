#ifndef _UTILS_HPP
#define _UTILS_HPP

#include <3ds/types.h>
#include "CTRPluginFramework.hpp"

#define MAX(a,b)    (((a) > (b)) ? (a):(b))
#define MIN(a,b)    (((a) < (b)) ? (a):(b))
#define CLP(v,a,b)  (((v) < (a)) ? (a) : (((v) > (b)) ? (b) : (v)));

int dateTimeToString(char *out, u64 in);

#endif