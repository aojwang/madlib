#ifndef MADLIB_MODULES_BITMAP_BITMAP_PROTO_HPP
#define MADLIB_MODULES_BITMAP_BITMAP_PROTO_HPP

#include <dbconnector/dbconnector.hpp>

namespace madlib {
namespace modules {
namespace bitmap {

using madlib::dbconnector::postgres::madlib_get_typlenbyvalalign;

#define INT64FORMAT  "%lld"
#define MAXBITSOFINT64   25

// the following macros will be used to calculate the number of 1s in an integer
#define BM_POW(c, T) ((T)1<<(c))
#define BM_MASK(c, T) (((T)-1) / (BM_POW(BM_POW(c, T), T) + 1))
#define BM_ROUND(n, c, T) (((n) & BM_MASK(c, T)) + ((n) >> BM_POW(c, T) & BM_MASK(c, T)))

// align the 'val' with 'align' length
#define BM_ALIGN(val, align) (((val) + (align) - 1) / (align)) * (align)

// does the composite word represent continuous 1
#define BM_COMPWORD_ONE(val) (((val) & (m_wordcnt_mask + 1)) > 0)

// the two composite words are the same sign?
#define BM_SAME_SIGN(lhs, rhs) (((lhs) < 0) && ((rhs) < 0) && \
                               (0 == (((lhs) ^ (rhs)) & (m_wordcnt_mask + 1))))

// get the number of words for representing the input number
#define BM_NUMWORDS_FOR_BITS(val) (((val) + m_base - 1) / m_base)

// get the number of words in the composite word
#define BM_NUMWORDS_IN_COMP(val) ((val) & m_wordcnt_mask)

// get the maximum 0s or 1s can be represented by a composite word
// for bitmap4, its 0x3F FF FF FF.
#define BM_MAXBITS_IN_COMP ((int64_t)1 << (m_base - 1)) - 1

// the wrapper for palloc0, this function will not align the 'size'
#define BM_ALLOC0(size) palloc0(size)

// the madlib allocation function will align the 'size' to 16
#define BM_ALIGN_ALLOC0(size) madlib::defaultAllocator().allocate<\
        madlib::dbal::FunctionContext, \
        madlib::dbal::DoZero, \
        madlib::dbal::ThrowBadAlloc>(size)

// wrapper definition for ArrayType related functions
#define BM_ARR_DATA_PTR(val, T) reinterpret_cast<T*>(ARR_DATA_PTR(val))
#define BM_ARRAY_LENGTH(val) ArrayGetNItems(ARR_NDIM(val), ARR_DIMS(val))
#define BM_CONSTRUCT_ARRAY(result, size) \
    construct_array( result, size, m_typoid, m_typlen, m_typbyval, m_typalign);
#define BM_CONSTRUCT_ARRAY_TYPE(result, size, typoid, typlen, typbyval, typalign) \
    construct_array( result, size, typoid, typlen, typbyval, typalign);


/**
 * the class for building and manipulating the bitmap.
 * Here, we use an array to represent the bitmap. The first element of the array
 * keeps the real size of the bitmap. The reasons are that 1) in the bitmap_agg
 * step function, the capacity is always greater than the real size of the bitmap,
 * therefore, it would be better if we can get the real size of the bitmap quickly
 * rather than traverse the bitmap to get the real size; 2) one addition element
 * is small compared to the whole bitmap. An element in the array except the
 * first one is called a word whose size is equal to sizeof(T).
 * There are two type of words: normal word and composite word. The difference
 * between them is the highest bit. 0 for normal word and 1 for composite word.
 * Therefore, the value of a normal word is greater than zero and that of a
 * composite word is less than 0.

 * the bitmap array can be dynamically reallocated. If the real size of the
 * bitmap is equal to the capacity of the bitmap and we insert a bit to it,
 * then we will reallocate and copy the old bitmap elements to the new bitmap.
 * And we define a parameter "size_per_add" to indicate the number of empty
 * elements will be added to the bitmap.
 *
 * For example:
 *     bitmap[] = {4, 16, 0x80000003, 0xC0000002, 0, 0, 0, 0};
 * Therefore:
 *     the input numbers for the bitmap are (5, 125 ~ 186)
 *     the capacity of the bitmap is 8
 *     the real size of the bitmap is bitmap[0] = 4
 *     bitmap[1] is a normal word, which means the 5th bit is 1
 *     bitmap[2] is a composite word, which contains 3 normal words.
 *               All of which are 0
 *     bitmap[3] is a composite word, which contains 2 normal words.
 *               All of which are 1
 *     bitmap[4 ~ 7] are empty words, which means we can insert numbers to them.
 *     i.e. the input number is 189, then the bitmap[4] = 0x00000004.
 *
 */
template <typename T>
class Bitmap{
    // function pointers for bitwise operation
    typedef T (Bitmap<T>::*bitwise_op)(T, T);
    typedef int (Bitmap<T>::*bitwise_postproc)(T*, int, T*, int, int, T);

public:
    //ctor
    Bitmap(int capacity = 1, int size_per_add = 2);

    //ctor
    Bitmap(ArrayType* arr, Bitmap& rhs);

    //ctor
    Bitmap(ArrayHandle<T> handle, int size_per_add = 2);

    //ctor
    Bitmap(Bitmap& rhs);

    // construct a bitmap for the input string
    // the format of input string looks like "1,3,19,20".
    // we use comma to split the numbers
    Bitmap(char* rhs);

    // if the bitmap array was reallocated, the flag will be set to true
    inline bool updated(){
        return m_bitmap_updated;
    }

    // is the bitmap array full
    inline bool full(){
        return m_size == m_capacity;
    }

    inline bool empty(){
        return 1 == m_size;
    }
    // for easily access a word in the bitmap array
    inline T& operator [] (size_t i){
        return m_bitmap[i];
    }

    // insert a bit to the bitmap
    inline Bitmap& insert(int64_t bit_pos);

    // transform the bitmap to an ArrayType* instance
    inline ArrayType* to_ArrayType(bool use_capacity = true);

    // override the OR operation
    inline Bitmap operator | (Bitmap& rhs);

    // override the AND operation
    inline Bitmap operator & (Bitmap& rhs);

    // the same with operator |, but with different return type
    // to avoid the overhead of constructing bitmap object and ArrayHandle object,
    // this function will return ArrayType*
    inline ArrayType* op_or(Bitmap& rhs);

    // the same with operator &, but with different return type
    // to avoid the overhead of constructing bitmap object and ArrayHandle object,
    // this function will return ArrayType*
    inline ArrayType* op_and(Bitmap& rhs);

    // override operator()
    inline ArrayType* operator ()(bool use_capacity = true){
        return to_ArrayType(use_capacity);
    }

    // convert the bitmap to a readable format
    inline char* to_string();

    // convert the bitmap to varbit
    inline VarBit* to_varbit();

    // get the positions of the non-zero bits. The position starts from 1
    inline ArrayType* nonzero_positions();

    // get an array containing the positions of the nonzero bits
    // the input parameter should not be null
    inline int64_t nonzero_positions(int64_t* result);

    // get an array containing the positions of the nonzero bits
    // the function will allocate memory for holding the positions
    // the size of the array is passed by reference, the returned value
    // is the nonzero positions
    inline int64_t* nonzero_positions(int64_t& size);

    // get the number of nonzero bits
    inline int64_t nonzero_count();

protected:
    // breakup composite word, and insert the specified bit
    inline void breakup_compword (T* newbitmap, int index,
            int pos_in_word, int word_pos, int num_words);

    // insert a bit 1 to the composite word
    inline void insert_compword(int64_t bit_pos, int64_t num_words, int index);

    // append a bit 1 to the bitmap
    inline void append( int64_t bit_pos);

    // allocate a new array with ArrayType encapsulated
    // X is the type of the array, we need to search the type
    // related information from cache.
    template <typename X>
    inline ArrayType* alloc_array(X*& res, int size){
        Oid typoid = get_Oid((X)0);
        int16 typlen;
        bool typbyval;
        char typalign;

        madlib_get_typlenbyvalalign
                    (typoid, &typlen, &typbyval, &typalign);

        ArrayType* arr = BM_CONSTRUCT_ARRAY_TYPE(
                (Datum*)NULL, size, typoid, typlen, typbyval, typalign);
        res = BM_ARR_DATA_PTR(arr, X);

        return arr;
    }

    // allocate an array to keep the elements in 'oldarr'
    // the number of those elements is 'size'. The type of
    // the array is T.
    inline ArrayType* alloc_array(T* oldarr, int size){
        ArrayType* res = BM_CONSTRUCT_ARRAY((Datum*)NULL, size);
        memcpy(BM_ARR_DATA_PTR(res, T), oldarr, size * sizeof(T));

        return res;
    }

    // allocate a new bitmap, all elements are zeros
    inline T* alloc_bitmap(int size){
        m_bmArray = BM_CONSTRUCT_ARRAY((Datum*)NULL, size);
        return BM_ARR_DATA_PTR(m_bmArray, T);
    }

    // allocate a new bitmap, and copy the oldbitmap to the new bitmap
    inline T* alloc_bitmap(int newsize, T* oldbitmap, int oldsize){
        T* result = alloc_bitmap(newsize);
        memcpy(result, oldbitmap, oldsize * sizeof(T));

        return result;
    }

    // get the number of 1s
    inline int64_t get_nonzero_cnt(int32_t value){
        uint32_t res = value;
        res = BM_ROUND(res, 0, uint32_t);
        res = BM_ROUND(res, 1, uint32_t);
        res = BM_ROUND(res, 2, uint32_t);
        res = BM_ROUND(res, 3, uint32_t);
        res = BM_ROUND(res, 4, uint32);

        return static_cast<int64_t>(res);
    }

    // get the number of 1s
    inline int64_t get_nonzero_cnt(int64_t value){
        uint64 res = value;
        res = BM_ROUND(res, 0, uint64);
        res = BM_ROUND(res, 1, uint64);
        res = BM_ROUND(res, 2, uint64);
        res = BM_ROUND(res, 3, uint64);
        res = BM_ROUND(res, 4, uint64);
        res = BM_ROUND(res, 5, uint64);

        return static_cast<T>(res);
    }

    // get the inserting position for the input number.
    // The result is in range of [1, m_base]
    inline int get_pos_word(int64_t bit_pos){
        int pos = bit_pos % m_base;
        return 0 == pos  ? m_base : pos;
    }

    // transform the specified value to a Datum
    inline Datum get_Datum(int32_t elem){
        return Int32GetDatum(elem);
    }

    // transform the specified value to a Datum
    inline Datum get_Datum(int64_t elem){
        return Int64GetDatum(elem);
    }

    // get the OID for int32
    inline Oid get_Oid(int32_t){
        return INT4OID;
    }

    // get the OID for int64_t
    inline Oid get_Oid(int64_t){
        return INT8OID;
    }

    // set the type related information to the member variables
    inline void set_typInfo(){
        m_typoid = get_Oid((T)0);
        madlib_get_typlenbyvalalign
            (m_typoid, &m_typlen, &m_typbyval, &m_typalign);
    }

    // the entry function for doing the bitwise operations,
    // such as | and &, etc.
    ArrayType* bitwise_proc(Bitmap<T>& rhs, bitwise_op op,
            bitwise_postproc postproc);

    // lhs should not be a composite word
    inline T bitwise_or(T lhs, T rhs){
        T res = rhs > 0 ? lhs | rhs :
                        BM_COMPWORD_ONE(rhs) ?
                        m_sw_one_mask | 1 : lhs;
        // if all the bits of the result are 1, then use a composite word
        // to represent it
        return res == (~m_sw_zero_mask) ? m_sw_one_mask | 1 : res;
    }

    // lhs should not be a composite word
    inline T bitwise_and(T lhs, T rhs){
        T res = rhs > 0 ? lhs & rhs :
                        BM_COMPWORD_ONE(rhs) ?
                        lhs : m_sw_zero_mask | 1;
        // if all the bits of the result are 0, then use a composite word
        // to represent it
        return (0 == res) ? (m_sw_zero_mask | 1) : res;

    }

    // the post-processing for the OR operation. Here, we need to concat
    // the remainder bitmap elements to the result
    inline int or_postproc(T* result, int k, T* bitmap,
                            int i, int n, T pre_word){
        for (; i < n; ++i, ++k){
            T temp = (bitmap[i] < 0) ? bitmap[i] :
                        ((bitmap[i] == (~m_sw_zero_mask)) ?
                        (m_sw_one_mask | 1) : bitmap[i]);
            if (k >= 2 && BM_SAME_SIGN(temp, pre_word)){
                pre_word += BM_NUMWORDS_IN_COMP(temp);
                result[--k] = pre_word;
            }else{
                result[k] = bitmap[i];
                pre_word = bitmap[i];
            }
        }

        return k;
    }

    // the post-processing for the AND operation. Here, we need do nothing.
    inline int and_postproc(T*, int k, T*, int, int, T){
        return k;
    }

    // convert int64 number to a string
    int int64_to_string (char* result, int64_t value){
        int len = 0;
        if ((len = snprintf(result, MAXBITSOFINT64, INT64FORMAT, value)) < 0)
            throw std::invalid_argument("the input value is not int8 number");
        result[len] = '\0';

        return len;
    }

private:
    // the bitmap array related info
    ArrayType* m_bmArray;
    T* m_bitmap;
    int m_size;
    int m_capacity;
    int m_size_per_add;
    bool m_bitmap_updated;

    // const variables
    const int m_base;
    const T m_wordcnt_mask;
    const T m_sw_zero_mask;
    const T m_sw_one_mask;

    // type information
    Oid m_typoid;
    int16 m_typlen;
    bool m_typbyval;
    char m_typalign;

}; // class bitmap

} // namespace bitmap
} // namespace modules
} // namespace madlib

#endif
