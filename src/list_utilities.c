#include "list_utilities.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void increase_list_capacity(void** data, size_t* capacity, size_t stride, size_t increase_amount) {
    assert(data != NULL && increase_amount > 0 && stride > 0 && capacity != NULL);

    size_t updated_capacity = *capacity + increase_amount;

    void* new_list = calloc(updated_capacity, stride);
    assert(new_list != NULL);
    void* old_list = *data;
    if (old_list != NULL) {
        memcpy(new_list, old_list, *capacity * stride);
        free(old_list);
    }
    *data = new_list;
    *capacity = updated_capacity;
}