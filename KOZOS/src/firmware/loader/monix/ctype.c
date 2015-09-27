/* $Id: ctype.c,v 1.2.2.6 2007/01/18 14:30:00 kuwa Exp $ */
/*
 * Copyright (C) 2005-2007 Shuji KUWAHARA. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "ctype.h"

/*
ASCII code

000  0000  ^@	032  0x20   	064  0x40  @	096  0x60  `	
001  0x01  ^A	033  0x21  !	065  0x41  A	097  0x61  a	
002  0x02  ^B	034  0x22  "	066  0x42  B	098  0x62  b	
003  0x03  ^C	035  0x23  #	067  0x43  C	099  0x63  c	
004  0x04  ^D	036  0x24  $	068  0x44  D	100  0x64  d	
005  0x05  ^E	037  0x25  %	069  0x45  E	101  0x65  e	
006  0x06  ^F	038  0x26  &	070  0x46  F	102  0x66  f	
007  0x07  ^G	039  0x27  '	071  0x47  G	103  0x67  g	
008  0x08  ^H	040  0x28  (	072  0x48  H	104  0x68  h	
009  0x09  ^I	041  0x29  )	073  0x49  I	105  0x69  i	
010  0x0a  ^J	042  0x2a  *	074  0x4a  J	106  0x6a  j	
011  0x0b  ^K	043  0x2b  +	075  0x4b  K	107  0x6b  k	
012  0x0c  ^L	044  0x2c  ,	076  0x4c  L	108  0x6c  l	
013  0x0d  ^M	045  0x2d  -	077  0x4d  M	109  0x6d  m	
014  0x0e  ^N	046  0x2e  .	078  0x4e  N	110  0x6e  n	
015  0x0f  ^O	047  0x2f  /	079  0x4f  O	111  0x6f  o	
016  0x10  ^P	048  0x30  0	080  0x50  P	112  0x70  p	
017  0x11  ^Q	049  0x31  1	081  0x51  Q	113  0x71  q	
018  0x12  ^R	050  0x32  2	082  0x52  R	114  0x72  r	
019  0x13  ^S	051  0x33  3	083  0x53  S	115  0x73  s	
020  0x14  ^T	052  0x34  4	084  0x54  T	116  0x74  t	
021  0x15  ^U	053  0x35  5	085  0x55  U	117  0x75  u	
022  0x16  ^V	054  0x36  6	086  0x56  V	118  0x76  v	
023  0x17  ^W	055  0x37  7	087  0x57  W	119  0x77  w	
024  0x18  ^X	056  0x38  8	088  0x58  X	120  0x78  x	
025  0x19  ^Y	057  0x39  9	089  0x59  Y	121  0x79  y	
026  0x1a  ^Z	058  0x3a  :	090  0x5a  Z	122  0x7a  z	
027  0x1b  ^[	059  0x3b  ;	091  0x5b  [	123  0x7b  {	
028  0x1c  ^\	060  0x3c  <	092  0x5c  \	124  0x7c  |	
029  0x1d  ^]	061  0x3d  =	093  0x5d  ]	125  0x7d  }	
030  0x1e  ^^	062  0x3e  >	094  0x5e  ^	126  0x7e  ~	
031  0x1f  ^_	063  0x3f  ?	095  0x5f  _	127  0x7f  	
*/

const unsigned char __ctype[128] = {
	/* 0x00 ... 0x0f */
	__C, __C, __C, __C,
	__C, __C, __C, __C,
	__C, __C | __S, __C | __S, __C | __S,
	__C | __S, __C | __S, __C, __C,

	/* 0x10 ... 0x1f */
	__C, __C, __C, __C,
	__C, __C, __C, __C,
	__C, __C, __C, __C,
	__C, __C, __C, __C,

	/* 0x20 ... 0x2f */
	__S, __P, __P, __P,
	__P, __P, __P, __P,
	__P, __P, __P, __P,
	__P, __P, __P, __P,

	/* 0x30 ... 0x3f*/
	__D | __X, __D | __X, __D | __X, __D | __X,
	__D | __X, __D | __X, __D | __X, __D | __X,
	__D | __X, __D| __X, __P, __P,
	__P, __P, __P, __P,

	/* 0x40 ... 0x4f */
	__P, __U | __X, __U | __X, __U | __X,
	__U | __X, __U | __X, __U | __X, __U,
	__U, __U, __U, __U,
	__U, __U, __U, __U,

	/* 0x50 ... 0x5f */
	__U, __U, __U, __U,
	__U, __U, __U, __U,
	__U, __U, __U, __P,
	__P, __P, __P, __P,

	/* 0x60 ... 0x6f */
	__P, __L | __X, __L | __X, __L | __X,
	__L | __X, __L | __X, __L | __X, __L,
	__L, __L, __L, __L,
	__L, __L, __L, __L,

	/* 0x70 ... 0x7f */
	__L, __L, __L, __L,
	__L, __L, __L, __L,
	__L, __L, __L, __P,
	__P, __P, __P, __C
};

