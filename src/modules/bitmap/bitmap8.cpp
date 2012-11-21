#include <dbconnector/dbconnector.hpp>

#include "BitmapUtil.hpp"
#include "bitmap8.hpp"


namespace madlib {
namespace modules {
namespace bitmap {


/**
 * @brief the step function for the bitmap aggregate.
 */
AnyType
bitmap8_agg_sfunc::run(AnyType &args){
   return BitmapUtil::bitmap_agg_sfunc<int64>(args);
}


/**
 * @brief the pre-function for the bitmap aggregate.
 */
AnyType
bitmap8_agg_pfunc::run(AnyType &args){
    return BitmapUtil::bitmap_agg_pfunc<int64>(args);
}


/**
 * @brief the operator "AND" implementation
 */
AnyType
bitmap8_and::run(AnyType &args){
    return BitmapUtil::bitmap_and<int64>(args);
}


/**
 * @brief the operator "OR" implementation
 */
AnyType
bitmap8_or::run(AnyType &args){
    return BitmapUtil::bitmap_or<int64>(args);
}


/**
 * @brief get the number of bits whose value is 1.
 */
AnyType
bitmap8_nonzero_count::run(AnyType &args){
    return BitmapUtil::bitmap_nonzero_count<int64>(args);
}


/**
 * @brief get the positions of the non-zero bits.
 */
AnyType
bitmap8_nonzero_positions::run(AnyType &args){
    return BitmapUtil::bitmap_nonzero_positions<int64>(args);
}


/**
 * @brief the in function for the bitmap data type
 */
AnyType
bitmap8_in::run(AnyType &args){
    return BitmapUtil::bitmap_in<int64>(args);
}


/**
 * @brief the out function for the bitmap data type
 */
AnyType
bitmap8_out::run(AnyType &args){
    return BitmapUtil::bitmap_out<int64>(args);
}


/**
 * @brief get an integer array from the bitmap.
 */
AnyType
bitmap8_return_array::run(AnyType &args){
    return AnyType(args[0].getAs<ArrayHandle<int64> >(true),
            (Oid)TypeTraits<ArrayHandle<int64> >::oid);
}


/**
 * @brief get the bitmap representation for int32 array
 */
AnyType
array_return_bitmap8::run(AnyType &args){
    return BitmapUtil::array_return_bitmap<int64>(args);
}


/**
 * @brief the implementation of = operator
 */
AnyType
bitmap8_eq::run(AnyType &args){
    return BitmapUtil::bitmap_eq<int64>(args, true);
}


/**
 * @brief the implementation of != operator
 */
AnyType
bitmap8_neq::run(AnyType &args){
    return BitmapUtil::bitmap_eq<int64>(args, false);
}

/**
 * @brief the implementation of > operator
 */
AnyType
bitmap8_gt::run(AnyType &args){
    return BitmapUtil::bitmap_gt<int64>(args, true);
}

/**
 * @brief the implementation of < operator
 */
AnyType
bitmap8_lt::run(AnyType &args){
    return BitmapUtil::bitmap_gt<int64>(args, false);
}

/**
 * @brief the implementation of >= operator
 */
AnyType
bitmap8_ge::run(AnyType &args){
    return BitmapUtil::bitmap_ge<int64>(args, true);
}


/**
 * @brief the implementation of <= operator
 */
AnyType
bitmap8_le::run(AnyType &args){
    return BitmapUtil::bitmap_ge<int64>(args, false);
}

/**
 * @brief compare the two bitmaps
 */
AnyType
bitmap8_cmp::run(AnyType &args){
    return BitmapUtil::bitmap_cmp<int64>(args);
}


} // namespace bitmap
} // namespace modules
} // namespace madlib
