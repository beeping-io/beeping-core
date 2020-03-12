#ifndef _defines_ooura_
#define _defines_ooura_

#include <math.h>

typedef int   INT32;
typedef float DATA;
typedef int  BOOL;

#define SOMAX(a,b) ((a >= b) ? a : b)
#define SOMIN(a,b) ((a < b) ? a : b)
#define _TWO_PI 6.28318530717958647692f
#define invTWO_PI 0.15915494309189533576901767873386f
#define _PI 3.141592653589793238462643f
#define PI_2 1.57079632679489661923f
#define INSIGNIFICANT 0.000001f

__inline DATA SOabs(DATA a)
{
	if (a >= 0)
		return a;
	else
		return -a;
}
//#define SOabs(a) ((a>=0) ? a : -a)

__inline DATA wrapPh(DATA ph)
{
	if ((ph < 0) || (ph >= _TWO_PI))
		ph -= floorf(ph*invTWO_PI)*_TWO_PI;
	if (ph>_PI)
		ph -= _TWO_PI;
	return ph;
}

__inline DATA getPhDiff(DATA toPhase,DATA fromPhase)
{
	if (toPhase >= fromPhase+_PI)
		return toPhase-_TWO_PI-fromPhase;
	else
	if (toPhase < fromPhase-_PI)
		return toPhase+_TWO_PI-fromPhase;
	else
		return toPhase-fromPhase;
}

__inline DATA getPh(DATA *pReal, DATA *pImag, DATA bin)
{
	INT32 iBin = (INT32)bin;
	DATA intp = bin-iBin;
	DATA lPh = atan2f( pImag[iBin] , pReal[iBin] );
	DATA rPh = atan2f( pImag[iBin+1] , pReal[iBin+1] );
	return lPh + intp*getPhDiff(rPh,lPh);
}

__inline void SOSwap( DATA **a, DATA **b )
{
	DATA *tmp = *a;
	*a = *b;
	*b = tmp;
}

#endif