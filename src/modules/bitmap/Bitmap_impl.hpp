#ifndef MADLIB_MODULES_BITMAP_BITMAP_IMPL_HPP
#define MADLIB_MODULES_BITMAP_BITMAP_IMPL_HPP

namespace madlib {
namespace modules {
namespace bitmap {


/**
 * @brief insert the input number to a composite word.
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
 * @param pos_in_word   the position in the inserted word for the input number
 * @param word_pos      the word for the input number will
 *                      be inserted to the new bitmap
 * @param num_words     the number of words in the active word
 *
 * @return the new bitmap after inserted.
 */
template<typename T>
Bitmap<T>& Bitmap<T>::insert_compositeword
(
    T* newbitmap,
    int index,
    int pos_in_word,
    int word_pos,
    int num_words
){
    //elog(NOTICE, "shift_bitmap: %d, %d, %d, %d", index, pos_in_word, word_pos, num_words);
    memmove(newbitmap, m_bitmap, (index + 1) * sizeof(T));
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
        if (1 == word_pos){
            newbitmap[index + 1] = (T)(num_words - 1) | m_sw_zero_mask;
        }else{
            newbitmap[index] = (T)(num_words - 1) | m_sw_zero_mask;
            ++index;
        }
        newbitmap[0] += 1;
    }

    // other bits will be erased
    newbitmap[index] = (T)1 << (pos_in_word - 1);

    m_bitmap = newbitmap;
    m_size = m_bitmap[0];

    return *this;
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
Bitmap<T>& Bitmap<T>::insert
(
    int64 bit_pos
){
    int64 cur_pos = 0;
    int64 num_words = 1;
    int i = 1;

    // for 32-bit bitmap, the maximum number is (1 << 30) - 1
    // for 64 bit bitmap, the maximum number is (1 << 62) - 1
    madlib_assert(bit_pos > 0 && bit_pos < (1 << (m_base - 1)),
        std::invalid_argument
        ("the input number is greater than the supported maximum number"));

    madlib_assert(m_size_per_add > 1,
        std::invalid_argument
        ("size_per_add should be greater than 2"));

    // visit each element of the bitmap array to find the right word to
    // insert the input number
    for (i = 1; i < m_size; ++i){
        if (m_bitmap[i] > 0){
            cur_pos += m_base;
            // insert the input bit position to a normal word
            if (cur_pos >= bit_pos){
                m_bitmap[i] |= (T)1 << ((get_pos_word(bit_pos)) - 1);
                break;
            }
        }else if (m_bitmap[i] < 0){
            // get the number of words for a composite word
            // each word contains m_base bits
            num_words = m_bitmap[i] & m_wordcnt_mask;
            cur_pos += num_words * m_base;

            if (cur_pos >= bit_pos){
                bit_pos -= (cur_pos - num_words * m_base);
                int pos_in_word = get_pos_word(bit_pos);
                int word_pos = get_num_words(bit_pos);

                // if the active word only contains 1 word
                if (1 == num_words){
                    m_bitmap[i] = (T)1 << (pos_in_word - 1);
                    break;
                }

                T* newbitmap = m_bitmap;
                // need to increase the memory size
                if (((1 == word_pos || num_words == word_pos) && m_size == m_capacity) ||
                    (word_pos > 1 && word_pos < num_words && m_size >= m_capacity - 1)){
                        m_capacity += m_size_per_add;
                        newbitmap = new T[m_capacity];
                        memset(newbitmap, 0x00, m_capacity * sizeof(T));
                        m_bitmap_updated = true;
                }

                insert_compositeword(newbitmap, i, pos_in_word, word_pos, num_words);

                break;
            }
        }else{
            break;
        }
    }

    // reach the end of the bitmap array
    if (i == m_size || 1 == m_bitmap[0] || 0 == m_bitmap[i]){
        bit_pos -= cur_pos;
        cur_pos = get_pos_word(bit_pos);
        num_words = get_num_words(bit_pos);
        if (((1 == num_words) ? 1 : 2) + m_size > m_capacity){
            // allocate new memory
            m_capacity += m_size_per_add;
            T* newbitmap = new T[m_capacity];
            memset(newbitmap, 0x00, m_capacity * sizeof(T));
            memcpy(newbitmap, m_bitmap, m_size * sizeof(T));
            m_bitmap = newbitmap;
            m_bitmap_updated = true;
         }
        if (1 == num_words){
            m_bitmap[i] = (T)1 << (cur_pos - 1);
            m_bitmap[0] += 1;
        }else{
            m_bitmap[i] = m_sw_zero_mask | (T)(num_words - 1);
            m_bitmap[i + 1] = (T)1 << (cur_pos - 1);
            m_bitmap[0] += 2;
        }

        // set the size of the bitmap
        m_size = m_bitmap[0];
    }

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
ArrayHandle<T> Bitmap<T>::to_ArrayHandle
(
    bool use_capacity
){
    int size = use_capacity ? m_capacity : m_size;

    Datum* result = new Datum[size];
    for (int i = 0; i < size; ++i){
        result[i] = get_Datum((T)m_bitmap[i]);
    }

    return construct_array(
            result,
            size,
            m_typoid,
            m_typlen,
            m_typbyval,
            m_typalign
            );
}


template <typename T>
inline
Datum Bitmap<T>::to_PointerDatum
(
    bool use_capacity
){
    int size = use_capacity ? m_capacity : m_size;

    Datum* result = new Datum[size];
    for (int i = 0; i < size; ++i){
        result[i] = get_Datum((T)m_bitmap[i]);
    }

    return PointerGetDatum(construct_array(
            result,
            size,
            m_typoid,
            m_typlen,
            m_typbyval,
            m_typalign
            ));
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
ArrayHandle<T> Bitmap<T>::bitwise_proc
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
    T bit_test_mask = m_wordcnt_mask + 1;
    T temp;
    T pre_word = 0;
    int capacity = m_size + rhs.m_size;
    Datum* result = new Datum[capacity];

    for (; i < m_size && j < rhs.m_size; ++k){
        // the two words have the same sign
        if ((m_bitmap[i] ^ rhs.m_bitmap[j]) >= 0){
            temp = (this->*op)(m_bitmap[i], rhs.m_bitmap[j]);
            // the two words are active words
            if (m_bitmap[i] < 0){
                num_words1 = m_bitmap[i] & m_wordcnt_mask;
                num_words2 = rhs.m_bitmap[j] & m_wordcnt_mask;

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
                j += (rhs.m_bitmap[j] & m_wordcnt_mask) > 0 ? 0 : 1;
                ++i;
            }else{
                // "this" is a composite word and rhs is a normal word
                temp = (this->*op)(rhs.m_bitmap[j], m_bitmap[i]);
                m_bitmap[i] -= 1;
                i += (m_bitmap[i] & m_wordcnt_mask) > 0 ? 0 : 1;
                ++j;
            }
        }

        // merge if needed
        if (k >= 2 && temp < 0 && pre_word < 0 &&
           (0 == ((pre_word ^ temp) & bit_test_mask))){
            pre_word += (temp & m_wordcnt_mask);
            result[--k] = get_Datum(pre_word);
        }else{
            result[k] = get_Datum(temp);
            pre_word = temp;
        }
    }

    // post-processing
    k = (this->*postproc)(result, k, m_bitmap, i, m_size);
    k = (this->*postproc)(result, k, rhs.m_bitmap, j, rhs.m_size);

    // if the bitmap only has one word, and the word is a composite word with all
    // values are 0, then trim it
    k = (2 == k) && ((pre_word & m_sw_one_mask) == m_sw_zero_mask) ? 1 : k;

    madlib_assert(k <= capacity,
        std::logic_error
        ("the real size of the bitmap should be no greater than its capacity"));

    result[0] = get_Datum((T)k);

    return construct_array(
            result,
            k,
            m_typoid,
            m_typlen,
            m_typbyval,
            m_typalign
            );
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
ArrayHandle<T> Bitmap<T>::operator | (Bitmap<T>& rhs){
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
ArrayHandle<T> Bitmap<T>::operator & (Bitmap<T>& rhs){
    return bitwise_proc(rhs, &Bitmap<T>::bitwise_and, &Bitmap<T>::and_postproc);
}


/**
 * @brief get the positions of the non-zero bits. The position starts from 1.
 *
 * @return the array contains the positions
 *
 */
template <typename T>
inline
ArrayHandle<T> Bitmap<T>::nonzero_positions(){
    Datum* result = new Datum[nonzero_count()];
    T j = 0;
    T k = 1;
    T begin_pos = 1;
    for (int i = 1; i < m_size; ++i){
        k = begin_pos;
        T word = m_bitmap[i];
        if (word > 0){
            do{
                if (1 == (word & 0x01))
                    result[j++] = get_Datum(k);
                word >>= 1;
                ++k;
            }while (word > 0);
            begin_pos += m_base;
        }else{
            if ((word & (m_wordcnt_mask + 1)) > 0){
                int n = (word & m_wordcnt_mask) * m_base;
                for (; n > 0 ; --n){
                    result[j++] = get_Datum(k++);
                }
            }
            begin_pos += (word & m_wordcnt_mask) * m_base;
        }
    }

    return construct_array(
            result,
            j,
            m_typoid,
            m_typlen,
            m_typbyval,
            m_typalign
            );
}

} // namespace bitmap
} // namespace modules
} // namespace madlib

#endif
