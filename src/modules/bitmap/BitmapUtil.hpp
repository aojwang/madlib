#ifndef MADLIB_MODULES_BITMAP_BITMAP_UTILITY_HPP
#define MADLIB_MODULES_BITMAP_BITMAP_UTILITY_HPP

#include <dbconnector/dbconnector.hpp>

#include "Bitmap_proto.hpp"
#include "Bitmap_impl.hpp"

namespace madlib {
namespace modules {
namespace bitmap {

using madlib::dbconnector::postgres::TypeTraits;
using madlib::dbconnector::postgres::madlib_get_typlenbyvalalign;

/**
 * @brief This class encapsulate the interfaces for manipulate the bitmap
 *        All functions are static.
 *
 */
class BitmapUtil{
protected:
    enum BITMAPOP {EQ = 0, GT = 1, LT = -1} ;
public:

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
 * @note the bitmap(UDT) uses integer array as underlying implementation. To
 *       achieve good performance, we will not converting between bitmap and
 *       integer array. Thus, we need to skip the type checks of the C++ AL.
 *          + the input argument is bitmap, we will get it as array.
 *          + the return value is bitmap, we will return it as array. Therefore,
 *            we will explicit set the OID of return value to InvalidOid, so that
 *            later the function getAsDatum will not check its type.
 *
 */
template<typename T>
static
AnyType
bitmap_agg_sfunc
(
    AnyType &args
){
    int64_t input_bit = args[1].getAs<int64_t>();
    int size_per_add = args[2].getAs<int32_t>();
    AnyType state = args[0];

    if (state.isNull()){
        Bitmap<T> bitmap(size_per_add, size_per_add);
        bitmap.insert(input_bit);
        return AnyType(bitmap.to_ArrayHandle(), InvalidOid);
    }
    // state is not null
    Bitmap<T> bitmap(state.getAs< MutableArrayHandle<T> >(false, false),
            size_per_add);
    bitmap.insert(input_bit);
    return bitmap.updated() ? AnyType(bitmap.to_ArrayHandle(), InvalidOid) :
            AnyType(state.getAs<ArrayHandle<T> >(false, false), InvalidOid);
}


/**
 * @brief the pre-function for the bitmap aggregation.
 *
 * @param args[0]   an array of the first state
 * @param args[1]   an array of the second state
 *
 * @return an array of merging the first state and the second state.
 * @note the bitmap(UDT) uses integer array as underlying implementation. To
 *       achieve good performance, we will not converting between bitmap and
 *       integer array. Thus, we need to skip the type checks of the C++ AL.
 *          + the input argument is bitmap, we will get it as array.
 *          + the return value is bitmap, we will return it as array. Therefore,
 *            we will explicit set the OID of return value to InvalidOid, so that
 *            later the function getAsDatum will not check its type.
 *
 */
template <typename T>
static
AnyType
bitmap_agg_pfunc
(
    AnyType &args
){
    AnyType states[] = {args[0], args[1]};

    int nonull_index = states[0].isNull() ?
            (states[1].isNull() ? -1 : 1) : (states[1].isNull() ? 0 : 2);

    // if the two states are all null, return one of them
    if (nonull_index < 0)
        return states[0];

    // one of the arguments is null
    // trim the zero elements in the non-null bitmap
    if (nonull_index < 2){
        // no need to increase the bitmap size
        Bitmap<T> bitmap(states[nonull_index].getAs< MutableArrayHandle<T> >(false, false), 0);
        if (bitmap.full()){
            return AnyType(states[nonull_index].getAs<ArrayHandle<T> >(false, false), InvalidOid);
        }
        return AnyType(bitmap.to_ArrayHandle(false), InvalidOid);
    }

    // all the arguments are not null
    // the two state-arrays can be written without copying it
    Bitmap<T> bitmap1(states[0].getAs< MutableArrayHandle<T> >(false, false), 0);
    Bitmap<T> bitmap2(states[1].getAs< MutableArrayHandle<T> >(false, false), 0);
    return AnyType(bitmap1 | bitmap2, InvalidOid);
}


/**
 * @brief The implementation of AND operation.
 *
 * @param args[0]   the first bitmap array
 * @param args[1]   the second bitmap array
 *
 * @return the result of args[0] AND args[1].
 * @note the bitmap(UDT) uses integer array as underlying implementation. To
 *       achieve good performance, we will not converting between bitmap and
 *       integer array. Thus, we need to skip the type checks of the C++ AL.
 *          + the input argument is bitmap, we will get it as array.
 *          + the return value is bitmap, we will return it as array. Therefore,
 *            we will explicit set the OID of return value to InvalidOid, so that
 *            later the function getAsDatum will not check its type.
 *
 */
template<typename T>
static
AnyType
bitmap_and
(
    AnyType &args
){
    Bitmap<T> bitmap1(args[0].getAs< MutableArrayHandle<T> >(false), 0);
    Bitmap<T> bitmap2(args[1].getAs< MutableArrayHandle<T> >(false), 0);

    return AnyType(bitmap1 & bitmap2, InvalidOid);
}


/**
 * @brief The implementation of OR operation.
 *
 * @param args[0]   the first bitmap array
 * @param args[1]   the second bitmap array
 *
 * @return the result of args[0] OR args[1].
 * @note the bitmap(UDT) uses integer array as underlying implementation. To
 *       achieve good performance, we will not converting between bitmap and
 *       integer array. Thus, we need to skip the type checks of the C++ AL.
 *          + the input argument is bitmap, we will get it as array.
 *          + the return value is bitmap, we will return it as array. Therefore,
 *            we will explicit set the OID of return value to InvalidOid, so that
 *            later the function getAsDatum will not check its type.
 *
 */
template<typename T>
static
AnyType
bitmap_or
(
    AnyType &args
){
    Bitmap<T> bitmap1(args[0].getAs< MutableArrayHandle<T> >(false), 0);
    Bitmap<T> bitmap2(args[1].getAs< MutableArrayHandle<T> >(false), 0);

    return AnyType(bitmap1 | bitmap2, InvalidOid);
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
static
AnyType
bitmap_nonzero_count
(
    AnyType &args
){
    // get the bitmap type as array
    Bitmap<T> bitmap(args[0].getAs< ArrayHandle<T> >(false), 0);

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
template <typename T>
static
AnyType
bitmap_nonzero_positions
(
    AnyType &args
){
    // get the bitmap as array
    Bitmap<T>bitmap(args[0].getAs< ArrayHandle<T> >(false), 0);

    return bitmap.nonzero_positions();
}


/**
 * @brief get the bitmap representation for the input array.
 *
 * @param args[0]   the input array.
 *
 * @return the bitmap for the input array.
 * @note T is the type of the bitmap
 *       the type of input array will be int64_t
 */
template <typename T>
static
AnyType
array_return_bitmap
(
    AnyType &args
){
    ArrayHandle<int64_t> handle = args[0].getAs< ArrayHandle<int64_t> >();
    const int64_t* array = handle.ptr();
    int size = handle.size();
    int bsize = (size >> 5) / 10;
    bsize = bsize < 2 ? 2 : bsize;
    Bitmap<T> bitmap(bsize, bsize);

    for (int i = 0; i < size; ++i){
        bitmap.insert(array[i]);
    }

    return AnyType(bitmap.to_ArrayHandle(false), InvalidOid);
}


/**
 * @brief the in function for the bitmap data type
 *
 * @param args[0]   the input string, which should be split by a comma.
 *
 * @return the bitmap representing the input string
 */
template <typename T>
static
AnyType
bitmap_in
(
    AnyType &args
){
    return AnyType((Bitmap<T>(args[0].getAs<char*>())).to_ArrayHandle(false),
            InvalidOid);
}


/**
 * @brief the out function for the bitmap data type.
 *
 * @param args[0]   the bitmap
 *
 * @return the string representing the bitmap.
 */
template <typename T>
static
AnyType
bitmap_out
(
    AnyType &args
){
    Bitmap<T> bitmap(args[0].getAs< ArrayHandle<T> >(false), 0);
    char* res = bitmap.to_string();
    return res ? AnyType(res) : AnyType();
}


/**
 * @brief convert the bitmap to varbit.
 *
 * @param args[0]   the bitmap
 *
 * @return the varbit representation for the bitmap.
 */
template <typename T>
static
AnyType
bitmap_return_varbit
(
    AnyType &args
){
    Bitmap<T> bitmap(args[0].getAs< ArrayHandle<T> >(false), 0);
    return AnyType(bitmap.to_varbit());
}


/**
 * @brief convert the bitmap to array.
 *
 * @param args[0]   the bitmap
 *
 * @return the array representation for the bitmap.
 */
template <typename T>
static
AnyType
bitmap_return_array
(
    AnyType &args
){
    return AnyType(args[0].getAs<ArrayHandle<T> >(false),
                (Oid)TypeTraits<ArrayHandle<T> >::oid);
}


public:
// comparators

/**
 * @brief the greater than (>) and less than (<) operators implementation
 *
 * @param args[0]   the bitmap array
 * @param args[1]   the bitmap array
 * @param is_gt     true if the invoker implements the > operator
 *                  false if the invoker implements the < operator
 *
 * @return is_gt ? args[0] > args[1] : args[0] < args[1]
 * @note currently, we will never use the > operator. Therefore, we just
 *       compare the length of the bitmap array for simplicity. This function
 *       will be used in the btree operator class.
 *
 */
template <typename T>
static
AnyType
bitmap_gt
(
    AnyType &args,
    bool is_gt = true
){
    BITMAPOP op = bitmap_cmp_internal<T>(args);
    return (is_gt ? op == GT : op == LT);
}


/**
 * @brief the >= and <= operators implementation
 *
 * @param args[0]   the bitmap array
 * @param args[1]   the bitmap array
 * @param is_ge     true if the invoker implements the >= operator
 *                  false if the invoker implements the < operator
 *
 * @return is_gt ? args[0] >= args[1] : args[0] <= args[1]
 * @note currently, we will never use the >= operator. Therefore, we just
 *       compare the length of the bitmap array for simplicity. This function
 *       will be used in the btree operator class.
 *
 */
template <typename T>
static
AnyType
bitmap_ge
(
    AnyType &args,
    bool is_ge = true
){
    BITMAPOP op = bitmap_cmp_internal<T>(args);
    return (is_ge ? op == GT : op == LT) || (op == EQ);
}


/**
 * @brief the = operator implementation
 *
 * @param args[0]   the bitmap array
 * @param args[1]   the bitmap array
 * @param is_eq     true if the invoker implements = operator
 *                  false if the invoker implements != operator
 *
 * @return is_eq ? args[0] == args[1] : !(args[0] == args[1])
 *
 */
template <typename T>
static
AnyType
bitmap_eq
(
    AnyType &args,
    bool is_eq = true
){

    // as the first element of bitmap is the size of the array
    // we don't need to care about the size of the array
    bool res = (bitmap_cmp_internal<T>(args) == EQ);
    return is_eq ? res : !res;
}


/**
 * @brief compare the two bitmap
 *
 * @param args[0]   the bitmap array
 * @param args[1]   the bitmap array
 *
 * @return 0 for equality; 1 for greater than; and -1 for less than
 *
 */
template <typename T>
static
AnyType
bitmap_cmp
(
    AnyType &args
){
    return static_cast<int32_t>(bitmap_cmp_internal<T>(args));
}

protected:
template <typename T>
static
BITMAPOP
bitmap_cmp_internal
(
    AnyType &args
){
    if (args[0].isNull() & args[1].isNull())
        return EQ;
    if (args[0].isNull() && !args[1].isNull())
        return LT;
    if (!args[0].isNull() && args[1].isNull())
        return GT;

    const T* lhs = (args[0].getAs< ArrayHandle<T> >(false)).ptr();
    const T* rhs = (args[1].getAs< ArrayHandle<T> >(false)).ptr();

    bool res = (0 == memcmp((const void*)lhs, (const void*)rhs,
                        lhs[0] * sizeof(T)));
    return res ? EQ :
            lhs[0] > rhs[0] ? GT : LT;
}

}; // class BitmapUtil

} // namespace bitmap
} // namespace modules
} // namespace madlib

#endif //  MADLIB_MODULES_BITMAP_BITMAP_PROTO_HPP
