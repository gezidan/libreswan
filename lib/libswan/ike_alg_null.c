/*
 * Copyright (C) 2017 Andrew Cagney <cagney@gnu.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>

#include "libreswan.h"
#include "constants.h"
#include "ike_alg.h"
#include "ike_alg_null.h"

/*
 * References for NULL.
 *
 * https://tools.ietf.org/html/rfc2410
 */

const struct encrypt_desc ike_alg_encrypt_null =
{
	.common = {
		.name = "null",
		.fqn = "NULL",
		.names = { "null", },
		.officname = "null",
		.algo_type = IKE_ALG_ENCRYPT,
		.id = {
			[IKEv1_ESP_ID] = ESP_NULL,
			[IKEv2_ALG_ID] = IKEv2_ENCR_NULL,
		},
	},
	.enc_blocksize =  1,
	.wire_iv_size =  0,
	.pad_to_blocksize = FALSE,
	.keylen_omitted = TRUE,
	.keydeflen = 0,
	.key_bit_lengths = { 0, },
};

/*
 * This doesn't really exist, yet life is easier if it does.
 */
const struct integ_desc ike_alg_integ_null = {
       .common = {
               .name = "null",
               .fqn = "NULL",
	       .names = { "null", },
	       .officname = "null",
	       .algo_type = IKE_ALG_INTEG,
	       .id = {
			/* [IKEv1_OAKLEY_ID] = AUTH_ALGORITHM_NONE or AUTH_ALGORITHM_NULL_KAME? */
			[IKEv1_ESP_ID] = ESP_reserved, /* not ESP_KAME_NULL=251 */
			[IKEv2_ALG_ID] = IKEv2_AUTH_NONE,
		},
       },
};
