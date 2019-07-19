/* ================================================================ */
/*
File:	ConvertUTF.C
Author: Mark E. Davis
 * Copyright 1994-1999 IBM Corp.. All rights reserved.
 * 
 * IBM Corp. grants the user a nonexclusive right and license to use, execute,
 * display, modify, and prepare and/or have prepared derivative works of the
 * code, including but not limited to creating software products which
 * incorporate the code or derivative works thereof.
 * 
 * IBM Corp. grants the user a right and license to reproduce and distribute
 * the code as long as this entire copyright notice is reproduced in the code
 * or reproduction.
 * 
 * The code is provided AS-IS, and IBM Corp. disclaims all warranties,
 * either express or implied, including, but not limited to implied
 * warranties of merchantability and fitness for a particular purpose.
 * In no event will IBM Corp. be liable for any damages whatsoever (including,
 * without limitation, damages for loss of business profits, business
 * interruption, loss of business information, or other pecuniary
 * loss) arising out of the use or inability to use this code, even
 * if IBM Corp. has been advised of the possibility of such damages.
 * Because some states do not allow the exclusion or limitation of
 * liability for consequential or incidental damages, the above
 * limitation may not apply to you.
*/
/* ================================================================ */
#include "UT_CVTUTF.h"

/* ================================================================ */

const int halfShift				= 10;
const UCS4 halfBase				= 0x0010000UL;
const UCS4 halfMask				= 0x3FFUL;
const UCS4 kSurrogateHighStart	= 0xD800UL;
const UCS4 kSurrogateHighEnd	= 0xDBFFUL;
const UCS4 kSurrogateLowStart	= 0xDC00UL;
const UCS4 kSurrogateLowEnd		= 0xDFFFUL;

/* ================================================================ */

ConversionResult	ConvertUCS4toUTF16 (
		UCS4** sourceStart, const UCS4* sourceEnd, 
		UTF16** targetStart, const UTF16* targetEnd) {
	ConversionResult result = ok;
	UCS4* source = *sourceStart;
	UTF16* target = *targetStart;
	while (source < sourceEnd) {
		UCS4 ch;
		if (target >= targetEnd) {
			result = targetExhausted; break;
		};
		ch = *source++;
		if (ch <= kMaximumUCS2) {
			*target++ = ch;
		} else if (ch > kMaximumUTF16) {
			*target++ = kReplacementCharacter;
		} else {
			if (target + 1 >= targetEnd) {
				result = targetExhausted; break;
			};
			ch -= halfBase;
			*target++ = (ch >> halfShift) + kSurrogateHighStart;
			*target++ = (ch & halfMask) + kSurrogateLowStart;
		};
	};
	*sourceStart = source;
	*targetStart = target;
	return result;
};

/* ================================================================ */

ConversionResult	ConvertUTF16toUCS4 (
		UTF16** sourceStart, UTF16* sourceEnd, 
		UCS4** targetStart, const UCS4* targetEnd) {
	ConversionResult result = ok;
	UTF16* source = *sourceStart;
	UCS4* target = *targetStart;
	while (source < sourceEnd) {
		UCS4 ch;
		ch = *source++;
		if (ch >= kSurrogateHighStart && ch <= kSurrogateHighEnd && source < sourceEnd) {
			UCS4 ch2 = *source;
			if (ch2 >= kSurrogateLowStart && ch2 <= kSurrogateLowEnd) {
				ch = ((ch - kSurrogateHighStart) << halfShift)
					+ (ch2 - kSurrogateLowStart) + halfBase;
				++source;
			};
		};
		if (target >= targetEnd) {
			result = targetExhausted; break;
		};
		*target++ = ch;
	};
	*sourceStart = source;
	*targetStart = target;
	return result;
};

/* ================================================================ */

UCS4 offsetsFromUTF8[6] =	{0x00000000UL, 0x00003080UL, 0x000E2080UL, 
					 	 	 0x03C82080UL, 0xFA082080UL, 0x82082080UL};
char bytesFromUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};

UTF8 firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

/* ================================================================ */
/*	This code is similar in effect to making successive calls on the
mbtowc and wctomb routines in FSS-UTF. However, it is considerably
different in code:
* it is adapted to be consistent with UTF16,
* the interface converts a whole buffer to avoid function-call overhead
* constants have been gathered.
* loops & conditionals have been removed as much as possible for
efficiency, in favor of drop-through switch statements.
*/

/* ================================================================ */
ConversionResult	ConvertUTF16toUTF8 (
		UTF16** sourceStart, const UTF16* sourceEnd, 
		UTF8** targetStart, const UTF8* targetEnd)
{
	ConversionResult result = ok;
	UTF16* source = *sourceStart;
	UTF8* target = *targetStart;
	while (source < sourceEnd) {
		UCS4 ch;
		unsigned short bytesToWrite = 0;
		const UCS4 byteMask = 0xBF;
		const UCS4 byteMark = 0x80;
		ch = *source++;
		if (ch >= kSurrogateHighStart && ch <= kSurrogateHighEnd
				&& source < sourceEnd) {
			 UCS4 ch2 = *source;
			if (ch2 >= kSurrogateLowStart && ch2 <= kSurrogateLowEnd) {
				ch = ((ch - kSurrogateHighStart) << halfShift)
					+ (ch2 - kSurrogateLowStart) + halfBase;
				++source;
			};
		};
		if (ch < 0x80) {				bytesToWrite = 1;
		} else if (ch < 0x800) {		bytesToWrite = 2;
		} else if (ch < 0x10000) {		bytesToWrite = 3;
		} else if (ch < 0x200000) {		bytesToWrite = 4;
		} else if (ch < 0x4000000) {	bytesToWrite = 5;
		} else if (ch <= kMaximumUCS4){	bytesToWrite = 6;
		} else {						bytesToWrite = 2;
										ch = kReplacementCharacter;
		}; /* I wish there were a smart way to avoid this conditional */
		
		target += bytesToWrite;
		if (target > targetEnd) {
			target -= bytesToWrite; result = targetExhausted; break;
		};
		switch (bytesToWrite) {	/* note: code falls through cases! */
			case 6:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 5:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 4:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 3:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 2:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 1:	*--target =  ch | firstByteMark[bytesToWrite];
		};
		target += bytesToWrite;
	};
	*sourceStart = source;
	*targetStart = target;
	return result;
};

/* ================================================================ */

ConversionResult	ConvertUTF8toUTF16 (
		UTF8** sourceStart, UTF8* sourceEnd, 
		UTF16** targetStart, const UTF16* targetEnd)
{
	ConversionResult result = ok;
	UTF8* source = *sourceStart;
	UTF16* target = *targetStart;
	while (source < sourceEnd) {
		UCS4 ch = 0;
		unsigned short extraBytesToWrite = bytesFromUTF8[*source];
		if (source + extraBytesToWrite > sourceEnd) {
			result = sourceExhausted; break;
		};
		switch(extraBytesToWrite) {	/* note: code falls through cases! */
			case 5:	ch += *source++; ch <<= 6;
			case 4:	ch += *source++; ch <<= 6;
			case 3:	ch += *source++; ch <<= 6;
			case 2:	ch += *source++; ch <<= 6;
			case 1:	ch += *source++; ch <<= 6;
			case 0:	ch += *source++;
		};
		ch -= offsetsFromUTF8[extraBytesToWrite];

		if (target >= targetEnd) {
			result = targetExhausted; break;
		};
		if (ch <= kMaximumUCS2) {
			*target++ = ch;
		} else if (ch > kMaximumUTF16) {
			*target++ = kReplacementCharacter;
		} else {
			if (target + 1 >= targetEnd) {
				result = targetExhausted; break;
			};
			ch -= halfBase;
			*target++ = (ch >> halfShift) + kSurrogateHighStart;
			*target++ = (ch & halfMask) + kSurrogateLowStart;
		};
	};
	*sourceStart = source;
	*targetStart = target;
	return result;
};

/* ================================================================ */
ConversionResult	ConvertUCS4toUTF8 (
		UCS4** sourceStart, const UCS4* sourceEnd, 
		UTF8** targetStart, const UTF8* targetEnd)
{
	ConversionResult result = ok;
	UCS4* source = *sourceStart;
	UTF8* target = *targetStart;
	while (source < sourceEnd) {
		UCS4 ch;
		unsigned short bytesToWrite = 0;
		const UCS4 byteMask = 0xBF;
		const UCS4 byteMark = 0x80;
		ch = *source++;
		if (ch >= kSurrogateHighStart && ch <= kSurrogateHighEnd
				&& source < sourceEnd) {
			UCS4 ch2 = *source;
			if (ch2 >= kSurrogateLowStart && ch2 <= kSurrogateLowEnd) {
				ch = ((ch - kSurrogateHighStart) << halfShift)
					+ (ch2 - kSurrogateLowStart) + halfBase;
				++source;
			};
		};
		if (ch < 0x80) {				bytesToWrite = 1;
		} else if (ch < 0x800) {		bytesToWrite = 2;
		} else if (ch < 0x10000) {		bytesToWrite = 3;
		} else if (ch < 0x200000) {		bytesToWrite = 4;
		} else if (ch < 0x4000000) {	bytesToWrite = 5;
		} else if (ch <= kMaximumUCS4){	bytesToWrite = 6;
		} else {						bytesToWrite = 2;
										ch = kReplacementCharacter;
		}; /* I wish there were a smart way to avoid this conditional */
		
		target += bytesToWrite;
		if (target > targetEnd) {
			target -= bytesToWrite; result = targetExhausted; break;
		};
		switch (bytesToWrite) {	/* note: code falls through cases! */
			case 6:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 5:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 4:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 3:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 2:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 1:	*--target =  ch | firstByteMark[bytesToWrite];
		};
		target += bytesToWrite;
	};
	*sourceStart = source;
	*targetStart = target;
	return result;
};

/* ================================================================ */

ConversionResult	ConvertUTF8toUCS4 (
		UTF8** sourceStart, UTF8* sourceEnd, 
		UCS4** targetStart, const UCS4* targetEnd)
{
	ConversionResult result = ok;
	UTF8* source = *sourceStart;
	UCS4* target = *targetStart;
	while (source < sourceEnd) {
		UCS4 ch = 0;
		unsigned short extraBytesToWrite = bytesFromUTF8[*source];
		if (source + extraBytesToWrite > sourceEnd) {
			result = sourceExhausted; break;
		};
		switch(extraBytesToWrite) {	/* note: code falls through cases! */
			case 5:	ch += *source++; ch <<= 6;
			case 4:	ch += *source++; ch <<= 6;
			case 3:	ch += *source++; ch <<= 6;
			case 2:	ch += *source++; ch <<= 6;
			case 1:	ch += *source++; ch <<= 6;
			case 0:	ch += *source++;
		};
		ch -= offsetsFromUTF8[extraBytesToWrite];

		if (target >= targetEnd) {
			result = targetExhausted; break;
		};
		if (ch <= kMaximumUCS2) {
			*target++ = ch;
		} else if (ch > kMaximumUCS4) {
			*target++ = kReplacementCharacter;
		} else {
			if (target + 1 >= targetEnd) {
				result = targetExhausted; break;
			};
			ch -= halfBase;
			*target++ = (ch >> halfShift) + kSurrogateHighStart;
			*target++ = (ch & halfMask) + kSurrogateLowStart;
		};
	};
	*sourceStart = source;
	*targetStart = target;
	return result;
};
