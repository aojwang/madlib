/*
 * Bitmap.hpp
 *
 * This file contains the interfaces for get bitmap from arguments and
 * return the bitmap as AnyType
 */

#ifndef MADLIB_MODULES_BITMAP_BITMAP_HPP
#define MADLIB_MODULES_BITMAP_BITMAP_HPP

// the public interfaces for bitmap
#define RETURN_BITMAP_INTERNAL(val, T) \
            return AnyType(ArrayHandle<T>(val), InvalidOid)
#define RETURN_BITMAP_NULL_INTERNAL(val, T) \
            const ArrayType* res = val; \
            return NULL != res ? AnyType(ArrayHandle<T>(res), InvalidOid) : AnyType()
#define RETURN_ARRAY(val, T) \
            return AnyType(ArrayHandle<T>(val))
#define RETURN_BASE(val) \
            return AnyType(val, (bool)false)
#define GETARG_MUTABLE_BITMAP_INTERNAL(arg, T) \
            ((arg).getAs< MutableArrayHandle<T> >(false, false))
#define GETARG_CLONED_BITMAP_INTERNAL(arg, T) \
            ((arg).getAs< MutableArrayHandle<T> >(false, true))
#define GETARG_IMMUTABLE_BITMAP_INTERNAL(arg, T) \
            ((arg).getAs< ArrayHandle<T> >(false, false))

// return
#define RETURN_BITMAP(val)             RETURN_BITMAP_INTERNAL(val, int32_t)
#define RETURN_BITMAP_NULL(val)        RETURN_BITMAP_NULL_INTERNAL(val, int32_t)
#define RETURN_INT4_ARRAY(val)         RETURN_ARRAY(val, int32_t)
#define RETURN_INT8_ARRAY(val)         RETURN_ARRAY(val, int64_t)

// get arguments
#define GETARG_MUTABLE_BITMAP(arg)     GETARG_MUTABLE_BITMAP_INTERNAL(arg, int32_t)
#define GETARG_CLONED_BITMAP(arg)      GETARG_CLONED_BITMAP_INTERNAL(arg, int32_t)
#define GETARG_IMMUTABLE_BITMAP(arg)   GETARG_IMMUTABLE_BITMAP_INTERNAL(arg, int32_t)


#endif /* MADLIB_MODULES_BITMAP_BITMAP_HPP */
