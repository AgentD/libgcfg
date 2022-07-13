/* SPDX-License-Identifier: ISC */
/*
 * number_to_double.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#include <math.h>

double gcfg_number_to_double(const gcfg_number_t *num)
{
	double ret;

	ret = (double)num->value * pow(10.0, num->exponent);

	return (num->flags & GCFG_NUM_NEGATIVE) ? -ret : ret;
}
