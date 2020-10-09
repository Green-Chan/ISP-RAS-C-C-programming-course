#include <string_view>

#include "text_sorting.h"

int main() {
    sort_text("romeo_and_juliet.txt", "romeo_and_juliet_sorted.txt", "romeo_and_juliet_sorted_back.txt", "romeo_and_juliet_origin.txt", ENGLISH);
    sort_text("eugene_onegin.txt", "eugene_onegin_sorted.txt", "eugene_onegin_sorted_back.txt", "eugene_onegin_origin.txt", RUSSIAN);
    return 0;
}

