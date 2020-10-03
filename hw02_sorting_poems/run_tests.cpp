#include "qsort.h"
#include "windows_unit_tests.h"
#include "text_sorting.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <string_view>


#define $test_qsort(vec, cmp)                    \
{                                                \
    std::cout << "array: " << vec << std::endl;  \
    auto vec1 = vec;                             \
    auto vec2 = vec;                             \
    qsort(&vec1[0], &vec1[0] + vec1.size(), cmp);\
    std::sort(vec2.begin(), vec2.end());         \
    $unit_test(vec1, vec2);                      \
    std::cout << std::endl;                      \
}

#define $test_str_cmp(cmp, str1, str2, res)          \
{                                                    \
    std::cout << str1 << " vs " << str2 << std::endl;\
    $unit_test(cmp(str1, str2), res);                \
    std::cout << std::endl;                          \
}

std::ostream& operator<< (std::ostream &out, const std::vector<int> &vec)
{
    out << "{ ";
    if (vec.size() > 0) {
        for (size_t i = 0; i < vec.size() - 1; i++) {
            out << vec[i] << ", ";
        }
        out << vec[vec.size() - 1] << " }";
    } else {
        out << "}";
    }
    return out;
}

std::ostream& operator<< (std::ostream &out, const std::basic_string_view<char16_t> &str)
{
    out << "\"";
    for (size_t i = 0; i < str.size(); i++) {
        out << (char) str[i];
    }
    out << "\"";
    return out;
}

int main() {
    comparator<int> int_cmp = [](const int &arg1, const int &arg2) { return (arg1 <= arg2); };


    $begin_testing();

    std::cout << "Testing mini_qsort" << std::endl;
    std::vector<int> mini_vec = {};

    $test_qsort(mini_vec, int_cmp);
    for (int a1 = 0; a1 < 5; a1++) {
        mini_vec.push_back(a1);
        $test_qsort(mini_vec, int_cmp);
        for (int a2 = 0; a2 < 5; a2++) {
            mini_vec.push_back(a2);
            $test_qsort(mini_vec, int_cmp);
            for(int a3 = 0; a3 < 5; a3++) {
                mini_vec.push_back(a3);
                $test_qsort(mini_vec, int_cmp);
                for(int a4 = 0; a4 < 5; a4++) {
                    mini_vec.push_back(a4);
                    $test_qsort(mini_vec, int_cmp);
                    for (int a5 = 0; a5 < 5; a5++) {
                        mini_vec.push_back(a5);
                        $test_qsort(mini_vec, int_cmp);
                        mini_vec.pop_back();
                    }
                    mini_vec.pop_back();
                }
                mini_vec.pop_back();
            }
            mini_vec.pop_back();
        }
        mini_vec.pop_back();
    }

    std::cout << "Testing qsort" << std::endl;

    std::vector<int> big_vec = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    $test_qsort(big_vec, int_cmp);

    big_vec.resize(64);
    $test_qsort(big_vec, int_cmp);

    big_vec = { 1, 4, 4, 5, 2, 7, 5, 7, 9, 3, 5, 4, 9, 23, 43, 66, 15, 15, 54, 4, 5, 4, 5, 3, 4, 6, 7, 1 };
    $test_qsort(big_vec, int_cmp);

    big_vec = { 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    $test_qsort(big_vec, int_cmp);

    big_vec = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 4, 4, 4, 4};
    $test_qsort(big_vec, int_cmp);

    big_vec = { 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 24 };
    $test_qsort(big_vec, int_cmp);

    std::cout << "Testing comparators" << std::endl;

    const char16_t char_arr1[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd' };
    const char16_t char_arr2[] = {'.', '.', '.', 'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd' };
    std::basic_string_view<char16_t> str1(char_arr1, sizeof(char_arr1) / sizeof(char_arr1[0]));
    std::basic_string_view<char16_t> str2(char_arr2, sizeof(char_arr2) / sizeof(char_arr2[0]));
    $test_str_cmp(compare_en_strings, str1, str2, 0);
    $test_str_cmp(compare_en_strings, str2, str1, 0);
    $test_str_cmp(compare_en_strings_r, str1, str2, 0);
    $test_str_cmp(compare_en_strings_r, str2, str1, 0);

    const char16_t char_arr3[] = { 'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd', '!' };
    str2 = { char_arr3, sizeof(char_arr3) / sizeof(char_arr3[0]) };
    $test_str_cmp(compare_en_strings, str1, str2, 0);
    $test_str_cmp(compare_en_strings, str2, str1, 0);
    $test_str_cmp(compare_en_strings_r, str1, str2, 0);
    $test_str_cmp(compare_en_strings_r, str2, str1, 0);

    char16_t char_arr4[] = { 'h', 'e', 'l', 'l', '.', '.', '.', '.', '.', '.', '!' };
    str2 = { char_arr4, sizeof(char_arr4) / sizeof(char_arr4[0]) };
    $test_str_cmp(compare_en_strings, str1, str2, 1);
    $test_str_cmp(compare_en_strings, str2, str1, -1);
    $test_str_cmp(compare_en_strings_r, str1, str2, -1);
    $test_str_cmp(compare_en_strings_r, str2, str1, 1);

    char_arr4[1] = 'l';
    $test_str_cmp(compare_en_strings, str1, str2, -1);
    $test_str_cmp(compare_en_strings, str2, str1, 1);

    char_arr4[4] = 'd';
    $test_str_cmp(compare_en_strings_r, str1, str2, 1);
    $test_str_cmp(compare_en_strings_r, str2, str1, -1);

    $testing_result();

    return 0;
}
