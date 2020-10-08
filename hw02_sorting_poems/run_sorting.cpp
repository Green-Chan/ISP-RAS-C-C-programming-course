#include <string_view>

#include "text_sorting.h"

int main() {
    sort_text("romeo_and_juliet.txt", "romeo_and_juliet_sorted.txt", "romeo_and_juliet_sorted_back.txt", "romeo_and_juliet_origin.txt");
    return 0;
}

