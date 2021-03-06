/*
 * Algorithm lookup, for libreswan
 *
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

#include <stdlib.h>

#include "lswlog.h"
#include "alg_info.h"
#include "alg_byname.h"
#include "ike_alg.h"
#include "ike_alg_null.h"

bool alg_byname_ok(const struct parser_param *param,
		   const struct parser_policy *const policy,
		   const struct ike_alg *alg,
		   const char *name,
		   char *err_buf, size_t err_buf_len)
{
	/*
	 * If the connection is IKEv1|IKEv2 then this code will
	 * exclude anything not supported by both protocols.
	 */
	if (policy->ikev1 && alg->id[param->ikev1_alg_id] == 0) {
		snprintf(err_buf, err_buf_len,
			 "%s %s algorithm '%s' is not supported by IKEv1",
			 param->protocol, ike_alg_type_name(alg->algo_type),
			 name);
		return false;
	}
	if (policy->ikev2 && alg->id[IKEv2_ALG_ID] == 0) {
		snprintf(err_buf, err_buf_len,
			 "%s %s algorithm '%s' is not supported by IKEv2",
			 param->protocol, ike_alg_type_name(alg->algo_type),
			 name);
		return false;
	}
	/*
	 * Is it backed by an implementation?  For IKE that is
	 * in-process, for ESP/AH, presumably that is in the kernel
	 * (well except for DH which is still in-process).
	 */
	if (!param->alg_is_implemented(alg)) {
		snprintf(err_buf, err_buf_len,
			 "%s %s algorithm '%s' is not implemented",
			 param->protocol, ike_alg_type_name(alg->algo_type),
			 name);
		return false;
	}
	/*
	 * Check that the algorithm is valid.
	 *
	 * FIPS, for instance, will invalidate some algorithms during
	 * startup.
	 *
	 * Since it likely involves a lookup, it is left until last.
	 */
	if (!ike_alg_is_valid(alg)) {
		snprintf(err_buf, err_buf_len,
			 "%s %s algorithm '%s' is not valid",
			 param->protocol, ike_alg_type_name(alg->algo_type),
			 name);
		return false;
	}
	return true;
}

static const struct ike_alg *alg_byname(const struct parser_param *param,
					const struct parser_policy *const policy,
					const struct ike_alg_type *type,
					char *err_buf, size_t err_buf_len,
					const char *name)
{
	const struct ike_alg *alg = ike_alg_byname(type, name);
	if (alg == NULL) {
		/*
		 * Known at all?  Poke around in the enum tables to
		 * see if it turns up.
		 */
		if (ike_alg_enum_match(type, param->ikev1_alg_id, name) >= 0
		    || ike_alg_enum_match(type, IKEv2_ALG_ID, name) >= 0) {
			snprintf(err_buf, err_buf_len,
				 "%s %s algorithm '%s' is not supported",
				 param->protocol, ike_alg_type_name(type), name);
		} else {
			snprintf(err_buf, err_buf_len,
				 "%s %s algorithm '%s' is not recognized",
				 param->protocol, ike_alg_type_name(type), name);
		}
		return NULL;
	}

	/*
	 * Does it pass muster?
	 */
	if (!alg_byname_ok(param, policy, alg, name,
			   err_buf, err_buf_len)) {
		passert(err_buf[0] != '\0');
		return NULL;
	}

	return alg;
}

const struct ike_alg *encrypt_alg_byname(const struct parser_param *param,
					 const struct parser_policy *const policy,
					 char *err_buf, size_t err_buf_len,
					 const char *name, size_t key_bit_length)
{
	const struct ike_alg *alg = alg_byname(param, policy, IKE_ALG_ENCRYPT,
					       err_buf, err_buf_len, name);
	if (alg == NULL) {
		return NULL;
	}
	const struct encrypt_desc *encrypt = encrypt_desc(alg);
	if (!DBGP(IMPAIR_SEND_KEY_SIZE_CHECK) && key_bit_length > 0) {
		if (encrypt->keylen_omitted) {
			snprintf(err_buf, err_buf_len,
				 "%s does not take variable key lengths",
				 enum_short_name(&ikev2_trans_type_encr_names,
						 encrypt->common.id[IKEv2_ALG_ID]));
			return NULL;
		}
		if (!encrypt_has_key_bit_length(encrypt, key_bit_length)) {
			/*
			 * XXX: make list up to keep tests happy;
			 * should instead generate a real list from
			 * encrypt.
			 */
			snprintf(err_buf, err_buf_len,
				 "wrong encryption key length - key size must be 128 (default), 192 or 256");
			return NULL;
		}
	}
	return alg;
}

const struct ike_alg *prf_alg_byname(const struct parser_param *param,
				     const struct parser_policy *const policy,
				     char *err_buf, size_t err_buf_len,
				     const char *name,
				     size_t key_bit_length UNUSED)
{
	return alg_byname(param, policy, IKE_ALG_PRF,
			  err_buf, err_buf_len,
			  name);
}

const struct ike_alg *integ_alg_byname(const struct parser_param *param,
				       const struct parser_policy *const policy,
				       char *err_buf, size_t err_buf_len,
				       const char *name,
				       size_t key_bit_length UNUSED)
{
	if (strcasecmp(name, "null") == 0) {
		return &ike_alg_integ_null.common;
	}
	return alg_byname(param, policy, IKE_ALG_INTEG,
			  err_buf, err_buf_len,
			  name);
}

const struct ike_alg *dh_alg_byname(const struct parser_param *param,
				    const struct parser_policy *const policy,
				    char *err_buf, size_t err_buf_len,
				    const char *name,
				    size_t key_bit_length UNUSED)
{
	return alg_byname(param, policy, IKE_ALG_DH,
			  err_buf, err_buf_len,
			  name);
}
