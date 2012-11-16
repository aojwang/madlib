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
   return BitmapUtil::bitmap_agg_sfunc<int32>(args);
}

/**
 * @brief the pre-function for the bitmap aggregate.
 */
AnyType
bitmap4_agg_pfunc::run(AnyType &args){
    return BitmapUtil::bitmap_agg_pfunc<int32>(args);
}


/**
 * @brief the operator "AND" implementation
 */
AnyType
bitmap4_and::run(AnyType &args){
    return BitmapUtil::bitmap_and<int32>(args);
}


/**
 * @brief the operator "OR" implementation
 */
AnyType
bitmap4_or::run(AnyType &args){
    return BitmapUtil::bitmap_or<int32>(args);
}


/**
 * @brief get the number of bits whose value is 1.
 */
AnyType
bitmap4_nonzero_count::run(AnyType &args){
    return BitmapUtil::bitmap_nonzero_count<int32>(args);
}


/**
 * @brief get the positions of the non-zero bits.
 */
AnyType
bitmap4_nonzero_positions::run(AnyType &args){
    return BitmapUtil::bitmap_nonzero_positions<int32>(args);
}


/**
 * @brief the in function for the bitmap data type
 */
AnyType
bitmap4_in::run(AnyType &args){
    return BitmapUtil::bitmap_in<int32>(args);
}


/**
 * @brief the out function for the bitmap data type
 */
AnyType
bitmap4_out::run(AnyType &args){
    return BitmapUtil::bitmap_out<int32>(args);
}


/**
 * @brief get an integer array from the bitmap.
 */
AnyType
bitmap4_return_array::run(AnyType &args){
    return AnyType(args[0].getAs<ArrayHandle<int32> >(true),
            (Oid)TypeTraits<ArrayHandle<int32> >::oid);
}

} // bitmap
} // modules
} // madlib
