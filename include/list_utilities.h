#pragma once

#include <stddef.h>
void increase_list_capacity(void** data, size_t* capacity, size_t stride, size_t increase_amount);