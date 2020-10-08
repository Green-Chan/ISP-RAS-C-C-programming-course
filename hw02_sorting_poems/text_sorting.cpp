
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <cctype>
#include <string>
#include <string_view>
#include <windows.h>
#include <vector>
#include <cstdio>

#include "text_sorting.h"
#include "qsort.h"


std::string GetLastErrorAsString()
{
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

std::vector< std::basic_string_view<char16_t> > data_to_strings(const char16_t *file_data, uint32_t file_size)
{
    assert(file_data);
    size_t file_data_size = file_size / sizeof(file_data[0]);

    size_t string_num = 1; // the last string ends with EOF not "\r\n"
    for (size_t i = 0; i < file_data_size; i++) {
        if (file_data[i] == '\r') {
            if (i + 1 < file_data_size && file_data[i + 1] == '\n') {
                string_num++;
                i++; // skipping '\n'
            } else {
                throw std::invalid_argument("data_to_strings: in file file_in_path there is '\r' which is not belong to \"\r\n\"");
            }
        } else if (file_data[i] == '\n'){
            throw std::invalid_argument("data_to_strings: in file file_in_path there is '\n' which is not belong to \"\r\n\"");
        } else if (file_data[i] == '\0') {
            throw std::invalid_argument("data_to_strings: in file file_in_path there is '\0'");
        }
    }

    std::vector< std::basic_string_view<char16_t> > string_vec(string_num);

    { //initializing string_vec
        size_t cur_char = 0, cur_string = 0, cur_string_begin_char = 0;
        if (file_data[0] == 0xfeff) { // skipping Byte Order Mark
            cur_char++;
            cur_string_begin_char = cur_char;
            for (; cur_char < file_data_size; cur_char++) {
                if(file_data[cur_char] == '\r') {
                    string_vec[cur_string++] = {&file_data[cur_string_begin_char], cur_char - cur_string_begin_char};
                    cur_char++; // skipping '\n'
                    cur_string_begin_char = cur_char + 1;
                }
            }
            string_vec[cur_string++] = {&file_data[cur_string_begin_char], cur_char - cur_string_begin_char};
            assert(cur_string == string_num);
        } else if (file_data[0] == 0xfffe) {
            throw std::invalid_argument("data_to_strings: file has incorrect endianness");
        } else {
            throw std::invalid_argument("data_to_strings: file has no byte order mask");
        }
    }

    return string_vec;
}

bool is_en_char_dig(char16_t c) {
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'));
}

int compare_en_strings(const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2) {
    size_t i1 = 0, i2 = 0;
    while (i1 < str1.size() && i2 < str2.size()) {
        while(i1 < str1.size() && !is_en_char_dig(str1[i1])) {
            i1++;
        }
        while(i2 < str2.size() && !is_en_char_dig(str2[i2])) {
            i2++;
        }
        if (i1 < str1.size() && i2 < str2.size()) {
            if (std::tolower(str1[i1]) < std::tolower(str2[i2])) {
                return -1;
            } else if (std::tolower(str1[i1]) > std::tolower(str2[i2])) {
                return 1;
            } else {
                i1++;
                i2++;
            }
        }
    }
    while(i1 < str1.size() && !is_en_char_dig(str1[i1])) {
        i1++;
    }
    while(i2 < str2.size() && !is_en_char_dig(str2[i2])) {
        i2++;
    }
    if (i1 == str1.size()) {
        if (i2 == str2.size()) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return 1;
    }
}

int compare_en_strings_r(const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2) {
    ssize_t i1 = str1.size() - 1, i2 = str2.size() - 1;
    while (i1 >= 0 && i2 >= 0) {
        while(i1 >= 0 && !is_en_char_dig(str1[i1])) {
            i1--;
        }
        while(i2 >= 0 && !is_en_char_dig(str2[i2])) {
            i2--;
        }
        if (i1 >= 0 && i2 >= 0) {
            if (std::tolower(str1[i1]) < std::tolower(str2[i2])) {
                return -1;
            } else if (std::tolower(str1[i1]) > std::tolower(str2[i2])) {
                return 1;
            } else {
                i1--;
                i2--;
            }
        }
    }
    while(i1 >= 0 && !is_en_char_dig(str1[i1])) {
        i1--;
    }
    while(i2 >= 0 && !is_en_char_dig(str2[i2])) {
        i2--;
    }
    if (i1 < 0) {
        if (i2 < 0) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return 1;
    }
}

void print_to_file (FILE *file_out, std::vector< std::basic_string_view<char16_t> > &string_vec, const char *file_name) {
    char16_t endline[2] = {'\r', '\n' };
    if (string_vec.size() > 0) {
        size_t i = 0;
        for (; i < string_vec.size() - 1; i++) {
            size_t written = fwrite((void *)string_vec[i].data(), sizeof(string_vec[i][0]), string_vec[i].size(), file_out);
            if (written != string_vec[i].size()) {
                throw std::runtime_error((std::string)"sort_text: error occurred while writing in" + file_name);
            }
            if (fwrite((void *)endline, sizeof(endline[0]), 2, file_out) != 2) {
                throw std::runtime_error((std::string)"sort_text: error occurred while writing in " + file_name);
            }
        }
        size_t written = fwrite((void *)string_vec[i].data(), sizeof(string_vec[i][0]), string_vec[i].size(), file_out);
        if (written != string_vec[i].size()) {
            throw std::runtime_error((std::string)"sort_text: error occurred while writing in" + file_name);
        }
    }
}

void sort_text(const char *file_in_path, const char *file_out_sorted_path, const char *file_out_sorted_back_path, const char *file_out_origin_path)
{
    HANDLE file_in_handle = CreateFile(file_in_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_in_handle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error((std::string)"sort_text: cannot open file_in_path: " + GetLastErrorAsString());
    }
    uint32_t file_in_size = GetFileSize(file_in_handle, NULL);
    if (file_in_size == INVALID_FILE_SIZE) {
        throw std::runtime_error((std::string)"sort_text: cannot get size of file_in_path: " + GetLastErrorAsString());
    }
    HANDLE file_in_mapping = CreateFileMapping(file_in_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (file_in_mapping == NULL) {
        throw std::runtime_error((std::string)"sort_text: cannot map file_in_path: " + GetLastErrorAsString());
    }
    const char16_t *file_in_data = (const char16_t *)MapViewOfFile(file_in_mapping, FILE_MAP_READ, 0, 0, 0);
    if (file_in_data == NULL) {
        throw std::runtime_error((std::string)"sort_text: cannot map file_in_path: " + GetLastErrorAsString());
    }

    std::vector< std::basic_string_view<char16_t> > string_vec = data_to_strings(file_in_data, file_in_size);

    FILE *file_out = fopen(file_out_sorted_path, "wb");
    if (file_out == nullptr) {
        throw std::runtime_error((std::string)"sort_text: cannot open " + file_out_sorted_path);
    }
    char16_t bom = 0xfeff;
    if (fwrite((void *)&bom, sizeof(bom), 1, file_out) != 1) {
        throw std::runtime_error((std::string)"sort_text: error occurred while writing in " + file_out_sorted_path);
    }

    qsort< std::basic_string_view<char16_t> >(&(string_vec[0]), &(string_vec[0]) + string_vec.size(),
          [](const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2) -> bool
          {
              return compare_en_strings(str1, str2) <= 0;
          });
    print_to_file(file_out, string_vec, file_out_sorted_path);

    if (fclose(file_out) != 0) {
        throw std::runtime_error((std::string)"sort_text: cannot close " + file_out_sorted_path);
    }


    file_out = fopen(file_out_sorted_back_path, "wb");
    if (file_out == nullptr) {
        throw std::runtime_error((std::string)"sort_text: cannot open " + file_out_sorted_back_path);
    }
    if (fwrite((void *)&bom, sizeof(bom), 1, file_out) != 1) {
        throw std::runtime_error((std::string)"sort_text: error occurred while writing in " + file_out_sorted_back_path);
    }

    qsort< std::basic_string_view<char16_t> >(&(string_vec[0]), &(string_vec[0]) + string_vec.size(),
          [](const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2) -> bool
          {
              return compare_en_strings_r(str1, str2) <= 0;
          });
    print_to_file(file_out, string_vec, file_out_sorted_back_path);

    if (fclose(file_out) != 0) {
        throw std::runtime_error((std::string)"sort_text: cannot close " + file_out_sorted_back_path);
    }


    file_out = fopen(file_out_origin_path, "wb");
    if (file_out == nullptr) {
        throw std::runtime_error((std::string)"sort_text: cannot open " + file_out_origin_path);
    }
    if (fwrite((void *)&bom, sizeof(bom), 1, file_out) != 1) {
        throw std::runtime_error((std::string)"sort_text: error occurred while writing in " + file_out_origin_path);
    }

    /* We can just write data from file_in_data to file_out, but that's not interesting. Let's sort */

    qsort< std::basic_string_view<char16_t> >(&(string_vec[0]), &(string_vec[0]) + string_vec.size(),
          [](const std::basic_string_view<char16_t> &str1, const std::basic_string_view<char16_t> &str2) -> bool
          {
              return str1.data() <= str2.data();
          });
    print_to_file(file_out, string_vec, file_out_origin_path);

    if (fclose(file_out) != 0) {
        throw std::runtime_error((std::string)"sort_text: cannot close " + file_out_origin_path);
    }


    if (UnmapViewOfFile((LPCVOID)file_in_data) == 0) {
        throw std::runtime_error("sort_text: cannot unmap file_in_path: " + GetLastErrorAsString());
    }
    if (CloseHandle(file_in_mapping) == 0) {
        throw std::runtime_error("sort_text: cannot unmap file_in_path: " + GetLastErrorAsString());
    }
    if (CloseHandle(file_in_handle) == 0) {
        throw std::runtime_error("sort_text: cannot close file_in_path: " + GetLastErrorAsString());
    }
}
