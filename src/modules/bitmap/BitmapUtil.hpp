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

#define INT64FORMAT  "%lld"
#define MAXBITSOFINT32   10
#define MAXBITSOFINT64   25
/**
 * @brief This class encapsulate the interface for manipulate the bitmap
 *        All functions are static.
 *
 */
class BitmapUtil{

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
 *          + the input argument is bitmap, we will get it as array. However,
 *            the types of bitmap and array are not the same, we will explicit
 *            set the argument OID to array in function getUDTAs
 *          + the return is bitmap, we will return it as array. Therefore, we
 *            will explicit set the OID of return value to InvalidOid, so that
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
    int64 input_bit = args[1].getAs<int64>();
    int size_per_add = args[2].getAs<int32>();
    Bitmap<T> bitmap;

    // the first time of entering the step function,
    // initialize the bitmap array
    AnyType state = args[0];

    if (state.isNull()){
        bitmap.init(size_per_add);
    }
    else{
        bitmap.init(state.getAs< MutableArrayHandle<T> >(true, false),
                size_per_add);
    }

    bitmap.insert(input_bit);

    if (bitmap.updated()){
        return AnyType(bitmap.to_ArrayHandle(), InvalidOid);
    }

    // no new memory was reallocated to the state array
    return AnyType(state.getAs<ArrayHandle<T> >(true), InvalidOid);
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
 *          + the input argument is bitmap, we will get it as array. However,
 *            the types of bitmap and array are not the same, we will explicit
 *            set the argument OID to array in function getUDTAs
 *          + the return is bitmap, we will return it as array. Therefore, we
 *            will explicit set the OID of return value to InvalidOid, so that
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
        Bitmap<T> bitmap(states[nonull_index].getAs< MutableArrayHandle<T> >(true, false), 0);
        if (bitmap.full()){
            return AnyType(states[nonull_index].getAs<ArrayHandle<T> >(true), InvalidOid);
        }
        return AnyType(bitmap.to_ArrayHandle(false), InvalidOid);
    }

    // all the arguments are not null
    // the two state-arrays can be written without copying it
    Bitmap<T> bitmap1(states[0].getAs< MutableArrayHandle<T> >(true, false), 0);
    Bitmap<T> bitmap2(states[1].getAs< MutableArrayHandle<T> >(true, false), 0);
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
 *          + the input argument is bitmap, we will get it as array. However,
 *            the types of bitmap and array are not the same, we will explicit
 *            set the argument OID to array in function getUDTAs
 *          + the return is bitmap, we will return it as array. Therefore, we
 *            will explicit set the OID of return value to InvalidOid, so that
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
    Bitmap<T> bitmap1(args[0].getAs< MutableArrayHandle<T> >(true), 0);
    Bitmap<T> bitmap2(args[1].getAs< MutableArrayHandle<T> >(true), 0);

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
 *          + the input argument is bitmap, we will get it as array. However,
 *            the types of bitmap and array are not the same, we will explicit
 *            set the argument OID to array in function getUDTAs
 *          + the return is bitmap, we will return it as array. Therefore, we
 *            will explicit set the OID of return value to InvalidOid, so that
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
    Bitmap<T> bitmap1(args[0].getAs< MutableArrayHandle<T> >(true), 0);
    Bitmap<T> bitmap2(args[1].getAs< MutableArrayHandle<T> >(true), 0);

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
    Bitmap<T> bitmap(args[0].getAs< ArrayHandle<T> >(true, false), 0);

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
static
AnyType
bitmap_nonzero_positions
(
    AnyType &args
){
    // get the bitmap as array
    Bitmap<T>bitmap(args[0].getAs< ArrayHandle<T> >(true, false), 0);

    return bitmap.nonzero_positions();
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
    char* input = args[0].getAs<char*>();
    Bitmap<T> bitmap(32, 32);
    char elem[24] = {'\0'};
    int j = 0;
    int64 input_pos = 0;

    for (; *input != '\0'; ++input){
        if (',' == *input){
            elem[j] = '\0';
            (void) scanint8(elem, false, &input_pos);
            bitmap.insert(input_pos);
            j = 0;
        }else{
            elem[j++] = *input;
        }
    }
    (void) scanint8(elem, false, &input_pos);
    bitmap.insert(input_pos);

    return AnyType(bitmap.to_ArrayHandle(), InvalidOid);
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
    AnyType arg = args[0];
    ArrayHandle<T> array = arg.getAs< ArrayHandle<T> >(true, false);
    int size = array[0];
    bool is_bit32 = sizeof(T) == sizeof(int32);
    char *res = (char*)palloc0(size *
            (is_bit32 ? (MAXBITSOFINT32 + 1) : (MAXBITSOFINT64 + 1)) *
            sizeof(char));

    char *res_begin = res;
    char temp[MAXBITSOFINT64 + 1] = {'\0'};
    char* temp2;

    for (int i = 1; i < size; ++i){
        temp2 = temp;
        temp2 = is_bit32 ? int32_to_string(array[i], temp2) :
                int64_to_string(array[i], temp2);

        while (*temp2 != '\0'){
            *res++ = *temp2++;
        }
        // replace the last '\0' to ','
        *res++ = ',';
    }

    // remove the last comma
    *res = '\0';
    return AnyType(res_begin);
}

protected:
static
char* int32_to_string(int32 value, char* result){
    pg_ltoa(value, result);
    return result;
}

static
char* int64_to_string(int64 value, char* result){
    int len = 0;
    if ((len = snprintf(result, MAXBITSOFINT64, INT64FORMAT, value)) < 0)
        elog(ERROR, "could not format int8");
    result[len] = '\0';
    return result;
}
}; // class BitmapUtil

} // namespace bitmap
} // namespace modules
} // namespace madlib

#endif //  MADLIB_MODULES_BITMAP_BITMAP_PROTO_HPP
