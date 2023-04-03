/* SPDX-License-Identifier: ISC */
/*
 * number_to_double.c
 *
 * Copyright (C) 2020 David Oberhollenzer <goliath@infraroot.at>
 */
#include "gcfg.h"

#include <math.h>

double gcfg_number_to_double(const gcfg_value_t *num, size_t index)
{
	double value = (double)num->data.number[index].value;
	int32_t exponent = num->data.number[index].exponent;

	return value * pow(10.0, exponent);
}
