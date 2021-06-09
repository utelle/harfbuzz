/*
 * Copyright © 2018  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Google Author(s): Rod Sheeter
 */

#ifndef HB_SUBSET_H
#define HB_SUBSET_H

#include "hb.h"

HB_BEGIN_DECLS

/*
 * hb_subset_input_t
 *
 * Things that change based on the input. Characters to keep, etc.
 */

typedef struct hb_subset_input_t hb_subset_input_t;

typedef enum
{
  HB_SUBSET_FLAG_HINTING =	     1,
  HB_SUBSET_FLAG_RETAIN_GIDS =	     2,
  HB_SUBSET_FLAG_DESUBROUTINIZE =    3,
  HB_SUBSET_FLAG_NAME_LEGACY =	     4,
  HB_SUBSET_FLAG_SET_OVERLAPS_FLAG = 5,
} hb_subset_flag_t;

HB_EXTERN hb_subset_input_t *
hb_subset_input_create_or_fail (void);

HB_EXTERN hb_subset_input_t *
hb_subset_input_reference (hb_subset_input_t *subset_input);

HB_EXTERN void
hb_subset_input_destroy (hb_subset_input_t *subset_input);

HB_EXTERN hb_set_t *
hb_subset_input_unicode_set (hb_subset_input_t *subset_input);

HB_EXTERN hb_set_t *
hb_subset_input_glyph_set (hb_subset_input_t *subset_input);

HB_EXTERN hb_set_t *
hb_subset_input_nameid_set (hb_subset_input_t *subset_input);

HB_EXTERN hb_set_t *
hb_subset_input_namelangid_set (hb_subset_input_t *subset_input);

HB_EXTERN hb_set_t *
hb_subset_input_layout_features_set (hb_subset_input_t *subset_input);

HB_EXTERN void
hb_subset_input_set_retain_all_features (hb_subset_input_t *subset_input,
					 hb_bool_t value);
HB_EXTERN hb_bool_t
hb_subset_input_get_retain_all_features (hb_subset_input_t *subset_input);

HB_EXTERN hb_set_t *
hb_subset_input_drop_tables_set (hb_subset_input_t *subset_input);


HB_EXTERN hb_bool_t
hb_subset_input_get_flag (hb_subset_input_t *input,
			  hb_subset_flag_t flag);

HB_EXTERN void
hb_subset_input_set_flag (hb_subset_input_t *input,
			  hb_subset_flag_t flag,
			  hb_bool_t value);

// TODO(garretrieger): remove bool property get/set methods.

HB_EXTERN void
hb_subset_input_set_overlaps_flag (hb_subset_input_t *subset_input,
				   hb_bool_t overlaps_flag);

HB_EXTERN hb_bool_t
hb_subset_input_get_overlaps_flag (hb_subset_input_t *subset_input);

HB_EXTERN void
hb_subset_input_set_notdef_outline (hb_subset_input_t *subset_input,
                                    hb_bool_t notdef_outline);

HB_EXTERN hb_bool_t
hb_subset_input_get_notdef_outline (hb_subset_input_t *subset_input);

HB_EXTERN void
hb_subset_input_set_no_prune_unicode_ranges (hb_subset_input_t *subset_input,
                                             hb_bool_t no_prune_unicode_ranges);

HB_EXTERN hb_bool_t
hb_subset_input_get_no_prune_unicode_ranges (hb_subset_input_t *subset_input);

/* hb_subset () */
HB_EXTERN hb_face_t *
hb_subset (hb_face_t *source, const hb_subset_input_t *input);

HB_EXTERN hb_face_t *
hb_subset_or_fail (hb_face_t *source, const hb_subset_input_t *input);

HB_EXTERN hb_bool_t
hb_subset_input_set_user_data (hb_subset_input_t  *input,
			       hb_user_data_key_t *key,
			       void *		   data,
			       hb_destroy_func_t   destroy,
			       hb_bool_t	   replace);

HB_EXTERN void *
hb_subset_input_get_user_data (const hb_subset_input_t *input,
			       hb_user_data_key_t	   *key);

HB_END_DECLS

#endif /* HB_SUBSET_H */
