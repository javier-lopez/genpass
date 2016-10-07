#ifndef _ENCODERS_H_
#define _ENCODERS_H_

#include "b10.h"
#include "b64.h"
#include "base91.h"
#include "base16.h"
#include "skey.h"
#include "z85.h"

/* encode stuff using basE91 internal functions */
int base91_glue_encode(unsigned char const *src, size_t srclength,
				  	   void *target, size_t targsize);

#endif