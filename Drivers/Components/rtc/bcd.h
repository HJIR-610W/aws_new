/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BCD_H
#define _BCD_H

#include <stdint.h>

#define bcd2bin(x)					\
		(((uint8_t )(x)) ?	\
		const_bcd2bin(x) :			\
		_bcd2bin(x))

#define bin2bcd(x)					\
		(((uint8_t )(x)) ?	\
		const_bin2bcd(x) :			\
		_bin2bcd(x))

#define const_bcd2bin(x)	(((x) & 0x0f) + ((x) >> 4) * 10)
#define const_bin2bcd(x)	((((x) / 10) << 4) + (x) % 10)

unsigned _bcd2bin(unsigned char val) ;
unsigned char _bin2bcd(unsigned val) ;

#endif /* _BCD_H */
