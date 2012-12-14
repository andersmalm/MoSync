#include <e32math.h>
#include "config_platform.h"
#include <helpers/types.h>
#include <helpers/log.h>
#include "symbian_helpers.h"

extern "C" {

TReal sin(TReal x) {
	TReal d;
	//bugfix for 6630
	if(x == 0)
		return 0;
	//LOGD("sin(%f %016LX)\n", x, MAKE(TInt64, x));
	int result = Math::Sin(d, x);
	if(result == -6) {
		LOGD("sin error, returning 1\n");
		/*LOGD("Returning NaN\n");
		const TInt64 nan = MAKE_TINT64(0x7ff80000, 0x00000000);
		double d = MAKE(double, nan);
		LOGD("Returning NaN 2\n");*/
		return 1;
	}
	LHEL(result);
	LOGC("sin(%f %016LX) = %f %016LX\n", x, MAKE(TInt64, x), d, MAKE(TInt64, d));
	return d;
}

TReal cos(TReal a) {
	TReal s;
	Math::Cos(s, a);
	return s;
}

TReal exp(TReal a) {
	TReal s;
	Math::Exp(s, a);
	return s;
}

TReal log(TReal a) {
	TReal s;
	Math::Log(s, a);
	return s;
}

TReal sqrt(TReal a) {
	TReal s;
	Math::Sqrt(s, a);
	return s;
}

TReal pow(TReal a, TReal b) {
	TReal s;
	Math::Pow(s, a, b);
	return s;
}

TReal atan2(TReal a, TReal b) {
	TReal s;
	Math::ATan(s, a, b);
	return s;
}

TReal __floatdidf(TInt64 a) {
	return I64REAL(a);
}

TInt64 __fixdfdi(TReal a) {
	return TInt64(a);
}

TInt64 __fixunsdfdi(TReal a) {
	return TInt64(a);
}

int __cmpdi2(TInt64 a, TInt64 b) {
	return a > b ? 1 : ((a < b) ? -1 : 0);
}

}
