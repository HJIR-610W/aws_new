/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __VDSO_BITS_H
#define __VDSO_BITS_H


#define BIT(nr)			((1UL) << (nr))


#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)
#define _UL(x)		(_AC(x, UL))
#define _ULL(x)		(_AC(x, ULL))


#define _BITUL(x)	(_UL(1) << (x))
#define _BITULL(x)	(_ULL(1) << (x))

#endif	/* __VDSO_BITS_H */
