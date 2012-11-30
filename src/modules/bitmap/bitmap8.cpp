#include <dbconnector/dbconnector.hpp>

#include "BitmapUtil.hpp"
#include "Bitmap.hpp"
#include "bitmap8.hpp"


namespace madlib {
namespace modules {
namespace bitmap {


/**
 * @brief the step function for the bitmap aggregate.
 */
AnyType
bitmap8_agg_sfunc::run(AnyType &args){
    RETURN_BITMAP8(BitmapUtil::bitmap_agg_sfunc<int64_t>(args));
}


/**
 * @brief the pre-function for the bitmap aggregate.
 */
AnyType
bitmap8_agg_pfunc::run(AnyType &args){
    RETURN_BITMAP8_NULL(BitmapUtil::bitmap_agg_pfunc<int64_t>(args));
}


/**
 * @brief the operator "AND" implementation
 */
AnyType
bitmap8_and::run(AnyType &args){
    RETURN_BITMAP8_NULL(BitmapUtil::bitmap_and<int64_t>(args));
}


/**
 * @brief the operator "OR" implementation
 */
AnyType
bitmap8_or::run(AnyType &args){
    RETURN_BITMAP8(BitmapUtil::bitmap_or<int64_t>(args));
}


/**
 * @brief get the number of bits whose value is 1.
 */
AnyType
bitmap8_nonzero_count::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_nonzero_count<int64_t>(args));
}


/**
 * @brief get the positions of the non-zero bits.
 */
AnyType
bitmap8_nonzero_positions::run(AnyType &args){
    RETURN_INT8_ARRAY(BitmapUtil::bitmap_nonzero_positions<int64_t>(args));
}


/**
 * @brief get the bitmap representation for int32 array
 */
AnyType
array_return_bitmap8::run(AnyType &args){
    RETURN_BITMAP8_NULL(BitmapUtil::array_return_bitmap<int64_t>(args));
}

/**
 * @brief the in function for the bitmap data type
 */
AnyType
bitmap8_in::run(AnyType &args){
    RETURN_BITMAP8(BitmapUtil::bitmap_in<int64_t>(args));
}


/**
 * @brief the out function for the bitmap data type
 */
AnyType
bitmap8_out::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_out<int64_t>(args));
}


/**
 * @brief get the varbit representation for the bitmap
 */
AnyType
bitmap8_return_varbit::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_return_varbit<int64_t>(args));
}


/**
 * @brief get an integer array from the bitmap.
 */
AnyType
bitmap8_return_array::run(AnyType &args){
    RETURN_INT8_ARRAY(BitmapUtil::bitmap_return_array<int64_t>(args));
}



/**
 * @brief the implementation of = operator
 */
AnyType
bitmap8_eq::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_eq<int64_t>(args));
}


/**
 * @brief the implementation of != operator
 */
AnyType
bitmap8_neq::run(AnyType &args){
    RETURN_BASE(!BitmapUtil::bitmap_eq<int64_t>(args));
}

/**
 * @brief the implementation of > operator
 */
AnyType
bitmap8_gt::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_gt<int64_t>(args));
}

/**
 * @brief the implementation of < operator
 */
AnyType
bitmap8_lt::run(AnyType &args){
    RETURN_BASE(!BitmapUtil::bitmap_ge<int64_t>(args));
}

/**
 * @brief the implementation of >= operator
 */
AnyType
bitmap8_ge::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_ge<int64_t>(args));
}


/**
 * @brief the implementation of <= operator
 */
AnyType
bitmap8_le::run(AnyType &args){
    RETURN_BASE(!BitmapUtil::bitmap_gt<int64_t>(args));
}

/**
 * @brief compare the two bitmaps
 */
AnyType
bitmap8_cmp::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_cmp<int64_t>(args));
}


} // namespace bitmap
} // namespace modules
} // namespace madlib
