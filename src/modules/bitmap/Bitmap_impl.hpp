#ifndef MADLIB_MODULES_BITMAP_BITMAP_IMPL_HPP
#define MADLIB_MODULES_BITMAP_BITMAP_IMPL_HPP

namespace madlib {
namespace modules {
namespace bitmap {

template <typename T>
inline
Bitmap<T>::Bitmap
(
    int capacity,
    int size_per_add
) :
    m_bmArray(NULL), m_bitmap(NULL), m_size(1), m_capacity(capacity),
    m_size_per_add(size_per_add), m_bitmap_updated(true),
    m_base(sizeof(T) * 8 - 1),
    m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
    m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
    m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)){
    set_typInfo();
    m_bmArray = BM_CONSTRUCT_ARRAY((Datum*)NULL, capacity);
    m_bitmap = reinterpret_cast<T*>(ARR_DATA_PTR(m_bmArray));
    m_bitmap[0] = 1;
}

//ctor
template <typename T>
inline
Bitmap<T>::Bitmap
(
    ArrayHandle<T> handle,
    int size_per_add
) :
    m_bmArray(const_cast<ArrayType*>(handle.array())),
    m_bitmap(const_cast<T*>(handle.ptr())), m_size(handle[0]),
    m_capacity(handle.size()), m_size_per_add(size_per_add),
    m_bitmap_updated(false), m_base(sizeof(T) * 8 - 1),
    m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
    m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
    m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)) {
    set_typInfo();
}

//ctor
template <typename T>
inline
Bitmap<T>::Bitmap
(
    Bitmap& rhs
):
    m_bmArray(rhs.m_bmArray), m_bitmap(rhs.m_bitmap), m_size(rhs.m_size),
    m_capacity(rhs.m_capacity), m_size_per_add(rhs.m_size_per_add),
    m_bitmap_updated(rhs.m_bitmap_updated), m_base(sizeof(T) * 8 - 1),
    m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
    m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
    m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)),
    m_typoid(rhs.m_typoid), m_typlen(rhs.m_typelen),
    m_typbyval(rhs.m_typbyval), m_typalign(rhs.m_typalign){
}

//ctor
template <typename T>
inline
Bitmap<T>::Bitmap
(
    char* str
):
    m_bmArray(NULL), m_bitmap(NULL), m_size(1), m_capacity(8),
    m_size_per_add(8), m_bitmap_updated(false),
    m_base(sizeof(T) * 8 - 1),
    m_wordcnt_mask(((T)1 << (sizeof(T) * 8 - 2)) - 1),
    m_sw_zero_mask((T)1 << (sizeof(T) * 8 - 1)),
    m_sw_one_mask((T)3 << (sizeof(T) * 8 - 2)){
    // init the member variables
    set_typInfo();
    m_bmArray = BM_CONSTRUCT_ARRAY((Datum*)NULL, m_capacity);
    m_bitmap = reinterpret_cast<T*>(ARR_DATA_PTR(m_bmArray));
    m_bitmap[0] = 1;

    // convert the input string to a bitmap
    char elem[MAXBITSOFINT64] = {'\0'};
    int j = 0;
    int64 input_pos = 0;

    for (; *str != '\0'; ++str){
        if (',' == *str){
            elem[j] = '\0';
            (void) scanint8(elem, false, &input_pos);
            insert(input_pos);
            j = 0;
        }else{
            elem[j++] = *str;
        }
    }

    (void) scanint8(elem, false, &input_pos);
    insert(input_pos);
}


/**
 * @brief breakup the composite word, and insert the input number to it.
 *        three conditions need to be considered here (assume that the active
 *        word contains n normal words):
 *        + if the input number is inserted into the 1st word, then the active
 *          word will be breakup to:
 *              "a normal word" + "a composite word with n - 1 words"
 *        + if the input number is inserted into the nth word, then the active
 *          word will be break up to:
 *              "a composite word with n - 1 words" + "a normal word"
 *        + if the input number is inserted into ith word (1<i<n), then the
 *          active word will be breakup to:
 *              "a composite word with i - 1 words" + "a normal word" + "a composite
 *               word with n - i words
 *
 * @param newbitmap     the new bitmap for keeping the breakup bitmap. If the
 *                      old bitmap is not large enough to keep the breakup
 *                      bitmap, then we need to reallocate. Otherwise, the new
 *                      bitmap is the same as the old bitmap (in this case,
 *                      just move the elements)
 * @param index         the index of the active word will be used to insert the
 *                      input number
 * @param pos_in_word   the inserting position for the input number
 * @param word_pos      the word for the input number will
 *                      be inserted to the new bitmap
 * @param num_words     the number of words in the composite word
 *
 * @return the new bitmap.
 */
template<typename T>
void
Bitmap<T>::breakup_compword
(
    T* newbitmap,
    int index,
    int pos_in_word,
    int word_pos,
    int num_words
){
    memmove(newbitmap, m_bitmap, (index + 1) * sizeof(T));
    // the inserted position is in the middle of a composite word
    if (word_pos > 1 && word_pos < num_words){
        memcpy(newbitmap + index + 2,
                m_bitmap + index, (m_size - index) * sizeof(T));
        newbitmap[index] = (T)(word_pos - 1) | m_sw_zero_mask;
        newbitmap[index + 2] = (T)(num_words - word_pos) | m_sw_zero_mask;
        ++index;
        newbitmap[0] += 2;
    }else{
        memmove(newbitmap + index + 1,
                m_bitmap + index, (m_size - index) * sizeof(T));
        // the inserted position is in the beginning of a composite word
        if (1 == word_pos){
            newbitmap[index + 1] = (T)(num_words - 1) | m_sw_zero_mask;
        }else{
            // the inserted position is in the end of a composite word
            newbitmap[index] = (T)(num_words - 1) | m_sw_zero_mask;
            ++index;
        }
        newbitmap[0] += 1;
    }

    newbitmap[index] = (T)1 << (pos_in_word - 1);
    m_bitmap = newbitmap;
    m_size = m_bitmap[0];
}


/**
 * @breif insert the specified bit to the composite word
 *
 * @param bit_pos       the position of the input bit
 * @param num_words     the number of words in the composite word
 * @param index         the subscript of bitmap array where the input
 *                      bit will be inserted
 */
template<typename T>
void
Bitmap<T>::insert_compword
(
    int64_t bit_pos,
    int64_t num_words,
    int index
){
    int pos_in_word = get_pos_word(bit_pos);

    // if the composite word only contains 1 word and
    // all of them are zero
    if (1 == num_words){
        m_bitmap[index] = (T)1 << (pos_in_word - 1);
        return;
    }

    int64_t word_pos = BM_NUMWORDS_FOR_BITS(bit_pos);
    T* newbitmap = m_bitmap;

    // need to increase the memory size
    if (((1 == word_pos || num_words == word_pos) && m_size == m_capacity) ||
        (word_pos > 1 && word_pos < num_words && m_size >= m_capacity - 1)){
            m_capacity += m_size_per_add;
            newbitmap = alloc_bitmap(m_capacity);
            m_bitmap_updated = true;
    }

    breakup_compword(newbitmap, index, pos_in_word, word_pos, num_words);
}


/**
 * @brief append the given number to the bitmap.
 *
 * @param bit_pos   the input number
 *
 * @return the new bitmap after appended.
 *
 */
template <typename T>
inline
void
Bitmap<T>::append
(
    int64_t bit_pos
){
    int64_t need_elems = 1;
    T max_bits = (T)BM_MAXBITS_IN_COMP;
    int64_t num_words = BM_NUMWORDS_FOR_BITS(bit_pos);
    int64_t cur_pos = get_pos_word(bit_pos);
    int i = m_size;

    if (num_words <= max_bits + 1){
        // a composite word can represent all zeros,
        // then two new elements are enough
        need_elems = (1 == num_words) ? 1 : 2;
    }else{
        // we need 2 more composite words to represent all zeros
        need_elems = (num_words - 1 + max_bits - 1) / max_bits + 1;
        num_words = (num_words - 1) % max_bits + 1;
    }

    if (need_elems + m_size > m_capacity){
        m_capacity += ((need_elems + m_size_per_add - 1 ) / m_size_per_add)
                      * m_size_per_add;
        m_bitmap = alloc_bitmap(m_capacity, m_bitmap, m_size);
        m_bitmap_updated = true;
    }

    // fill the composite words
    for (; need_elems > 2; --need_elems){
        m_bitmap[i++] = m_sw_zero_mask | max_bits;
        ++m_bitmap[0];
    }

    // the first word is composite word
    // the second is a normal word
    if ((2 == need_elems) && (num_words > 1)){
        m_bitmap[i] = m_sw_zero_mask | (T)(num_words - 1);
        m_bitmap[++i] = (T)1 << (cur_pos - 1);
        m_bitmap[0] += 2;
    }else{
        // only one normal word can represent the input number
        m_bitmap[i] = (T)1 << (cur_pos - 1);
        m_bitmap[0] += 1;
    }

    // set the size of the bitmap
    m_size = m_bitmap[0];
}


/**
 * @brief insert the give number to the bitmap.
 *
 * @param bit_pos   the input number
 *
 * @return the new bitmap after inserted.
 *
 */
template <typename T>
inline
Bitmap<T>&
Bitmap<T>::insert
(
    int64_t bit_pos
){
    madlib_assert(bit_pos > 0,
            std::invalid_argument("the bit position must be a positive number"));

    int64_t cur_pos = 0;
    int64_t num_words = 1;
    int i = 1;

    // visit each element of the bitmap array to find the right word to
    // insert the input number
    for (i = 1; i < m_size; ++i){
        if (m_bitmap[i] > 0){
            cur_pos += m_base;
            // insert the input bit position to a normal word
            if (cur_pos >= bit_pos){
                m_bitmap[i] |= (T)1 << ((get_pos_word(bit_pos)) - 1);
                return *this;
            }
        }else if (m_bitmap[i] < 0){
            // get the number of words for a composite word
            // each word contains m_base bits
            num_words = BM_NUMWORDS_IN_COMP(m_bitmap[i]);
            int64_t temp = num_words * m_base;
            cur_pos += temp;
            if (cur_pos >= bit_pos){
                insert_compword(bit_pos - cur_pos - temp, num_words, i);
                return *this;
            }
        }
    }

    // reach the end of the bitmap
    append(bit_pos - cur_pos);

    return *this;
}


/**
 * @brief transform the bitmap to an ArrayHandle instance
 *
 * @param use_capacity  true if we don't trim the 0 elements
 *
 * @param the ArrayHandle instance for the given bitmap.
 */
template <typename T>
inline
ArrayHandle<T>
Bitmap<T>::to_ArrayHandle
(
    bool use_capacity /* true */
){
    if (use_capacity || (m_size == m_capacity))
        return m_bmArray;

    // do not change m_bitmap and m_bmArray
    return alloc_array(m_bitmap, m_size);
}


/**
 * @brief the entry function for doing the bitwise operations,
 *        such as |, & and ^, etc.
 *
 * @param rhs       the bitmap
 * @param op        the function used to do the real operation, such as | and &
 * @param postproc  the function used to post-processing the bitmap
 *
 * @return the result of "this" OP "rhs", where OP can be |, & and ^.
 */
template <typename T>
inline
ArrayType*
Bitmap<T>::bitwise_proc
(
    Bitmap<T>& rhs,
    bitwise_op op,
    bitwise_postproc postproc
){
    int i = 1;
    int j = 1;
    int k = 1;
    int num_words1 = 0;
    int num_words2 = 0;
    T temp;
    T pre_word = 0;
    int capacity = m_size + rhs.m_size;
    T* result = new T[capacity];

    for (; i < m_size && j < rhs.m_size; ++k){
        // the two words have the same sign
        if ((m_bitmap[i] ^ rhs.m_bitmap[j]) >= 0){
            temp = (this->*op)(m_bitmap[i], rhs.m_bitmap[j]);
            // the two words are active words
            if (m_bitmap[i] < 0){
                num_words1 = BM_NUMWORDS_IN_COMP(m_bitmap[i]);
                num_words2 = BM_NUMWORDS_IN_COMP(rhs.m_bitmap[j]);

                // get the sign of the result
                temp = temp & m_sw_one_mask;
                if (num_words1 > num_words2){
                    temp |= num_words2;
                    m_bitmap[i] -= num_words2;
                    --i;
                }else if (num_words1 < num_words2){
                    temp |= num_words1;
                    rhs.m_bitmap[j] -= num_words1;
                    --j;
                }else{
                    temp |= num_words1;
                }
            }
            ++i;
            ++j;
        }else{
            // "this" is a normal word and rhs is a composite word
            if (m_bitmap[i] > 0){
                temp = (this->*op)(m_bitmap[i], rhs.m_bitmap[j]);
                rhs.m_bitmap[j] -= 1;
                j += BM_NUMWORDS_IN_COMP(rhs.m_bitmap[j]) > 0 ? 0 : 1;
                ++i;
            }else{
                // "this" is a composite word and rhs is a normal word
                temp = (this->*op)(rhs.m_bitmap[j], m_bitmap[i]);
                m_bitmap[i] -= 1;
                i += BM_NUMWORDS_IN_COMP(m_bitmap[i]) > 0 ? 0 : 1;
                ++j;
            }
        }

        // merge if needed
        if (k >= 2 && BM_SAME_SIGN(temp, pre_word)){
            pre_word += BM_NUMWORDS_IN_COMP(temp);
            result[--k] = pre_word;
        }else{
            result[k] = temp;
            pre_word = temp;
        }
    }

    // post-processing
    k = (this->*postproc)(result, k, m_bitmap, i, m_size, pre_word);
    k = (this->*postproc)(result, k, rhs.m_bitmap, j, rhs.m_size, pre_word);

    // if the bitmap only has one word, and the word is a composite word with all
    // values are 0, then trim it
    k = (2 == k) && ((pre_word & m_sw_one_mask) == m_sw_zero_mask) ? 1 : k;

    madlib_assert(k <= capacity,
        std::logic_error
        ("the real size of the bitmap should be no greater than its capacity"));

    result[0] = (T)k;

    return alloc_array(result, k);
}


/**
 * @brief override the operator |
 *
 * @param rhs   the bitmap
 *
 * @return the result of "this" | "rhs"
 *
 */
template <typename T>
inline
ArrayHandle<T>
Bitmap<T>::operator | (Bitmap<T>& rhs){
    return bitwise_proc(rhs, &Bitmap<T>::bitwise_or, &Bitmap<T>::or_postproc);
}


/**
 * @brief override the operator &
 *
 * @param rhs   the bitmap
 *
 * @return the result of "this" & "rhs"
 *
 */
template <typename T>
inline
ArrayHandle<T>
Bitmap<T>::operator & (Bitmap<T>& rhs){
    return bitwise_proc(rhs, &Bitmap<T>::bitwise_and, &Bitmap<T>::and_postproc);
}


/**
 * @brief get the number of nonzero bits
 *
 * @return the nonzero count
 */
template <typename T>
inline
int64_t
Bitmap<T>::nonzero_count(){
    int64_t   res = 0;
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


/**
 * @brief get the positions of the non-zero bits. The position starts from 1.
 *
 * @param result    the array used to keep the positions
 *
 * @return the array containing the positions whose bits are 1.
 */
template <typename T>
inline
int64_t*
Bitmap<T>::nonzero_positions(int64_t* result){
    madlib_assert(result != NULL,
            std::invalid_argument("the positions array must not be NULL"));

    int64_t j = 0;
    int64_t k = 1;
    int64_t begin_pos = 1;
    for (int i = 1; i < m_size; ++i){
        k = begin_pos;
        T word = m_bitmap[i];
        if (word > 0){
            do{
                if (1 == (word & 0x01))
                    result[j++] = k;
                word >>= 1;
                ++k;
            }while (word > 0);
            begin_pos += m_base;
        }else{
            if ((word & (m_wordcnt_mask + 1)) > 0){
                int n = (word & m_wordcnt_mask) * m_base;
                for (; n > 0 ; --n){
                    result[j++] = k++;
                }
            }
            begin_pos += (word & m_wordcnt_mask) * m_base;
        }
    }

    return result;
}


/**
 * @brief get the positions of the non-zero bits. The position starts from 1.
 *
 * @return the array contains the positions
 *
 */
template <typename T>
inline
ArrayHandle<int64_t>
Bitmap<T>::nonzero_positions(){
    int64_t* result;
    ArrayType* res_arr = alloc_array<int64_t>(result, nonzero_count());
    nonzero_positions(result);

    return res_arr;
}


/**
 * @brief convert the bitmap to a readable format.
 *        If more than 2 elements are continuous, then we use '~' to concat
 *        the begin and the end of the continuous numbers. Otherwise, we will
 *        use ',' to concat them.
 *        e.g. assume the bitmap is '1,2,3,5,6,8,10,11,12,13'::bitmap, then
 *        the output of to_string is '1~3,5,6,8,10~13'
 *
 * @return the readable string representation for the bitmap.
 */
template <typename T>
inline
char*
Bitmap<T>::to_string(){
    int64_t size = nonzero_count();

    // no elements in the bitmap
    if (0 == size){
        return NULL;
    }

    int64_t* result = new int64_t[size + 1];
    // here, we shouldn't align the size, don't use BM_ALIGN_ALLOC0
    char* res = (char*) BM_ALLOC0(size * MAXBITSOFINT64 * sizeof(char));
    char* pstr = res;
    nonzero_positions(result);
    int j = 0;
    int len = 0;
    result[size] = -1;
    ++size;
    for (int i = 1; i < size; ++i){
        if (result[i - 1] != result[i] - 1){
            if (j == i - 1){
                // j~j+1 is not continuous
                len = int64_to_string(pstr, result[j]);
                pstr += len;
            }else if (j == i - 2){
                // j ~ j + 1 is continuous, we use comma to separate them
                len = int64_to_string(pstr, result[j]);
                pstr += len;
                *pstr++ = ',';
                len = int64_to_string(pstr, result[j + 1]);
                pstr += len;
            }else{
                // j ~ j + n is continuous, where n > 2
                // in this case, we use ~ to ignore the middle elements
                len = int64_to_string(pstr, result[j]);
                pstr += len;
                *pstr++ = '~';
                len = int64_to_string(pstr, result[i - 1]);
                pstr += len;
            }
            j = i;
            *pstr++ = ',';
        }
    }

    *--pstr = '\0';

    if (0 == j){
        int64_to_string(pstr, result[0]);
    }

    return res;
}


/**
 * @brief convert the bitmap to varbit
 *
 * @return the varbit representation for the bitmap
 */
template <typename T>
inline
VarBit*
Bitmap<T>::to_varbit(){
    int64_t size = nonzero_count();

    if (0 == size){
        return NULL;
    }

    int64_t* pos = new int64_t[size];
    (void*) nonzero_positions(pos);

    // get the varbit related information
    int64_t bitlen = pos[size - 1];
    int64_t len = VARBITTOTALLEN(bitlen);
    VarBit* result = (VarBit*)BM_ALIGN_ALLOC0(len);
    SET_VARSIZE(result, len);
    VARBITLEN(result) = bitlen;
    bits8* pres = VARBITS(result);

    // set the varbit
    for (int i = 0; i < size; ++i){
        int64_t curindex = ((pos[i] + 7) >> 3) - 1;
        int64_t curpos = (pos[i] & 0x07);
        curpos = (0 == curpos) ? 0 : 8 - curpos;
        *(pres + curindex) += ((bits8)1 << curpos);
    }
    return result;
}

} // namespace bitmap
} // namespace modules
} // namespace madlib

#endif
