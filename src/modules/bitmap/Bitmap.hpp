/*
 * Bitmap.hpp
 *
 * This file contains the interfaces for get bitmap from arguments and
 * return the bitmap as AnyType
 */

#ifndef MADLIB_MODULES_BITMAP_BITMAP_HPP
#define MADLIB_MODULES_BITMAP_BITMAP_HPP

// the public interfaces for bitmap
#define RETURN_BITMAP(val, T) \
            return AnyType(ArrayHandle<T>(val), InvalidOid)
#define RETURN_BITMAP_NULL(val, T) \
            const ArrayType* res = val; \
            return NULL != res ? AnyType(ArrayHandle<T>(res), InvalidOid) : AnyType()
#define RETURN_ARRAY(val, T) \
            return AnyType(ArrayHandle<T>(val))
#define RETURN_BASE(val) \
            return AnyType(val, (bool)false)
#define GETARG_MUTABLE_BITMAP(arg, T) \
            ((arg).getAs< MutableArrayHandle<T> >(false, false))
#define GETARG_CLONEABLE_BITMAP(arg, T) \
            ((arg).getAs< MutableArrayHandle<T> >(false, true))
#define GETARG_IMMUTABLE_BITMAP(arg, T) \
            ((arg).getAs< ArrayHandle<T> >(false, false))

// return
#define RETURN_BITMAP4(val)         RETURN_BITMAP(val, int32_t)
#define RETURN_BITMAP4_NULL(val)    RETURN_BITMAP_NULL(val, int32_t)
#define RETURN_BITMAP8(val)         RETURN_BITMAP(val, int64_t)
#define RETURN_BITMAP8_NULL(val)    RETURN_BITMAP_NULL(val, int64_t)
#define RETURN_INT4_ARRAY(val)      RETURN_ARRAY(val, int32_t)
#define RETURN_INT8_ARRAY(val)      RETURN_ARRAY(val, int64_t)

// get argument
#define GETARG_MUTABLE_BITMAP4(arg)     GETARG_MUTABLE_BITMAP(arg, int32_t)
#define GETARG_CLONEABLE_BITMAP4(arg)   GETARG_CLONEABLE_BITMAP(arg, int32_t)
#define GETARG_IMMUTABLE_BITMAP4(arg)   GETARG_IMMUTABLE_BITMAP(arg, int32_t)
#define GETARG_MUTABLE_BITMAP8(arg)     GETARG_MUTABLE_BITMAP(arg, int64_t)
#define GETARG_CLONEABLE_BITMAP8(arg)   GETARG_CLONEABLE_BITMAP(arg, int64_t)
#define GETARG_IMMUTABLE_BITMAP8(arg)   GETARG_IMMUTABLE_BITMAP(arg, int64_t)


#endif /* MADLIB_MODULES_BITMAP_BITMAP_HPP */
