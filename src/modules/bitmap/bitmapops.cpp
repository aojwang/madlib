#include <dbconnector/dbconnector.hpp>

#include "BitmapUtil.hpp"
#include "bitmapops.hpp"

namespace madlib {
namespace modules {
namespace bitmap {

/**
 * @brief the step function for the bitmap aggregate.
 */
AnyType
bitmap_agg_sfunc::run(AnyType &args){
    RETURN_BITMAP(BitmapUtil::bitmap_agg_sfunc<int32_t>(args));
}

/**
 * @brief the pre-function for the bitmap aggregate.
 */
AnyType
bitmap_agg_pfunc::run(AnyType &args){
    RETURN_BITMAP_NULL(BitmapUtil::bitmap_agg_pfunc<int32_t>(args));
}


/**
 * @brief the operator "AND" implementation
 */
AnyType
bitmap_and::run(AnyType &args){
    RETURN_BITMAP_NULL(BitmapUtil::bitmap_and<int32_t>(args));
}


/**
 * @brief the operator "OR" implementation
 */
AnyType
bitmap_or::run(AnyType &args){
    RETURN_BITMAP(BitmapUtil::bitmap_or<int32_t>(args));
}


/**
 * @brief get the number of bits whose value is 1.
 */
AnyType
bitmap_nonzero_count::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_nonzero_count<int32_t>(args));
}


/**
 * @brief get the positions of the non-zero bits.
 */
AnyType
bitmap_nonzero_positions::run(AnyType &args){
    RETURN_INT8_ARRAY(BitmapUtil::bitmap_nonzero_positions<int32_t>(args));
}


/**
 * @brief get the bitmap representation for int64 array
 */
AnyType
bitmap_from_int8array::run(AnyType &args){
    RETURN_BITMAP_NULL((BitmapUtil::bitmap_from_array<int32_t, int64_t>(args)));
}


/**
 * @brief get the bitmap representation for int32 array
 */
AnyType
bitmap_from_int4array::run(AnyType &args){
    RETURN_BITMAP_NULL((BitmapUtil::bitmap_from_array<int32_t, int32_t>(args)));
}


/**
 * @brief the in function for the bitmap data type
 */
AnyType
bitmap_in::run(AnyType &args){
    RETURN_BITMAP(BitmapUtil::bitmap_in<int32_t>(args));
}


/**
 * @brief the out function for the bitmap data type
 */
AnyType
bitmap_out::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_out<int32_t>(args));
}


/**
 * @brief get the varbit representation for the bitmap
 */
AnyType
bitmap_return_varbit::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_return_varbit<int32_t>(args));
}


/**
 * @brief get an integer array from the bitmap.
 */
AnyType
bitmap_return_array::run(AnyType &args){
    RETURN_INT4_ARRAY(BitmapUtil::bitmap_return_array<int32_t>(args));
}


/**
 * @brief the implementation of = operator
 */
AnyType
bitmap_eq::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_eq<int32_t>(args));
}


/**
 * @brief the implementation of != operator
 */
AnyType
bitmap_neq::run(AnyType &args){
    RETURN_BASE(!BitmapUtil::bitmap_eq<int32_t>(args));
}


/**
 * @brief the implementation of > operator
 */
AnyType
bitmap_gt::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_gt<int32_t>(args));
}


/**
 * @brief the implementation of < operator
 */
AnyType
bitmap_lt::run(AnyType &args){
    RETURN_BASE(!BitmapUtil::bitmap_ge<int32_t>(args));
}


/**
 * @brief the implementation of >= operator
 */
AnyType
bitmap_ge::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_ge<int32_t>(args));
}


/**
 * @brief the implementation of <= operator
 */
AnyType
bitmap_le::run(AnyType &args){
    RETURN_BASE(!BitmapUtil::bitmap_gt<int32_t>(args));
}


/**
 * @brief compare the two bitmaps
 */
AnyType
bitmap_cmp::run(AnyType &args){
    RETURN_BASE(BitmapUtil::bitmap_cmp<int32_t>(args));
}

} // bitmap
} // modules
} // madlib
