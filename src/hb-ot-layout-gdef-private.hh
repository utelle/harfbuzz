/*
 * Copyright (C) 2007,2008,2009  Red Hat, Inc.
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
 * Red Hat Author(s): Behdad Esfahbod
 */

#ifndef HB_OT_LAYOUT_GDEF_PRIVATE_HH
#define HB_OT_LAYOUT_GDEF_PRIVATE_HH

#include "hb-ot-layout-common-private.hh"

#include "hb-font-private.h"


/*
 * Attachment List Table
 */

typedef ArrayOf<USHORT> AttachPoint;	/* Array of contour point indices--in
					 * increasing numerical order */

struct AttachList
{
  inline unsigned int get_attach_points (hb_codepoint_t glyph_id,
					 unsigned int start_offset,
					 unsigned int *point_count /* IN/OUT */,
					 unsigned int *point_array /* OUT */) const
  {
    unsigned int index = (this+coverage) (glyph_id);
    if (index == NOT_COVERED)
    {
      if (point_count)
	*point_count = 0;
      return 0;
    }

    const AttachPoint &points = this+attachPoint[index];

    if (point_count) {
      const USHORT *array = points.sub_array (start_offset, point_count);
      unsigned int count = *point_count;
      for (unsigned int i = 0; i < count; i++)
	point_array[i] = array[i];
    }

    return points.len;
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return coverage.sanitize (context, this)
	&& attachPoint.sanitize (context, this);
  }

  private:
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table -- from
					 * beginning of AttachList table */
  OffsetArrayOf<AttachPoint>
		attachPoint;		/* Array of AttachPoint tables
					 * in Coverage Index order */
  public:
  DEFINE_SIZE_VAR (4, OffsetTo<AttachPoint>);
};

/*
 * Ligature Caret Table
 */

struct CaretValueFormat1
{
  friend struct CaretValue;

  private:
  inline int get_caret_value (hb_ot_layout_context_t *context, hb_codepoint_t glyph_id HB_UNUSED) const
  {
    /* TODO vertical */
    return _hb_16dot16_mul_round (context->font->x_scale, coordinate);
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return context->check_struct (this);
  }

  private:
  USHORT	caretValueFormat;	/* Format identifier--format = 1 */
  SHORT		coordinate;		/* X or Y value, in design units */
  public:
  DEFINE_SIZE_STATIC (4);
};

struct CaretValueFormat2
{
  friend struct CaretValue;

  private:
  inline int get_caret_value (hb_ot_layout_context_t *context, hb_codepoint_t glyph_id) const
  {
    /* TODO vertical */
    hb_position_t x, y;
    if (hb_font_get_contour_point (context->font, context->face, caretValuePoint, glyph_id, &x, &y))
      return x;
    else
      return 0;
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return context->check_struct (this);
  }

  private:
  USHORT	caretValueFormat;	/* Format identifier--format = 2 */
  USHORT	caretValuePoint;	/* Contour point index on glyph */
  public:
  DEFINE_SIZE_STATIC (4);
};

struct CaretValueFormat3
{
  friend struct CaretValue;

  inline int get_caret_value (hb_ot_layout_context_t *context, hb_codepoint_t glyph_id HB_UNUSED) const
  {
    /* TODO vertical */
    return _hb_16dot16_mul_round (context->font->x_scale, coordinate) +
	   ((this+deviceTable).get_delta (context->font->x_ppem) << 16);
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return context->check_struct (this)
	&& deviceTable.sanitize (context, this);
  }

  private:
  USHORT	caretValueFormat;	/* Format identifier--format = 3 */
  SHORT		coordinate;		/* X or Y value, in design units */
  OffsetTo<Device>
		deviceTable;		/* Offset to Device table for X or Y
					 * value--from beginning of CaretValue
					 * table */
  public:
  DEFINE_SIZE_STATIC (6);
};

struct CaretValue
{
  inline int get_caret_value (hb_ot_layout_context_t *context, hb_codepoint_t glyph_id) const
  {
    switch (u.format) {
    case 1: return u.format1->get_caret_value (context, glyph_id);
    case 2: return u.format2->get_caret_value (context, glyph_id);
    case 3: return u.format3->get_caret_value (context, glyph_id);
    default:return 0;
    }
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    if (!u.format.sanitize (context)) return false;
    switch (u.format) {
    case 1: return u.format1->sanitize (context);
    case 2: return u.format2->sanitize (context);
    case 3: return u.format3->sanitize (context);
    default:return true;
    }
  }

  private:
  union {
  USHORT		format;		/* Format identifier */
  CaretValueFormat1	format1[VAR];
  CaretValueFormat2	format2[VAR];
  CaretValueFormat3	format3[VAR];
  } u;
  public:
  DEFINE_SIZE_UNION (2, format);
};

struct LigGlyph
{
  inline unsigned int get_lig_carets (hb_ot_layout_context_t *context,
				      hb_codepoint_t glyph_id,
				      unsigned int start_offset,
				      unsigned int *caret_count /* IN/OUT */,
				      int *caret_array /* OUT */) const
  {
    if (caret_count) {
      const OffsetTo<CaretValue> *array = carets.sub_array (start_offset, caret_count);
      unsigned int count = *caret_count;
      for (unsigned int i = 0; i < count; i++)
	caret_array[i] = (this+array[i]).get_caret_value (context, glyph_id);
    }

    return carets.len;
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return carets.sanitize (context, this);
  }

  private:
  OffsetArrayOf<CaretValue>
		carets;			/* Offset array of CaretValue tables
					 * --from beginning of LigGlyph table
					 * --in increasing coordinate order */
  public:
  DEFINE_SIZE_VAR (2, OffsetTo<CaretValue>);
};

struct LigCaretList
{
  inline unsigned int get_lig_carets (hb_ot_layout_context_t *context,
				      hb_codepoint_t glyph_id,
				      unsigned int start_offset,
				      unsigned int *caret_count /* IN/OUT */,
				      int *caret_array /* OUT */) const
  {
    unsigned int index = (this+coverage) (glyph_id);
    if (index == NOT_COVERED)
    {
      if (caret_count)
	*caret_count = 0;
      return 0;
    }
    const LigGlyph &lig_glyph = this+ligGlyph[index];
    return lig_glyph.get_lig_carets (context, glyph_id, start_offset, caret_count, caret_array);
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return coverage.sanitize (context, this)
	&& ligGlyph.sanitize (context, this);
  }

  private:
  OffsetTo<Coverage>
		coverage;		/* Offset to Coverage table--from
					 * beginning of LigCaretList table */
  OffsetArrayOf<LigGlyph>
		ligGlyph;		/* Array of LigGlyph tables
					 * in Coverage Index order */
  public:
  DEFINE_SIZE_VAR (4, OffsetTo<LigGlyph>);
};


struct MarkGlyphSetsFormat1
{
  inline bool covers (unsigned int set_index, hb_codepoint_t glyph_id) const
  { return (this+coverage[set_index]).get_coverage (glyph_id) != NOT_COVERED; }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return coverage.sanitize (context, this);
  }

  private:
  USHORT	format;			/* Format identifier--format = 1 */
  LongOffsetArrayOf<Coverage>
		coverage;		/* Array of long offsets to mark set
					 * coverage tables */
  public:
  DEFINE_SIZE_VAR (4, LongOffsetTo<Coverage>);
};

struct MarkGlyphSets
{
  inline bool covers (unsigned int set_index, hb_codepoint_t glyph_id) const
  {
    switch (u.format) {
    case 1: return u.format1->covers (set_index, glyph_id);
    default:return false;
    }
  }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    if (!u.format.sanitize (context)) return false;
    switch (u.format) {
    case 1: return u.format1->sanitize (context);
    default:return true;
    }
  }

  private:
  union {
  USHORT		format;		/* Format identifier */
  MarkGlyphSetsFormat1	format1[VAR];
  } u;
  public:
  DEFINE_SIZE_UNION (2, format);
};


/*
 * GDEF
 */

struct GDEF
{
  static const hb_tag_t Tag	= HB_OT_TAG_GDEF;

  enum {
    UnclassifiedGlyph	= 0,
    BaseGlyph		= 1,
    LigatureGlyph	= 2,
    MarkGlyph		= 3,
    ComponentGlyph	= 4
  };

  inline bool has_glyph_classes () const { return glyphClassDef != 0; }
  inline hb_ot_layout_class_t get_glyph_class (hb_codepoint_t glyph) const
  { return (this+glyphClassDef).get_class (glyph); }

  inline bool has_mark_attachment_types () const { return markAttachClassDef != 0; }
  inline hb_ot_layout_class_t get_mark_attachment_type (hb_codepoint_t glyph) const
  { return (this+markAttachClassDef).get_class (glyph); }

  inline bool has_attach_points () const { return attachList != 0; }
  inline unsigned int get_attach_points (hb_codepoint_t glyph_id,
					 unsigned int start_offset,
					 unsigned int *point_count /* IN/OUT */,
					 unsigned int *point_array /* OUT */) const
  { return (this+attachList).get_attach_points (glyph_id, start_offset, point_count, point_array); }

  inline bool has_lig_carets () const { return ligCaretList != 0; }
  inline unsigned int get_lig_carets (hb_ot_layout_context_t *context,
				      hb_codepoint_t glyph_id,
				      unsigned int start_offset,
				      unsigned int *caret_count /* IN/OUT */,
				      int *caret_array /* OUT */) const
  { return (this+ligCaretList).get_lig_carets (context, glyph_id, start_offset, caret_count, caret_array); }

  inline bool has_mark_sets () const { return version >= 0x00010002 && markGlyphSetsDef[0] != 0; }
  inline bool mark_set_covers (unsigned int set_index, hb_codepoint_t glyph_id) const
  { return version >= 0x00010002 && (this+markGlyphSetsDef[0]).covers (set_index, glyph_id); }

  inline bool sanitize (hb_sanitize_context_t *context) {
    TRACE_SANITIZE ();
    return version.sanitize (context) && likely (version.major == 1)
	&& glyphClassDef.sanitize (context, this)
	&& attachList.sanitize (context, this)
	&& ligCaretList.sanitize (context, this)
	&& markAttachClassDef.sanitize (context, this)
	&& (version < 0x00010002 || markGlyphSetsDef[0].sanitize (context, this));
  }

  private:
  FixedVersion	version;		/* Version of the GDEF table--currently
					 * 0x00010002 */
  OffsetTo<ClassDef>
		glyphClassDef;		/* Offset to class definition table
					 * for glyph type--from beginning of
					 * GDEF header (may be Null) */
  OffsetTo<AttachList>
		attachList;		/* Offset to list of glyphs with
					 * attachment points--from beginning
					 * of GDEF header (may be Null) */
  OffsetTo<LigCaretList>
		ligCaretList;		/* Offset to list of positioning points
					 * for ligature carets--from beginning
					 * of GDEF header (may be Null) */
  OffsetTo<ClassDef>
		markAttachClassDef;	/* Offset to class definition table for
					 * mark attachment type--from beginning
					 * of GDEF header (may be Null) */
  OffsetTo<MarkGlyphSets>
		markGlyphSetsDef[VAR];	/* Offset to the table of mark set
					 * definitions--from beginning of GDEF
					 * header (may be NULL).  Introduced
					 * in version 00010002. */
  public:
  DEFINE_SIZE_VAR (12, OffsetTo<MarkGlyphSets>);
};


#endif /* HB_OT_LAYOUT_GDEF_PRIVATE_HH */
