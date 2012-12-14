/* Copyright (C) 2009 Mobile Sorcery AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#ifdef MA_PROF_SUPPORT_CLDC_10

#define DOUBLE Real
#define DZERO Real.ZERO

#define LTD(f, x) f.assignDoubleBits(x)
#define DTL(x) x.toDoubleBits()
#define ITF(f, x) f.assignFloatBits(x)
#define FTI(x) x.toFloatBits()

#define DASSIGN(dst, src) dst.assign(src)
#define EQUAL_TO(a, b) a.equalTo(b)

#define TO_INTEGER(x) x.toInteger()
#define TO_LONG(x) x.toLong()

#define FADD(a, b) a.add(b)
#define FSUB(a, b) a.sub(b)
#define FMUL(a, b) a.mul(b)
#define FDIV(a, b) a.div(b)
#define FSQRT(a, b) a.assign(b); a.sqrt();
#define FSIN(a, b) a.assign(b); a.sin();
#define FCOS(a, b) a.assign(b); a.cos();
#define FEXP(a, b) a.assign(b); a.exp();
#define FLOG(a, b) a.assign(b); a.ln();
#define FPOW(a, b) a.pow(b);
#define FATAN2(a, b) a.atan2(b);

#else

#define DOUBLE double
#define DZERO 0.0

#define LTD(f, x) f = Double.longBitsToDouble(x)
#define DTL(x) Double.doubleToLongBits(x)
#define ITF(f, x) f = (double)Float.intBitsToFloat(x)
#define FTI(x) Float.floatToIntBits((float)x)

#define DASSIGN(dst, src) dst = (double)src
#define EQUAL_TO(a, b) (a == b)

#define TO_INTEGER(x) (int)x
#define TO_LONG(x) (long)x

#define FADD(a, b) a += b
#define FSUB(a, b) a -= b
#define FMUL(a, b) a *= b
#define FDIV(a, b) a /= b
#define FSQRT(a, b) a = Math.sqrt(b);
#define FSIN(a, b) a = Math.sin(b);
#define FCOS(a, b) a = Math.cos(b);
#define FEXP(a, b) a = Float11.exp(b);
#define FLOG(a, b) a = Float11.log(b);
#define FPOW(a, b) a = Float11.pow(a, b);
#define FATAN2(a, b) a = Float11.atan2(a, b);

#endif
