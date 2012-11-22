#include <dbconnector/dbconnector.hpp>

#include "BitmapUtil.hpp"
#include "bitmap4.hpp"

namespace madlib {
namespace modules {
namespace bitmap {

/**
 * @brief the step function for the bitmap aggregate.
 */
AnyType
bitmap4_agg_sfunc::run(AnyType &args){
   return BitmapUtil::bitmap_agg_sfunc<int32_t>(args);
}

/**
 * @brief the pre-function for the bitmap aggregate.
 */
AnyType
bitmap4_agg_pfunc::run(AnyType &args){
    return BitmapUtil::bitmap_agg_pfunc<int32_t>(args);
}


/**
 * @brief the operator "AND" implementation
 */
AnyType
bitmap4_and::run(AnyType &args){
    return BitmapUtil::bitmap_and<int32_t>(args);
}


/**
 * @brief the operator "OR" implementation
 */
AnyType
bitmap4_or::run(AnyType &args){
    return BitmapUtil::bitmap_or<int32_t>(args);
}


/**
 * @brief get the number of bits whose value is 1.
 */
AnyType
bitmap4_nonzero_count::run(AnyType &args){
    return BitmapUtil::bitmap_nonzero_count<int32_t>(args);
}


/**
 * @brief get the positions of the non-zero bits.
 */
AnyType
bitmap4_nonzero_positions::run(AnyType &args){
    return BitmapUtil::bitmap_nonzero_positions<int32_t>(args);
}


/**
 * @brief the in function for the bitmap data type
 */
AnyType
bitmap4_in::run(AnyType &args){
    return BitmapUtil::bitmap_in<int32_t>(args);
}


/**
 * @brief the out function for the bitmap data type
 */
AnyType
bitmap4_out::run(AnyType &args){
    return BitmapUtil::bitmap_out<int32_t>(args);
}


/**
 * @brief get an integer array from the bitmap.
 */
AnyType
bitmap4_return_array::run(AnyType &args){
    return AnyType(args[0].getAs<ArrayHandle<int32_t> >(true),
            (Oid)TypeTraits<ArrayHandle<int32_t> >::oid);
}


/**
 * @brief get the bitmap representation for int64 array
 */
AnyType
array_return_bitmap4::run(AnyType &args){
    return BitmapUtil::array_return_bitmap<int32_t>(args);
}


/**
 * @brief the implementation of = operator
 */
AnyType
bitmap4_eq::run(AnyType &args){
    return BitmapUtil::bitmap_eq<int32_t>(args, true);
}


/**
 * @brief the implementation of != operator
 */
AnyType
bitmap4_neq::run(AnyType &args){
    return BitmapUtil::bitmap_eq<int32_t>(args, false);
}

/**
 * @brief the implementation of > operator
 */
AnyType
bitmap4_gt::run(AnyType &args){
    return BitmapUtil::bitmap_gt<int32_t>(args, true);
}

/**
 * @brief the implementation of < operator
 */
AnyType
bitmap4_lt::run(AnyType &args){
    return BitmapUtil::bitmap_gt<int32_t>(args, false);
}

/**
 * @brief the implementation of >= operator
 */
AnyType
bitmap4_ge::run(AnyType &args){
    return BitmapUtil::bitmap_ge<int32_t>(args, true);
}


/**
 * @brief the implementation of <= operator
 */
AnyType
bitmap4_le::run(AnyType &args){
    return BitmapUtil::bitmap_ge<int32_t>(args, false);
}

/**
 * @brief compare the two bitmaps
 */
AnyType
bitmap4_cmp::run(AnyType &args){
    return BitmapUtil::bitmap_cmp<int32_t>(args);
}

} // bitmap
} // modules
} // madlib
