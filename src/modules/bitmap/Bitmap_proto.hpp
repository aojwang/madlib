#ifndef MADLIB_MODULES_BITMAP_BITMAP_PROTO_HPP
#define MADLIB_MODULES_BITMAP_BITMAP_PROTO_HPP

#include <dbconnector/dbconnector.hpp>

namespace madlib {
namespace modules {
namespace bitmap {

using madlib::dbconnector::postgres::madlib_get_typlenbyvalalign;

// the following macros will be used to calculate the number of 1s in an integer
#define BM_POW(c, T) ((T)1<<(c))
#define BM_MASK(c, T) (((T)-1) / (BM_POW(BM_POW(c, T), T) + 1))
#define BM_ROUND(n, c, T) (((n) & BM_MASK(c, T)) + ((n) >> BM_POW(c, T) & BM_MASK(c, T)))


/**
 * the class for building and manipulating the bitmap.
 * Here, we use an array to represent the bitmap. The first element of the array
 * keeps the real size of the bitmap. An element in the array is
 * called a word whose size is equal to sizeof(T). There are two type of words:
 * normal word and composite word. The difference between them is the highest bit.
 * 0 for normal word and 1 for composite word. Therefore, the value of a normal
 * word is greater than zero and that of a composite word is less than 0. One thing
 * should be noted is that 0 is empty word.
 *
 * the bitmap array can be dynamically reallocated. If the real size of the
 * bitmap is equal to the capacity of the bitmap, then we will reallocate and
 * copy the old bitmap elements to the new bitmap. And we define a parameter
 * "size_per_add" to indicate the number of empty elements will be added to
 * the bitmap.
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
    // function pointers
    typedef T (Bitmap<T>::*bitwise_op)(T, T);
    typedef int (Bitmap<T>::*bitwise_postproc)(Datum*, int, T*, int, int);

public:
    //ctor
    Bitmap(T* bitmap, int size, int capacity, int size_per_add) :
        m_bitmap(bitmap), m_size(size), m_capacity(capacity),
        m_size_per_add(size_per_add), m_bitmap_updated(false),
        m_base(sizeof(T) * 8 - 1),
        m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
        m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
        m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)){
        set_typInfo();
    }

    //ctor
    Bitmap() :
        m_bitmap(NULL), m_size(0), m_capacity(0),
        m_size_per_add(0), m_bitmap_updated(false),
        m_base(sizeof(T) * 8 - 1),
        m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
        m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
        m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)){
        set_typInfo();
    }

    //ctor
    Bitmap(int capacity, int size_per_add) :
        m_bitmap(NULL), m_size(1), m_capacity(capacity),
        m_size_per_add(size_per_add), m_bitmap_updated(false),
        m_base(sizeof(T) * 8 - 1),
        m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
        m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
        m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)){

        m_bitmap = new T[capacity];
        memset(m_bitmap, 0x00, capacity * sizeof(T));
        m_bitmap[0] = 1;
        set_typInfo();
    }

    //ctor
    Bitmap(ArrayHandle<T> handle, int size_per_add) :
        m_bitmap(const_cast<T*>(handle.ptr())), m_size(handle[0]), m_capacity(handle.size()),
        m_size_per_add(size_per_add), m_bitmap_updated(false),
        m_base(sizeof(T) * 8 - 1),
        m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
        m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
        m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)) {
        set_typInfo();
    }

    //ctor
    Bitmap(Bitmap& rhs):
        m_bitmap(rhs.m_bitmap), m_size(rhs.m_size), m_capacity(rhs.m_capacity),
        m_size_per_add(rhs.m_size_per_add), m_bitmap_updated(rhs.m_bitmap_updated),
        m_base(sizeof(T) * 8 - 1),
        m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
        m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
        m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)),
        m_typoid(rhs.m_typoid), m_typlen(rhs.m_typelen),
        m_typbyval(rhs.m_typbyval), m_typalign(rhs.m_typalign){
    }

    // initialize the member variables. constant variables and type related
    // information will be initialized in the constructor
    inline void init(MutableArrayHandle<T> handle, int size_per_add){
        m_bitmap = handle.ptr();
        m_size = handle[0];
        m_capacity = handle.size();
        m_size_per_add = size_per_add;
        m_bitmap_updated = false;
    }

    // allocate memory for the bitmap array and initialize the member variables
    inline void init(int size_per_add){
        m_size = 1;
        m_capacity = size_per_add;
        m_size_per_add = size_per_add;
        m_bitmap = new T[m_capacity];
        memset(m_bitmap, 0x00, m_capacity * sizeof(T));
        m_bitmap[0] = 1;
        m_bitmap_updated = true;
    }

    // if the bitmap array was reallocated, the flag will be set to true
    inline bool updated(){
        return m_bitmap_updated;
    }

    // is the bitmap array was full
    inline bool full(){
        return m_size == m_capacity;
    }

    // for easily access a word in the bitmap array
    inline T& operator [] (size_t i){
        return m_bitmap[i];
    }

    // insert a bit to the bitmap
    inline Bitmap& insert(int64 bit_pos);

    // transform the bitmap to an ArrayHandle instance
    inline ArrayHandle<T> to_ArrayHandle(bool use_capacity = true);

    inline Datum to_PointerDatum(bool use_capacity = true);

    // override the OR operation
    inline ArrayHandle<T> operator | (Bitmap& rhs);

    // override the AND operation
    inline ArrayHandle<T> operator & (Bitmap& rhs);

    // get the positions of the non-zero bits. The position starts from 1
    inline ArrayHandle<T> nonzero_positions();
    inline T nonzero_count(){
        T   res = 0;
        for (int i = 1; i < m_size; ++i){
            if (m_bitmap[i] > 0){
                res += get_nonzero_cnt(m_bitmap[i]);
            }else{
                // this is a composite word
                res += (0 == (m_bitmap[i] & (m_wordcnt_mask + 1))) ?
                        0 : (m_bitmap[i] & m_wordcnt_mask) * m_base;
            }
        }

        return res;
    }

protected:
    // get the number of 1s
    inline T get_nonzero_cnt(int32 value){
        uint32 res = value;
        res = BM_ROUND(res, 0, uint32);
        res = BM_ROUND(res, 1, uint32);
        res = BM_ROUND(res, 2, uint32);
        res = BM_ROUND(res, 3, uint32);
        res = BM_ROUND(res, 4, uint32);

        return static_cast<T>(res);
    }

    // get the number of 1s
    inline T get_nonzero_cnt(int64 value){
        uint64 res = value;
        res = BM_ROUND(res, 0, uint64);
        res = BM_ROUND(res, 1, uint64);
        res = BM_ROUND(res, 2, uint64);
        res = BM_ROUND(res, 3, uint64);
        res = BM_ROUND(res, 4, uint64);
        res = BM_ROUND(res, 5, uint64);

        return static_cast<T>(res);
    }

    // get the position in the bitmap for the input number.
    // The result is in range of [1, m_base]
    inline int get_pos_word(T bit_pos){
        int pos = bit_pos % m_base;
        return 0 == pos  ? m_base : pos;
    }

    // get the number of words for representing the input number
    inline int get_num_words(T bit_pos){
        return (bit_pos + m_base - 1) / m_base;
    }

    // transform the specified value to a Datum
    inline Datum get_Datum(int32 elem){
        return Int32GetDatum(elem);
    }

    // transform the specified value to a Datum
    inline Datum get_Datum(int64 elem){
        return Int64GetDatum(elem);
    }

    // get the value from the given Datum
    inline Datum get_value(int32 elem){
        return Int32GetDatum(elem);
    }

    // get the value from the given Datum
    inline Datum get_value(int64 elem){
        return Int64GetDatum(elem);
    }

    // get the OID for int32
    inline Oid get_Oid(int32){
        return INT4OID;
    }

    // get the OID for int64
    inline Oid get_Oid(int64){
        return INT8OID;
    }

    // set the type related information to the member variables
    inline void set_typInfo(){
        m_typoid = get_Oid((T)0);
        madlib_get_typlenbyvalalign
            (m_typoid, &m_typlen, &m_typbyval, &m_typalign);
    }

    // insert a number to a composite word
    Bitmap& insert_compositeword (T* newbitmap, int index,
            int pos_in_word, int word_pos, int num_words);

    // the entry function for doing the bitwise operations,
    // such as | and &, etc.
    ArrayHandle<T> bitwise_proc(Bitmap<T>& rhs, bitwise_op op,
            bitwise_postproc postproc);

    // lhs should not be a composite word
    inline T bitwise_or(T lhs, T rhs){
        T res = rhs > 0 ? lhs | rhs :
                   (rhs & (m_wordcnt_mask + 1)) > 0 ?
                       m_sw_one_mask | 1 : lhs;
        // if all the bits of the result are 1, then use a composite word
        // to represent it
        return res == (~m_sw_zero_mask) ? m_sw_one_mask | 1 : res;
    }

    // lhs should not be a composite word
    inline T bitwise_and(T lhs, T rhs){
        T res =  rhs > 0 ? lhs & rhs :
                   (rhs & (m_wordcnt_mask + 1)) > 0 ?
                       lhs : m_sw_zero_mask | 1;
        // if all the bits of the result are 0, then use a composite word
        // to represent it
        return (0 == res) ? (m_sw_zero_mask | 1) : res;

    }

    // the post-processing for the OR operation. Here, we need to concat
    // the remainder bitmap elements to the result
    inline int or_postproc(Datum* result, int res_start, T* bitmap,
                            int bm_start, int bm_end){
        for (; bm_start < bm_end; ++bm_start, ++res_start)
            result[res_start] = get_Datum(bitmap[bm_start]);

        return res_start;

    }

    // the post-processing for the AND operation. Here, we need do nothing.
    inline int and_postproc(Datum*, int res_start, T*, int, int){
        return res_start;
    }

private:
    // the bitmap array related info
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
