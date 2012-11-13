/* ----------------------------------------------------------------------- *//**
 *
 * @file wah.hpp
 *
 *//* ----------------------------------------------------------------------- */

/**
 * @brief   Given the text form of a closed frequent pattern (cfp), this function
 *          generates the association rules for that pattern. We use text format
 *          because text values are hash joinable. The output is a set of text
 *          array. For example, assuming the input pattern is '1,2,3'.
 *          The result rules:
 *              array['1', '2,3']
 *              array['2', '1,3']
 *              array['3', '1,2']
 *              array['1,2', '3']
 *              array['1,3', '2']
 *              array['2,3', '1']
 *          Note that two meaningless rules will be excluded:
 *              array['1,2,3', NULL]
 *              array[NULL, '1,2,3']
 *
 * @param arg 1     The text form of a closed frequent pattern.
 * @param arg 2     The number of items in the pattern.
 *
 * @return  A set of text array. Each array has two elements, corresponding to
 *          the left and right parts of an association rule.
 *
 */

//DECLARE_UDF(bitmap, wah_in)
DECLARE_UDF(bitmap, bitmap64_agg_sfunc)
DECLARE_UDF(bitmap, bitmap64_agg_pfunc)
DECLARE_UDF(bitmap, bitmap32_agg_sfunc)
DECLARE_UDF(bitmap, bitmap32_agg_pfunc)
DECLARE_UDF(bitmap, bitmap32_and)
DECLARE_UDF(bitmap, bitmap32_or)
DECLARE_UDF(bitmap, bitmap64_or)
DECLARE_UDF(bitmap, bitmap64_and)
DECLARE_UDF(bitmap, bitmap32_nonzero_count)
DECLARE_UDF(bitmap, bitmap64_nonzero_count)
DECLARE_UDF(bitmap, bitmap32_nonzero_positions)
DECLARE_UDF(bitmap, bitmap64_nonzero_positions)






//DECLARE_UDF(bitmap, bitmap_agg_ffunc)
