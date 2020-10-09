#ifndef __TEXT_SORTING_FOR_ONEGIN
#define __TEXT_SORTING_FOR_ONEGIN

#include <string_view>

enum language {
    RUSSIAN,
    ENGLISH
};

///-------------------------------------------------------------------------------------
//! <b> That is the function that performs the algorithm specified at the main page of the documentation. </b>
//! Sorts lines in text from file three times: in ascending order, in ascending order from the back of the line, to its original version.
//!
//! @param [in] file_in_path               Path to the file with text
//! @param [in] file_out_sorted_path       Path to the file where to write the sorted version
//! @param [in] file_out_sorted_back_path  Path to the file where to write the sorted from back version
//! @param [in] file_out_origin_path       Path to the file where to write the origin version
//! @param [in] lang                       The language of the text
//!
//! @attention If @c file_out_path exists, it will be overwritten
//!
//! @note While comparing lines not alpha and not digit symbols are ignored, uppercase and lowercase symbols are considered equal.
//!       Only letters of the specified ( @c lang ) alphabet are not ignored.
//!
///-------------------------------------------------------------------------------------
void sort_text(const char *file_in_path, const char *file_out_sorted_path, const char *file_out_sorted_back_path, const char *file_out_origin_path, language lang);

///-------------------------------------------------------------------------------------
//! Compares two strings ignoring not English alpha and not digit symbols and considering uppercase and lowercase symbols equal.
//!
//! @param [in] str1  First string
//! @param [in] str2  Second string
//!
//! @return -1 if str1 is less than str2. 1 if str1 is greater than str2. 0 if they are equal.
//!
///-------------------------------------------------------------------------------------
int compare_en_strings(const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2);

///-------------------------------------------------------------------------------------
//! Compares two strings from the backward (as they were reversed) ignoring not English alpha and not digit symbols and considering uppercase and lowercase symbols equal.
//!
//! @param [in] str1  First string
//! @param [in] str2  Second string
//!
//! @return -1 if str1 is less than str2. 1 if str1 is greater than str2. 0 if they are equal.
//!
///-------------------------------------------------------------------------------------
int compare_en_strings_r(const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2);

///-------------------------------------------------------------------------------------
//! Compares two strings ignoring not Russian alpha and not digit symbols and considering uppercase and lowercase symbols equal.
//!
//! @param [in] str1  First string
//! @param [in] str2  Second string
//!
//! @return -1 if str1 is less than str2. 1 if str1 is greater than str2. 0 if they are equal.
//!
///-------------------------------------------------------------------------------------
int compare_ru_strings(const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2);

///-------------------------------------------------------------------------------------
//! Compares two strings from the backward (as they were reversed) ignoring not Russian alpha and not digit symbols and considering uppercase and lowercase symbols equal.
//!
//! @param [in] str1  First string
//! @param [in] str2  Second string
//!
//! @return -1 if str1 is less than str2. 1 if str1 is greater than str2. 0 if they are equal.
//!
///-------------------------------------------------------------------------------------
int compare_ru_strings_r(const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2);

#endif
