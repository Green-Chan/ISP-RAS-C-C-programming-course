#ifndef __QSORT_FOR_ONEGIN
#define __QSORT_FOR_ONEGIN


#include <algorithm>
#include <cassert>
#include <stdexcept>


template<typename T>
using comparator = bool(*)(const T&, const T&);

namespace qsort_details
{
///-------------------------------------------------------------------------------------
//! Sorts array of size 2 in ascending order
//!
//! @param [in] arr  The pointer to the first element of the array
//! @param [in] cmp  Function that compare two elements of the array and
//!                  return true if the first is less than or equal to the second
//!
//! @note Does not check if @c arr is valid argument
//!
///-------------------------------------------------------------------------------------
    template<typename T>
    inline void m_sort_2(T *arr, comparator<T> cmp)
    {
        if (cmp(arr[1], arr[0])) {
            std::swap(arr[0], arr[1]);
        }
        assert(cmp(arr[0], arr[1]));
    };

///-------------------------------------------------------------------------------------
//! Sorts array of size 3 in ascending order
//!
//! @param [in] arr  The pointer to the first element of the array
//! @param [in] cmp  Function that compare two elements of the array and
//!                  return true if the first is less than or equal to the second
//!
//! @note Does not check if @c arr is valid argument
//!
///-------------------------------------------------------------------------------------
    template<typename T>
    inline void m_sort_3(T *arr, comparator<T> cmp)
    {
        m_sort_2(arr, cmp);
        assert(cmp(arr[0], arr[1]));
        if (cmp(arr[2], arr[1])) {
            auto tmp = arr[2];
            arr[2] = arr[1];
            if (cmp(tmp, arr[0])) {
                arr[1] = arr[0];
                arr[0] = tmp;
            } else {
                arr[1] = tmp;
            }
        }
        assert(cmp(arr[0], arr[1]) && cmp(arr[1], arr[2]));
    };

///-------------------------------------------------------------------------------------
//! Sorts array of size 4 in ascending order
//!
//! @param [in] arr  The pointer to the first element of the array
//! @param [in] cmp  Function that compare two elements of the array and
//!                  return true if the first is less than or equal to the second
//!
//! @note Does not check if @c arr is valid argument
//!
///-------------------------------------------------------------------------------------
    template<typename T>
    inline void m_sort_4(T *arr, comparator<T> cmp)
    {
        m_sort_3(arr, cmp);
        assert(cmp(arr[0], arr[1]) && cmp(arr[1], arr[2]));
        if (cmp(arr[3], arr[1])) {
            auto tmp = arr[3];
            arr[3] = arr[2];
            arr[2] = arr[1];
            if (cmp(tmp, arr[0])) {
                arr[1] = arr[0];
                arr[0] = tmp;
            } else {
                arr[1] = tmp;
            }
        } else {
            if (cmp(arr[3], arr[2])) {
                std::swap(arr[2], arr[3]);
            }
        }
        assert(cmp(arr[0], arr[1]) && cmp(arr[1], arr[2])
               && cmp(arr[2], arr[3]));
    }

///-------------------------------------------------------------------------------------
//! Sorts array of size 5 in ascending order
//!
//! @param [in] arr  The pointer to the first element of the array
//! @param [in] cmp  Function that compare two elements of the array and
//!                  return true if the first is less than or equal to the second
//!
//! @note Does not check if @c arr is valid argument
//!
///-------------------------------------------------------------------------------------
    template<typename T>
    inline void m_sort_5(T *arr, comparator<T> cmp)
    {
        m_sort_2(arr, cmp);
        m_sort_2(arr + 2, cmp);
        assert(cmp(arr[0], arr[1]) && cmp(arr[2], arr[3]));
        if (cmp(arr[2], arr[0])) {
            std::swap(arr[0], arr[2]);
            std::swap(arr[1], arr[3]);
        }
        if (cmp(arr[4], arr[2])) {
            auto tmp = arr[1];
            if (cmp(arr[4], arr[0])) {
                arr[1] = arr[0];
                arr[0] = arr[4];
            } else {
                arr[1] = arr[4];
            }
            if (cmp(tmp, arr[2])) {
                arr[4] = arr[3];
                arr[3] = arr[2];
                if (cmp(tmp, arr[1])) {
                    arr[2] = arr[1];
                    arr[1] = tmp;
                } else {
                    arr[2] = tmp;
                }
            } else {
                if (cmp(tmp, arr[3])) {
                    arr[4] = arr[3];
                    arr[3] = tmp;
                } else {
                    arr[4] = tmp;
                }
            }
        } else { // cmp(arr[4], arr[2]) >= 0
            if (cmp(arr[4], arr[3])) {
                std::swap(arr[3], arr[4]);
            }
            if (cmp(arr[1], arr[3])) {
                if (cmp(arr[2], arr[1])) {
                    std::swap(arr[1], arr[2]);
                }
            } else { // cmp(arr[1], arr[3]) > 0
                auto tmp = arr[1];
                arr[1] = arr[2];
                arr[2] = arr[3];
                if (cmp(arr[4], tmp)) {
                    arr[3] = arr[4];
                    arr[4] = tmp;
                } else {
                    arr[3] = tmp;
                }
            }
        }
        assert(cmp(arr[0], arr[1]) && cmp(arr[1], arr[2])
              && cmp(arr[2], arr[3]) && cmp(arr[3], arr[4]));
    }

    template<typename T>
    void inside_qsort(T *arr_begin, T *arr_end, comparator<T> cmp);

///-------------------------------------------------------------------------------------
//! Sorts array in ascending order
//!
//! @param [in] arr_begin  The pointer to the first element of the array
//! @param [in] arr_end    The pointer to the element after the last element of the array
//! @param [in] cmp        Function that compare two elements of the array and return
//!                        true if the first is less than or equal to the second
//!
//! @note Does not check if @c arr_begin and @c arr_end are valid arguments
//!
///-------------------------------------------------------------------------------------
    template<typename T>
    inline void choose_sort(T *arr_begin, T *arr_end, comparator<T> cmp)
    {
        assert(arr_end >= arr_begin);
        switch(arr_end - arr_begin) {
            case 0: // fallthroug
            case 1:
                return;
            case 2:
                m_sort_2<T>(arr_begin, cmp);
                break;
            case 3:
                m_sort_3<T>(arr_begin, cmp);
                break;
            case 4:
                m_sort_4<T>(arr_begin, cmp);
                break;
            case 5:
                m_sort_5<T>(arr_begin, cmp);
                break;
            default:
                inside_qsort<T>(arr_begin, arr_end, cmp);
        };
    }

///-------------------------------------------------------------------------------------
//! Sorts array of size more than 5 in ascending order
//!
//! @param [in] arr_begin  The pointer to the first element of the array
//! @param [in] arr_end    The pointer to the element after the last element of the array
//! @param [in] cmp        Function that compare two elements of the array and return
//!                        true if the first is less than or equal to the second
//!
//! @note Does not check if @c arr_begin and @c arr_end are valid arguments
//!
///-------------------------------------------------------------------------------------
    template<typename T>
    void inside_qsort(T *arr_begin, T *arr_end, comparator<T> cmp)
    {
        assert(arr_end >= arr_begin);

        T *little_elements_end = arr_begin + 1;
        T *big_elements_rend  = arr_end - 1;

        while (little_elements_end < big_elements_rend) {
            while (little_elements_end < big_elements_rend && cmp(*little_elements_end, arr_begin[0])) {
                little_elements_end++;
            }
            while (little_elements_end < big_elements_rend && !cmp(*big_elements_rend, arr_begin[0])) {
                big_elements_rend--;
            }
            if (little_elements_end == big_elements_rend) {
                if (cmp(*little_elements_end, arr_begin[0])) {
                    little_elements_end++;
                } else {
                    big_elements_rend--;
                }
            } else {
                std::swap(*little_elements_end++, *big_elements_rend--);
            }
        }
        if (little_elements_end == arr_end) {
            std::swap(arr_begin[0], *(arr_end - 1));
            little_elements_end--;
            big_elements_rend--;
        } else if (little_elements_end == big_elements_rend) {
            if (cmp(*little_elements_end, arr_begin[0])) {
                little_elements_end++;
            } else {
                big_elements_rend--;
            }
        }
        T* big_elements_begin = big_elements_rend + 1;
        choose_sort(arr_begin, little_elements_end, cmp);
        choose_sort(big_elements_begin, arr_end, cmp);
    }
}

///-------------------------------------------------------------------------------------
//! Sorts array in ascending order
//!
//! @param [in] arr_begin  The pointer to the first element of the array
//! @param [in] arr_end    The pointer to the element after the last element of the array
//! @param [in] cmp        Function that compare two elements of the array and return
//!                        true if the first is less than or equal to the second
//!
//! @note Checks if @c arr_begin and @c arr_end are valid arguments
//!
///-------------------------------------------------------------------------------------
template<typename T>
void qsort(T *arr_begin, T *arr_end, comparator<T> cmp)
{
    if (arr_begin == nullptr) {
        throw std::invalid_argument("qsort: arr_begin == nullptr");
    }
    if (arr_end == nullptr) {
        throw std::invalid_argument("qsort: arr_end == nullptr");
    }
    if (arr_end < arr_begin) {
        throw std::invalid_argument("qsort: arr_end < arr_begin");
    }
    qsort_details::choose_sort<T>(arr_begin, arr_end, cmp);
}

#endif // __QSORT_FOR_ONEGIN
