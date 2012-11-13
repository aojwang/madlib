#include <dbconnector/dbconnector.hpp>

#include "bitmap.hpp"
#include "Bitmap_proto.hpp"
#include "Bitmap_impl.hpp"

namespace madlib {
namespace modules {
namespace bitmap {

/**
 * @brief the step function for aggregating the input numbers to
 * a compressed bitmap.
 *
 * @param args[0]   an array indicating the state
 * @param args[1]   the input number
 * @param args[2]   the number of empty elements will be dynamically
 *                  added to the state array
 *
 * @return an array indicating the state after inserted the input number.
 *
 */
template<typename T>
AnyType bitmap_agg_sfunc(AnyType &args){
    int64 input_bit = args[1].getAs<int64>();
    int size_per_add = args[2].getAs<int32>();
    Bitmap<T> bitmap;

    // the first time of entering the step function,
    // initialize the bitmap array
    if (args[0].isNull()){
        bitmap.init(size_per_add);
    }
    else{
        // the state should be written without copying it
        args[0].setMutable(true);
        bitmap.init(args[0].getAs< MutableArrayHandle<T> >(), size_per_add);
    }

    bitmap.insert(input_bit);

    if (bitmap.updated()){
        return bitmap.to_ArrayHandle();
    }

    // no new memory was reallocated to the state array
    return args[0];
}


/**
 * @brief the pre-function for the bitmap aggregation.
 *
 * @param args[0]   an array of the first state
 * @param args[1]   an array of the second state
 *
 * @return an array of merging the first state and the second state.
 */
template <typename T>
AnyType
bitmap_agg_pfunc(AnyType &args){
    int nonull_index = args[0].isNull() ?
            (args[1].isNull() ? -1 : 1) : (args[1].isNull() ? 0 : 2);

    // if the two states are all null, return one of them
    if (nonull_index < 0)
        return args[0];

    // one of the arguments is null
    // trim the zero elements in the non-null bitmap
    if (nonull_index < 2){
        args[nonull_index].setMutable(true);
        // no need to increase the bitmap size
        Bitmap<T> bitmap(args[nonull_index].getAs< MutableArrayHandle<T> >(), 0);
        if (bitmap.full()){
            return args[nonull_index];
        }

        return bitmap.to_ArrayHandle(false);
    }

    // all the arguments are not null
    // the two state-arrays can be written without copying it
    args[0].setMutable(true);
    args[1].setMutable(true);
    Bitmap<T> bitmap1(args[0].getAs< MutableArrayHandle<T> >(), 0);
    Bitmap<T> bitmap2(args[1].getAs< MutableArrayHandle<T> >(), 0);

    return bitmap1 | bitmap2;

}


/**
 * @brief The implementation of AND operation.
 *
 * @param args[0]   the first bitmap array
 * @param args[1]   the second bitmap array
 *
 * @return the result of args[0] AND args[1].
 *
 */
template<typename T>
AnyType bitmap_and(AnyType &args){
    Bitmap<T> bitmap1(args[0].getAs< MutableArrayHandle<T> >(), 0);
    Bitmap<T> bitmap2(args[1].getAs< MutableArrayHandle<T> >(), 0);

    return bitmap1 & bitmap2;
}


/**
 * @brief The implementation of OR operation.
 *
 * @param args[0]   the first bitmap array
 * @param args[1]   the second bitmap array
 *
 * @return the result of args[0] OR args[1].
 *
 */
template<typename T>
AnyType bitmap_or(AnyType &args){
    Bitmap<T> bitmap1(args[0].getAs< MutableArrayHandle<T> >(), 0);
    Bitmap<T> bitmap2(args[1].getAs< MutableArrayHandle<T> >(), 0);

    return bitmap1 | bitmap2;
}


/**
 * @brief get the number of bits whose value is 1.
 *
 * @param args[0]   the bitmap array
 *
 * @return the count of non-zero bits in the bitmap.
 *
 */
template<typename T>
AnyType bitmap_nonzero_count(AnyType &args){
    Bitmap<T> bitmap(args[0].getAs< MutableArrayHandle<T> >(), 0);

    return bitmap.nonzero_count();
}


/**
 * @brief get the positions of the non-zero bits
 *
 * @param args[0]   the bitmap array
 *
 * @return the array contains the positions of the non-zero bits.
 * @note the position starts from 1.
 *
 */
template<typename T>
AnyType bitmap_nonzero_positions(AnyType &args){
    // ensure one time copy for each segment
    Bitmap<T>bitmap(args[0].getAs< MutableArrayHandle<T> >(), 0);

    return bitmap.nonzero_positions();
}



/**
 * @brief the step function for the bitmap aggregate.
 */
AnyType
bitmap32_agg_sfunc::run(AnyType &args){
   return bitmap_agg_sfunc<int32>(args);
}

AnyType
bitmap32_agg_pfunc::run(AnyType &args){
    return bitmap_agg_pfunc<int32>(args);
}

AnyType
bitmap32_and::run(AnyType &args){
    return bitmap_and<int32>(args);
}

AnyType
bitmap32_or::run(AnyType &args){
    return bitmap_or<int32>(args);
}

AnyType
bitmap32_nonzero_count::run(AnyType &args){
    return bitmap_nonzero_count<int32>(args);
}

AnyType
bitmap32_nonzero_positions::run(AnyType &args){
    return bitmap_nonzero_positions<int32>(args);
}

AnyType
bitmap64_agg_sfunc::run(AnyType &args){
   return bitmap_agg_sfunc<int64>(args);
}

AnyType
bitmap64_agg_pfunc::run(AnyType &args){
    return bitmap_agg_pfunc<int64>(args);
}

AnyType
bitmap64_and::run(AnyType &args){
    return bitmap_and<int64>(args);
}

AnyType
bitmap64_or::run(AnyType &args){
    return bitmap_or<int64>(args);
}

AnyType
bitmap64_nonzero_count::run(AnyType &args){
    return bitmap_nonzero_count<int64>(args);
}

AnyType
bitmap64_nonzero_positions::run(AnyType &args){
    return bitmap_nonzero_positions<int64>(args);
}
} // bitmap
} // modules
} // madlib
