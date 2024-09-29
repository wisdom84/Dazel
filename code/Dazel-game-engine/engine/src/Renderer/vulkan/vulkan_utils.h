#pragma once 
#include "vulkan_types.inl"
const char* vulkan_result_string(VkResult result, bool get_extented);
bool vulkan_result_is_success(VkResult result);
