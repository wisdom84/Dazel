#include "vulkan_shader.h"
#include "Renderer/vulkan/vulkan_utils.h"
#include "Renderer/vulkan/vulkan_pipeline.h"
#include "Renderer/vulkan/vulkan_buffer.h"
#include "core/logger.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "systems/resource_system.h"
#include "systems/texture_system.h"
#include "systems/shader_system.h"

const u32 DESC_SET_INDEX_GLOBAL = 0;

const u32 DESC_SET_INDEX_INSTANCE = 1;

const u32 BINDING_INDEX_UBO = 0;

const u32 BINDING_INDEX_SAMPLER = 1;

#define FAIL_DESTROY(shader)       \
    vulkan_shader_destroy(shader); \
    return false;

bool create_module(vulkan_shader *shader, vulkan_shader_stage_config config, vulkan_shader_stage_new *shader_stage);
bool uniform_name_valid(vulkan_shader *shader, const char *uniform_name);
bool shader_uniform_add_state_valid(vulkan_shader *shader);
bool uniform_add(vulkan_shader *shader, const char *uniform_name, u32 size, shader_scope scope, u32 *out_location, bool is_sampler);
range get_aligned_range(u64 push_const_size, u32 size, u32 alignment_value);

bool vulkan_shader_create(vulkan_context *context, const char *name, vulkan_renderpass *renderpass, VkShaderStageFlags stages, u32 max_descriptor_set_count, bool use_instance, bool use_local, vulkan_shader *out_shader)
{
    if (!context || !name || !out_shader)
    {
        DERROR("vulkan_shader_create must supply valid pointers to the context or the out shader variable");
        return false;
    }
    if (stages == 0)
    {
        DERROR("vulkan_shader_stage must be a nonzero enum");
        return false;
    }
    Dzero_memory(out_shader, sizeof(vulkan_shader));
    out_shader->context = context;
    out_shader->state = VULKAN_SHADER_STATE_NOT_CREATED;
    out_shader->name = string_duplicate(name);
    out_shader->use_push_constants = use_local;
    out_shader->use_instance = use_instance;
    out_shader->renderpass = renderpass;
    out_shader->config.attribute_stride = 0;
    out_shader->config.push_constant_range_count = 0;
    Dzero_memory(out_shader->push_constant_range, sizeof(range) * VULKAN_SHADER_MAX_PUSH_CONST_RANGES);
    out_shader->bound_instance_id = INVALID_ID;

    out_shader->config.max_desrciptor_set_count = max_descriptor_set_count;
    Dzero_memory(out_shader->config.stages, sizeof(vulkan_shader_stage_config) * VULKAN_SHADER_MAX_STAGE);
    out_shader->config.stage_count = 0;
    for (u32 i = VK_SHADER_STAGE_VERTEX_BIT; i < VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM; i <<= 1)
    {
        if ((stages & i) == i)
        {
            vulkan_shader_stage_config stage_config;
            switch (i)
            {
            case VK_SHADER_STAGE_VERTEX_BIT:
                string_n_copy(stage_config.stage_str, "vert", 7);
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                string_n_copy(stage_config.stage_str, "frag", 7);
                break;
            default:
                DERROR("vulkan_shader_create: Unsupported shader stage flag %d stage ignored", i);
                continue;
            }
            stage_config.stage_flag = (VkShaderStageFlagBits)i;
            if (out_shader->config.stage_count + 1 > VULKAN_SHADER_MAX_STAGE)
            {
                DERROR("shader stages may only have  a maximum of %d stages", VULKAN_SHADER_MAX_STAGE);
                return false;
            }
            out_shader->config.stages[out_shader->config.stage_count] = stage_config;
            out_shader->config.stage_count++;
        }
    }

    Dzero_memory(out_shader->config.descriptor_sets, sizeof(vulkan_descriptor_set_config) * 2);
    Dzero_memory(out_shader->global_texture, sizeof(Texture *) * VULKAN_SHADER_MAX_GLOBAL_TEXTURE);
    out_shader->global_texture_count = 0;
    Dzero_memory(out_shader->config.attributes, sizeof(VkVertexInputAttributeDescription) * VULKAN_SHADER_MAX_ATTRIBUTES);
    out_shader->config.attribute_count = 0;
    Dzero_memory(out_shader->uniforms, sizeof(vulkan_uniform_lookup_entry) * VULKAN_SHADER_MAX_UNIFORM);
    out_shader->uniform_count = 0;

    u64 element_size = sizeof(u32);
    u64 element_count = 1024;
    out_shader->hash_table_block = Dallocate_memory(element_size * element_count, MEMORY_TAG_UNKNOWN);
    hash_table_create(element_size, element_count, out_shader->hash_table_block, false, &out_shader->uniform_look_up);

    // invalidate the hash table completely
    u32 invalid = INVALID_ID;
    hash_table_fill(&out_shader->uniform_look_up, &invalid);

    out_shader->global_ubo_size = 0;
    out_shader->ubo_size = 0;
    out_shader->required_ubo_alignment = 256;
    out_shader->push_constant_stride = 128;
    out_shader->push_constant_size = 0;

    out_shader->config.pool_size[0].descriptorCount = 1024;
    out_shader->config.pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    out_shader->config.pool_size[1].descriptorCount = 1024;
    out_shader->config.pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    vulkan_descriptor_set_config global_descriptor_set_config = {};
    global_descriptor_set_config.binding[BINDING_INDEX_UBO].binding = BINDING_INDEX_UBO;
    global_descriptor_set_config.binding[BINDING_INDEX_UBO].descriptorCount = 1;
    global_descriptor_set_config.binding[BINDING_INDEX_UBO].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_descriptor_set_config.binding[BINDING_INDEX_UBO].pImmutableSamplers = nullptr;
    global_descriptor_set_config.binding[BINDING_INDEX_UBO].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    global_descriptor_set_config.binding_count++;

    out_shader->config.descriptor_sets[DESC_SET_INDEX_GLOBAL] = global_descriptor_set_config;
    out_shader->config.descriptor_set_count++;
    if (out_shader->use_instance)
    {
        vulkan_descriptor_set_config instance_descriptor_set_config = {};
        instance_descriptor_set_config.binding[BINDING_INDEX_UBO].binding = BINDING_INDEX_UBO;
        instance_descriptor_set_config.binding[BINDING_INDEX_UBO].descriptorCount = 1;
        instance_descriptor_set_config.binding[BINDING_INDEX_UBO].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        instance_descriptor_set_config.binding[BINDING_INDEX_UBO].pImmutableSamplers = nullptr;
        instance_descriptor_set_config.binding[BINDING_INDEX_UBO].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        instance_descriptor_set_config.binding_count++;

        out_shader->config.descriptor_sets[DESC_SET_INDEX_INSTANCE] = instance_descriptor_set_config;
        out_shader->config.descriptor_set_count++;
    }

    // invalidate all inatances created
    for (u32 i = 0; i < 1024; i++)
    {
        out_shader->instance_state[i].id = INVALID_ID;
    }

    out_shader->state = VULKAN_SHADER_STATE_UNINITIALIZED;
    return true;
}

bool vulkan_shader_destroy(vulkan_shader *shader)
{
    if (!shader)
    {
        DERROR("vulkan_shader_destroy requires a valid pointer to a shader object");
        return false;
    }
    VkDevice device = shader->context->device.logical_device;
    VkAllocationCallbacks *vk_allocator = shader->context->allocator;

    shader->state = VULKAN_SHADER_STATE_NOT_CREATED;
    u32 length = string_length(shader->name);
    Dfree_memory((void *)shader->name, MEMORY_TAG_STRING, length + 1);
    shader->name = nullptr;
    for (u32 i = 0; i < shader->config.descriptor_set_count; i++)
    {
        if (shader->descriptor_set_layout[i])
        {
            vkDestroyDescriptorSetLayout(device, shader->descriptor_set_layout[i], vk_allocator);
            shader->descriptor_set_layout[i] = nullptr;
        }
    }
    if (shader->descriptor_pool)
    {
        vkDestroyDescriptorPool(device, shader->descriptor_pool, vk_allocator);
    }
    // uniform buffers
    buffer_unmap_memory(shader->context, &shader->uniform_buffer);
    shader->mapped_uniform_buffer_block = 0;
    vulkan_buffer_destroy(shader->context, &shader->uniform_buffer);
    // pipeline resources
    vulkan_graphics_pipeline_destroy(shader->context, &shader->pipeline);

    // destroy shader modules
    for (u32 i = 0; i < shader->config.stage_count; i++)
    {
        vkDestroyShaderModule(device, shader->stages[i].handle, vk_allocator);
    }

    // destroy configuration
    Dzero_memory(&shader->config, sizeof(shader_config));

    return true;
}

typedef struct vulkan_format_size
{
    VkFormat format;
    u32 size;
} vulkan_format_size;

bool vulkan_shader_add_attributes(vulkan_shader *shader, const char *name, shader_attribute_type type)
{
    if (!shader || !name)
    {
        DERROR("vulkan_shader_add_attributes requires a valid pointer to shader  and an atrribute name");
        return false;
    }
    static vulkan_format_size *types = nullptr;
    static vulkan_format_size t[29];
    if (type)
    {
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32] = (vulkan_format_size){VK_FORMAT_R32_SFLOAT, 4};
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32_2] = (vulkan_format_size){VK_FORMAT_R32G32_SFLOAT, 8};
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32_3] = (vulkan_format_size){VK_FORMAT_R32G32B32_SFLOAT, 12};
        t[SHADER_ATTRIBUTE_TYPE_FLOAT32_4] = (vulkan_format_size){VK_FORMAT_R32G32B32A32_SFLOAT, 16};
        t[SHADER_ATTRIBUTE_TYPE_INT8] = (vulkan_format_size){VK_FORMAT_R8_SINT, 1};
        t[SHADER_ATTRIBUTE_TYPE_INT8_2] = (vulkan_format_size){VK_FORMAT_R8G8_SINT, 2};
        t[SHADER_ATTRIBUTE_TYPE_INT8_3] = (vulkan_format_size){VK_FORMAT_R8G8B8_SINT, 3};
        t[SHADER_ATTRIBUTE_TYPE_INT8_4] = (vulkan_format_size){VK_FORMAT_R8G8B8A8_SINT, 4};
        t[SHADER_ATTRIBUTE_TYPE_UINT8] = (vulkan_format_size){VK_FORMAT_R8_UINT, 1};
        t[SHADER_ATTRIBUTE_TYPE_UINT8_2] = (vulkan_format_size){VK_FORMAT_R8G8_UINT, 2};
        t[SHADER_ATTRIBUTE_TYPE_UINT8_3] = (vulkan_format_size){VK_FORMAT_R8G8B8_UINT, 3};
        t[SHADER_ATTRIBUTE_TYPE_UINT8_4] = (vulkan_format_size){VK_FORMAT_R8G8B8A8_UINT, 4};
        t[SHADER_ATTRIBUTE_TYPE_INT16] = (vulkan_format_size){VK_FORMAT_R16_SINT, 2};
        t[SHADER_ATTRIBUTE_TYPE_INT16_2] = (vulkan_format_size){VK_FORMAT_R16G16_SINT, 4};
        t[SHADER_ATTRIBUTE_TYPE_INT16_3] = (vulkan_format_size){VK_FORMAT_R16G16B16_SINT, 6};
        t[SHADER_ATTRIBUTE_TYPE_INT16_4] = (vulkan_format_size){VK_FORMAT_R16G16B16A16_SINT, 8};
        t[SHADER_ATTRIBUTE_TYPE_UINT16] = (vulkan_format_size){VK_FORMAT_R16_UINT, 2};
        t[SHADER_ATTRIBUTE_TYPE_UINT16_2] = (vulkan_format_size){VK_FORMAT_R16G16_UINT, 4};
        t[SHADER_ATTRIBUTE_TYPE_UINT16_3] = (vulkan_format_size){VK_FORMAT_R16G16B16_UINT, 6};
        t[SHADER_ATTRIBUTE_TYPE_UINT16_4] = (vulkan_format_size){VK_FORMAT_R16G16B16A16_UINT, 8};
        t[SHADER_ATTRIBUTE_TYPE_INT32] = (vulkan_format_size){VK_FORMAT_R32_SINT, 4};
        t[SHADER_ATTRIBUTE_TYPE_INT32_2] = (vulkan_format_size){VK_FORMAT_R32G32_SINT, 8};
        t[SHADER_ATTRIBUTE_TYPE_INT32_3] = (vulkan_format_size){VK_FORMAT_R32G32B32_SINT, 12};
        t[SHADER_ATTRIBUTE_TYPE_INT32_4] = (vulkan_format_size){VK_FORMAT_R32G32B32A32_SINT, 16};
        t[SHADER_ATTRIBUTE_TYPE_UINT32] = (vulkan_format_size){VK_FORMAT_R32_UINT, 4};
        t[SHADER_ATTRIBUTE_TYPE_UINT32_2] = (vulkan_format_size){VK_FORMAT_R32G32_UINT, 8};
        t[SHADER_ATTRIBUTE_TYPE_UINT32_3] = (vulkan_format_size){VK_FORMAT_R32G32B32_UINT, 12};
        t[SHADER_ATTRIBUTE_TYPE_UINT32_4] = (vulkan_format_size){VK_FORMAT_R32G32B32A32_UINT, 16};

        types = t;
    }
    // set up new attribute
    VkVertexInputAttributeDescription attribute;
    attribute.location = shader->config.attribute_count;
    attribute.binding = 0;
    attribute.format = types[type].format;
    attribute.offset = shader->config.attribute_stride;

    shader->config.attributes[shader->config.attribute_count] = attribute;
    shader->config.attribute_count++;
    shader->config.attribute_stride += types[type].size;
    return true;
};
bool vulkan_shader_add_sampler(vulkan_shader *shader, const char *sampler_name, shader_scope scope, u32 *out_location)
{
    if (scope == SHADER_SCOPE_INSTANCE && !shader->use_instance)
    {
        DERROR("vulkan_shader_add_sampler can not add sampler to an instance sampler for shader that does not use instances");
        return false;
    }
    if (scope == SHADER_SCOPE_LOCAL)
    {
        DERROR("vulkan_shader_add_sampler can not add sampler to a local scope");
        return false;
    }

    if (!uniform_name_valid(shader, sampler_name) || !shader_uniform_add_state_valid(shader))
    {
        return false;
    }

    const u32 set_index = (scope == SHADER_SCOPE_GLOBAL ? DESC_SET_INDEX_GLOBAL : DESC_SET_INDEX_INSTANCE);
    vulkan_descriptor_set_config *set_config = &shader->config.descriptor_sets[set_index];
    if (set_config->binding_count < 2)
    {
        set_config->binding[BINDING_INDEX_SAMPLER].binding = BINDING_INDEX_SAMPLER;
        set_config->binding[BINDING_INDEX_SAMPLER].descriptorCount = 1;
        set_config->binding[BINDING_INDEX_SAMPLER].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        set_config->binding[BINDING_INDEX_SAMPLER].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        set_config->binding_count++;

        *out_location = 0;
    }
    else
    {
        *out_location = set_config->binding[BINDING_INDEX_SAMPLER].descriptorCount;
        set_config->binding[BINDING_INDEX_SAMPLER].descriptorCount++;
    }

    if (scope == SHADER_SCOPE_GLOBAL)
    {
        shader->global_texture[shader->global_texture_count] = texture_system_get_defualt_texture();
        shader->global_texture_count++;
    }
    else
    {
        shader->instance_texture_count++;
    }

    if (!uniform_add(shader, sampler_name, 0, scope, out_location, true))
    {
        DERROR("Unable to add sampler uniform");
        return false;
    }
    return true;
};

// Verify shader state output pointer and uniform name are all valid
#define VERIFY_UNIFORM(shader, uniform_name, out_location)                                                     \
    if (!out_location || !shader_uniform_add_state_valid(shader) || !uniform_name_valid(shader, uniform_name)) \
        return false;

bool vulkan_shader_add_uniform_i16(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(i16), scope, out_location, false);
};

bool vulkan_shader_add_uniform_i32(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(int), scope, out_location, false);
};

bool vulkan_shader_add_uniform_u8(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(u8), scope, out_location, false);
};

bool vulkan_shader_add_uniform_u16(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(u16), scope, out_location, false);
};

bool vulkan_shader_add_uniform_u32(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(u32), scope, out_location, false);
};

bool vulkan_shader_add_uniform_f32(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(float), scope, out_location, false);
};

bool vulkan_shader_add_uniform_vec2(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(vec2), scope, out_location, false);
};

bool vulkan_shader_add_uniform_vec3(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(vec3), scope, out_location, false);
};

bool vulkan_shader_add_uniform_vec4(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(vec4), scope, out_location, false);
};

bool vulkan_shader_add_uniform_mat4(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location)
{
    VERIFY_UNIFORM(shader, uniform_name, out_location);
    return uniform_add(shader, uniform_name, sizeof(mat4), scope, out_location, false);
};

bool vulkan_shader_intialize(vulkan_shader *shader)
{
    if (!shader)
    {
        DERROR("vulkan shader initialize requires a valid pointer to a shader");
        return false;
    }
    vulkan_context *context = shader->context;
    VkDevice logical_device = context->device.logical_device;
    VkAllocationCallbacks *vk_Allocator = context->allocator;

    Dzero_memory(shader->stages, sizeof(vulkan_shader_stage) * VULKAN_SHADER_MAX_STAGE);
    for (u32 i = 0; i < shader->config.stage_count; i++)
    {
        if (!create_module(shader, shader->config.stages[i], &shader->stages[i]))
        {
            DERROR("Unable to create %s shader module for %s. shader will be destroyed ", shader->config.stages[i].stage_str, shader->name);
            FAIL_DESTROY(shader);
        }
    }
    VkDescriptorPoolCreateInfo pool_info;
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 2;
    pool_info.pNext = nullptr;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.pPoolSizes = shader->config.pool_size;
    pool_info.maxSets = shader->config.max_desrciptor_set_count;

    VkResult result = vkCreateDescriptorPool(logical_device, &pool_info, vk_Allocator, &shader->descriptor_pool);
    if (!vulkan_result_is_success(result))
    {
        DERROR("vulkan_shader_initialize failed to create descriptor pool: %s", vulkan_result_string(result, true));
        FAIL_DESTROY(shader);
    }
    Dzero_memory(shader->descriptor_set_layout, shader->config.descriptor_set_count);
    for (u32 i = 0; i < shader->config.descriptor_set_count; i++)
    {
        VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        layout_info.bindingCount = shader->config.descriptor_sets[i].binding_count;
        layout_info.pBindings = shader->config.descriptor_sets[i].binding;
        layout_info.pNext = nullptr;
        layout_info.flags = 0;
        result = vkCreateDescriptorSetLayout(logical_device, &layout_info, vk_Allocator, &shader->descriptor_set_layout[i]);
        if (!vulkan_result_is_success(result))
        {
            DERROR("vulkan_shader_initialize failed to create descriptor set layout: %s", vulkan_result_string(result, true));
            FAIL_DESTROY(shader);
        }
    }

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (float)shader->context->framebuffer_height;
    viewport.width = (float)shader->context->framebuffer_width;
    viewport.height = -(float)shader->context->framebuffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = shader->context->framebuffer_width;
    scissor.extent.height = shader->context->framebuffer_height;

    VkPipelineShaderStageCreateInfo stage_create_infos[VULKAN_SHADER_MAX_STAGE];
    Dzero_memory(stage_create_infos, sizeof(VkPipelineShaderStageCreateInfo) * VULKAN_SHADER_MAX_STAGE);
    for (u32 i = 0; i < shader->config.stage_count; i++)
    {
        stage_create_infos[i] = shader->stages[i].shader_satge_create_info;
    }
    bool pipeline_result = vulkan_graphics_pipeline_create(
        context,
        shader->renderpass,
        shader->config.attribute_stride,
        shader->config.attribute_count,
        shader->config.attributes,
        shader->descriptor_set_layout,
        shader->config.descriptor_set_count,
        shader->config.stage_count,
        stage_create_infos,
        viewport,
        scissor,
        false,
        true,
        &shader->pipeline);
    if (!pipeline_result)
    {
        DERROR("failed to load graphics pipeline for object shader");
        return false;
    }

    shader->global_ubo_stride = 0;
    while (shader->global_ubo_stride < shader->global_ubo_size)
    {
        shader->global_ubo_stride += shader->required_ubo_alignment;
    }

    if (shader->use_instance)
    {
        shader->ubo_stride = 0;
        while (shader->ubo_stride < shader->ubo_size)
        {
            shader->ubo_stride += shader->required_ubo_alignment;
        }
    }
    // uniform buffer
    u32 device_local_bits = context->device.supports_device_local_host_visible ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
    u64 total_buffer_size = shader->global_ubo_stride + (shader->ubo_size * VULKAN_OBJECT_MAX_COUNT);
    if (!vulkan_buffer_create(
            context,
            total_buffer_size,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | device_local_bits,
            true,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            &shader->uniform_buffer))
    {
        DERROR("vulkan buffer creation failed for object shader");
        return false;
    }

    // Global Ubo
    if (!vulkan_buffer_allocate(&shader->uniform_buffer, shader->global_ubo_stride, &shader->global_ubo_offset))
    {
        DERROR("failed to allocate space for the uniform buffer");
        return false;
    }

    shader->mapped_uniform_buffer_block = buffer_map_memory(shader->context, &shader->uniform_buffer, 0, total_buffer_size, 0);
    // Allocate global descriptor sets
    VkDescriptorSetLayout global_layouts[3] = {
        shader->descriptor_set_layout[DESC_SET_INDEX_GLOBAL],
        shader->descriptor_set_layout[DESC_SET_INDEX_GLOBAL],
        shader->descriptor_set_layout[DESC_SET_INDEX_GLOBAL]};
    VkDescriptorSetAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.descriptorPool = shader->descriptor_pool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.pSetLayouts = global_layouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device.logical_device, &alloc_info, shader->global_descriptor_set));
    shader->state = VULKAN_SHADER_STATE_INITIALIZED;
    return true;
};

bool vulkan_shader_use(vulkan_shader *shader)
{
    u32 image_index = shader->context->image_index;
    vulkan_bind_pipeline(&shader->context->graphics_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
    return true;
};
bool vulkan_shader_bind_globals(vulkan_shader *shader)
{
    if (!shader)
    {
        return false;
    }
    shader->bound_ubo_offset = shader->global_ubo_offset;
    return true;
};

bool vulkan_shader_bind_instance(vulkan_shader *shader, u32 instance_id)
{
    if (!shader)
    {
        DERROR("vulkan_shader_bind_instance requires a valid pointer to a shader");
        return false;
    }

    shader->bound_instance_id = instance_id;
    vulkan_shader_instance_state *object_state = &shader->instance_state[instance_id];
    shader->bound_ubo_offset = object_state->offset;
    return true;
};

bool vulkan_shader_apply_globals(vulkan_shader *shader)
{
    vulkan_context *context = shader->context;
    u32 image_index = context->image_index;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;
    VkDescriptorSet global_descriptor = shader->global_descriptor_set[image_index];

    // Apply UBO
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shader->uniform_buffer.handle;
    bufferInfo.offset = shader->global_ubo_offset;
    bufferInfo.range = shader->global_ubo_stride;
    // Update descriptor sets
    VkWriteDescriptorSet ubo_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    ubo_write.pNext = nullptr;
    ubo_write.dstSet = shader->global_descriptor_set[image_index];
    ubo_write.dstArrayElement = 0;
    ubo_write.dstBinding = 0;
    ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_write.descriptorCount = 1;
    ubo_write.pBufferInfo = &bufferInfo;
    ubo_write.pImageInfo = nullptr;
    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = ubo_write;

    u32 global_set_binding_count = shader->config.descriptor_sets[DESC_SET_INDEX_GLOBAL].binding_count;
    if (global_set_binding_count > 1)
    {
        global_set_binding_count = 1;
        DERROR("Global image samplers are not yet supported");
    }

    vkUpdateDescriptorSets(context->device.logical_device, global_set_binding_count, descriptor_writes, 0, 0);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout_handle, 0, 1, &global_descriptor, 0, 0);
    return true;
};

bool vulkan_shader_apply_instance(vulkan_shader *shader)
{
    if (!shader->use_instance)
    {
        DERROR("this shader does not use an instance");
        return false;
    }

    vulkan_context *context = shader->context;
    u32 image_index = context->image_index;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;

    vulkan_shader_instance_state *object_state = &shader->instance_state[shader->bound_instance_id];
    VkDescriptorSet object_descriptor_set = object_state->descriptor_set_state.descriptor_set[image_index];

    VkWriteDescriptorSet descriptor_writes[2];
    Dzero_memory(descriptor_writes, sizeof(VkWriteDescriptorSet) * 2);
    u32 descriptor_count = 0;
    u32 descriptor_index = 0;

    u32 *instance_ubo_generation = &(object_state->descriptor_set_state.descriptor_state[descriptor_index].generations[image_index]);
    if (*instance_ubo_generation == INVALID_ID)
    {
        VkDescriptorBufferInfo buffer_info;
        buffer_info.buffer = shader->uniform_buffer.handle;
        buffer_info.offset = object_state->offset;
        buffer_info.range = shader->ubo_stride;

        VkWriteDescriptorSet ubo_descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        ubo_descriptor.pNext = nullptr;
        ubo_descriptor.dstSet = object_descriptor_set;
        ubo_descriptor.dstArrayElement = 0;
        ubo_descriptor.dstBinding = descriptor_index;
        ubo_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_descriptor.descriptorCount = 1;
        ubo_descriptor.pBufferInfo = &buffer_info;
        ubo_descriptor.pImageInfo = nullptr;

        descriptor_writes[descriptor_count] = ubo_descriptor;
        descriptor_count++;

        *instance_ubo_generation = 1;
    }
    descriptor_index++;

    if (shader->config.descriptor_sets[DESC_SET_INDEX_INSTANCE].binding_count > 1)
    {
        u32 total_sample_count = shader->config.descriptor_sets[DESC_SET_INDEX_INSTANCE].binding[BINDING_INDEX_SAMPLER].descriptorCount;
        u32 update_sample_count = 0;
        VkDescriptorImageInfo image_infos[VULKAN_SHADER_MAX_GLOBAL_TEXTURE];
        for (u32 i = 0; i < total_sample_count; i++)
        {
            Texture *t = shader->instance_state[shader->bound_instance_id].instance_texture[i];
            vulkan_texture_data *internal_data = (vulkan_texture_data *)t->internal_data;
            image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_infos[i].imageView = internal_data->image.view;
            image_infos[i].sampler = internal_data->sampler;

            update_sample_count++;
        }

        VkWriteDescriptorSet sampler_descriptor;
        sampler_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        sampler_descriptor.pNext = nullptr;
        sampler_descriptor.descriptorCount = update_sample_count;
        sampler_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_descriptor.dstArrayElement = 0;
        sampler_descriptor.dstBinding = descriptor_index;
        sampler_descriptor.dstSet = object_descriptor_set;
        sampler_descriptor.pBufferInfo = nullptr;
        sampler_descriptor.pImageInfo = image_infos;
        sampler_descriptor.pTexelBufferView = nullptr;

        descriptor_writes[descriptor_count] = sampler_descriptor;
        descriptor_count++;
    }

    if (descriptor_count > 0)
    {
        vkUpdateDescriptorSets(context->device.logical_device, descriptor_count, descriptor_writes, 0, 0);
    }
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout_handle, 1, 1, &object_descriptor_set, 0, 0);
    return true;
};

bool vulkan_shader_acquire_instance_resources(vulkan_shader *shader, u32 *out_instance_id)
{
    *out_instance_id = INVALID_ID;
    for (u32 i = 0; i < 1024; i++)
    {
        if (shader->instance_state[i].id == INVALID_ID)
        {
            shader->instance_state[i].id = i;
            *out_instance_id = i;
            break;
        }
    }
    if (*out_instance_id == INVALID_ID)
    {
        DERROR("vulkan shader acquire resource failed to acquire new id");
        return false;
    }
    vulkan_shader_instance_state *instance_state = &shader->instance_state[*out_instance_id];
    u32 instance_texture_count = shader->config.descriptor_sets[DESC_SET_INDEX_INSTANCE].binding[BINDING_INDEX_SAMPLER].descriptorCount;
    Dzero_memory(instance_state->instance_texture, sizeof(Texture *) * VULKAN_SHADER_MAX_INSTANCE_TEXTURE);
    Texture *default_texture = texture_system_get_defualt_texture();
    for (u32 i = 0; i < instance_texture_count; i++)
    {
        instance_state->instance_texture[i] = default_texture;
    }

    u64 size = shader->ubo_size;
    if (!vulkan_buffer_allocate(&shader->uniform_buffer, size, &instance_state->offset))
    {
        DERROR("vulkan_material_shader_acquire_resources failed to acquire the ubo space");
        return false;
    }
    vulkan_descriptor_set_state *set_state = &instance_state->descriptor_set_state;

    u32 binding_count = shader->config.descriptor_sets[DESC_SET_INDEX_INSTANCE].binding_count;
    Dzero_memory(set_state->descriptor_state, sizeof(vulkan_descriptor_state) * VULKAN_SHADER_MAX_BINDINGS);
    for (u32 i = 0; i < binding_count; i++)
    {
        for (u32 j = 0; j < 3; j++)
        {
            set_state->descriptor_state[i].generations[j] = INVALID_ID;
            set_state->descriptor_state[i].id[j] = INVALID_ID;
        }
    }

    // Allocate descriptor sets
    VkDescriptorSetLayout layouts[3] = {shader->descriptor_set_layout[DESC_SET_INDEX_INSTANCE],
                                        shader->descriptor_set_layout[DESC_SET_INDEX_INSTANCE],
                                        shader->descriptor_set_layout[DESC_SET_INDEX_INSTANCE]};
    VkDescriptorSetAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.descriptorSetCount = 3;
    alloc_info.descriptorPool = shader->descriptor_pool;
    alloc_info.pSetLayouts = layouts;
    VkResult result = vkAllocateDescriptorSets(shader->context->device.logical_device, &alloc_info, instance_state->descriptor_set_state.descriptor_set);
    if (result != VK_SUCCESS)
    {
        DERROR("failed to allocate descriptor sets in shader");
        return false;
    }

    return true;
}

bool vulkan_shader_release_instance_resources(vulkan_shader *shader, u32 instance_id)
{
    vulkan_shader_instance_state *instance_state = &shader->instance_state[instance_id];
    vkDeviceWaitIdle(shader->context->device.logical_device);

    VkResult result = vkFreeDescriptorSets(shader->context->device.logical_device, shader->descriptor_pool, 1, instance_state->descriptor_set_state.descriptor_set);
    if (result != VK_SUCCESS)
    {
        DERROR("failed to free descriptor sets from the descriptor pool");
        return false;
    }
    Dzero_memory(instance_state->descriptor_set_state.descriptor_state, sizeof(vulkan_descriptor_state) * VULKAN_SHADER_MAX_BINDINGS);

    Dzero_memory(instance_state->instance_texture, sizeof(Texture *) * VULKAN_SHADER_MAX_INSTANCE_TEXTURE);

    vulkan_buffer_free(&shader->uniform_buffer, shader->ubo_stride, instance_state->offset);
    instance_state->offset = INVALID_ID;
    instance_state->id = INVALID_ID;
    return true;
}

u32 vulkan_shader_uniform_location(vulkan_shader *shader, const char *uniform_name)
{
    u32 location = INVALID_ID;
    if (!hash_table_get(&shader->uniform_look_up, uniform_name, &location) || location == INVALID_ID)
    {
        DERROR("shader %s does not have a registered uniform name %s", shader->name, uniform_name);
        return INVALID_ID;
    }
    return location;
};

bool vulkan_shader_set_sampler(vulkan_shader *shader, u32 location, Texture *t)
{
    vulkan_uniform_lookup_entry *entry = &shader->uniforms[location];
    if (entry->scope == SHADER_SCOPE_GLOBAL)
    {
        shader->global_texture[entry->location] = t;
    }
    else
    {
        shader->instance_state[shader->bound_instance_id].instance_texture[entry->location] = t;
    }
    return true;
}

bool check_uniform_size(vulkan_shader *shader, u32 location, u32 expected_size)
{
    vulkan_uniform_lookup_entry *entry = &shader->uniforms[location];
    if (entry->size != expected_size)
    {
        DERROR("uniform location %i on shader %s is a different size of (%i) than expected (%i)", location, shader->name, entry->size, expected_size);
        return false;
    }
    return true;
}

bool set_uniform(vulkan_shader *shader, u32 location, void *value, u32 size)
{
    if (!check_uniform_size(shader, location, size))
    {
        return false;
    }
    void *block = 0;
    vulkan_uniform_lookup_entry *entry = &shader->uniforms[location];
    if (entry->scope == SHADER_SCOPE_GLOBAL)
    {
        block = (void *)((char *)shader->mapped_uniform_buffer_block + shader->global_ubo_offset + entry->offset);
    }
    else if (entry->scope == SHADER_SCOPE_INSTANCE)
    {
        block = (void *)((char *)shader->mapped_uniform_buffer_block + shader->bound_ubo_offset + entry->offset);
    }
    else
    {
        VkCommandBuffer command_buffer = shader->context->graphics_command_buffers[shader->context->image_index].handle;
        vkCmdPushConstants(command_buffer, shader->pipeline.layout_handle, VK_SHADER_STAGE_VERTEX_BIT, entry->offset, entry->size, value);
        return true;
    }
    Dmemory_copy(block, value, size);
    return true;
}

bool vulkan_shader_set_uniform_i16(vulkan_shader *shader, u32 location, i16 value)
{
    u32 size = sizeof(i16);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_i32(vulkan_shader *shader, u32 location, int value)
{
    u32 size = sizeof(int);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_u8(vulkan_shader *shader, u32 location, u8 value)
{
    u32 size = sizeof(u8);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_u16(vulkan_shader *shader, u32 location, u16 value)
{
    u32 size = sizeof(u16);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_u32(vulkan_shader *shader, u32 location, u32 value)
{
    u32 size = sizeof(u32);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_f32(vulkan_shader *shader, u32 location, float value)
{
    u32 size = sizeof(float);
    return set_uniform(shader, location, &value, size);
}

bool vulkan_shader_set_uniform_vec2(vulkan_shader *shader, u32 location, vec2 value)
{
    u32 size = sizeof(vec2);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_vec2f(vulkan_shader *shader, u32 location, float value_0, float value_1)
{
    u32 size = sizeof(vec2);
    vec2 value = (vec2){value_0, value_1};
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_vec3(vulkan_shader *shader, u32 location, vec3 value)
{
    u32 size = sizeof(vec3);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_vec3f(vulkan_shader *shader, u32 location, float value_0, float value_1, float value_2)
{
    u32 size = sizeof(vec3);
    vec3 value = (vec3){value_0, value_1, value_2};
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_vec4(vulkan_shader *shader, u32 location, vec4 value)
{
    u32 size = sizeof(vec4);
    return set_uniform(shader, location, &value, size);
};

bool vulkan_shader_set_uniform_vec4f(vulkan_shader *shader, u32 location, float value_0, float value_1, float value_2, float value_3)
{
    u32 size = sizeof(vec4);
    vec4 value = (vec4){value_0, value_1, value_2, value_3};
    return set_uniform(shader, location, &value, size);
};
bool vulkan_shader_set_uniform_mat4(vulkan_shader *shader, u32 location, mat4 value)
{
    u32 size = sizeof(mat4);
    return set_uniform(shader, location, &value, size);
};

bool create_module(vulkan_shader *shader, vulkan_shader_stage_config config, vulkan_shader_stage_new *shader_stage)
{
    char file_name[512];
    string_format(file_name, "shader_bin/%s.spv", config.stage_str);

    resource binary_resource;
    if (!resource_system_load(file_name, RESOURCE_TYPE_BINARY, &binary_resource))
    {
        DERROR("unable to read shader module. failed to load shader resource %s", file_name);
        return false;
    }
    Dzero_memory(&shader_stage->shader_module_create_info, sizeof(VkShaderModuleCreateInfo));
    shader_stage->shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_stage->shader_module_create_info.flags = 0;
    shader_stage->shader_module_create_info.pNext = nullptr;
    shader_stage->shader_module_create_info.pCode = (u32 *)binary_resource.data;
    shader_stage->shader_module_create_info.codeSize = binary_resource.size_of_data;
    VK_CHECK(vkCreateShaderModule(
        shader->context->device.logical_device,
        &shader_stage->shader_module_create_info,
        shader->context->allocator,
        &shader_stage->handle));

    resource_system_unload(&binary_resource);

    Dzero_memory(&shader_stage->shader_satge_create_info, sizeof(VkPipelineShaderStageCreateInfo));
    shader_stage->shader_satge_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage->shader_satge_create_info.stage = config.stage_flag;
    shader_stage->shader_satge_create_info.module = shader_stage->handle;
    shader_stage->shader_satge_create_info.flags = 0;
    shader_stage->shader_satge_create_info.pNext = nullptr;
    shader_stage->shader_satge_create_info.pSpecializationInfo = nullptr;
    shader_stage->shader_satge_create_info.pName = "main";
    return true;
};

bool uniform_name_valid(vulkan_shader *shader, const char *uniform_name)
{
    if (!uniform_name || !string_length(uniform_name))
    {
        DERROR("uniform must have a valid name");
        return false;
    }
    u32 location;
    if (hash_table_get(&shader->uniform_look_up, uniform_name, &location) && location != INVALID_ID)
    {
        DERROR("A uniform by the name %s already exist on shader %s", uniform_name, shader->name);
        return false;
    }
    return true;
};
bool shader_uniform_add_state_valid(vulkan_shader *shader)
{
    if (shader->state != VULKAN_SHADER_STATE_UNINITIALIZED)
    {
        DERROR("Uniforms may only be added to shaders before initailization");
        return false;
    }
    return true;
};
bool uniform_add(vulkan_shader *shader, const char *uniform_name, u32 size, shader_scope scope, u32 *out_location, bool is_sampler)
{
    if (shader->uniform_count + 1 > VULKAN_SHADER_MAX_UNIFORM)
    {
        DERROR("A shader can only accept a combined  maximum of %i uniforms and samplers at global, instance and local scope", VULKAN_SHADER_MAX_UNIFORM);
        return false;
    }

    vulkan_uniform_lookup_entry entry;
    entry.index = shader->uniform_count;
    entry.scope = scope;
    bool is_global = (scope == SHADER_SCOPE_GLOBAL);
    if (is_sampler)
    {
        entry.location = *out_location;
    }
    else
    {
        entry.location = entry.index;
    }
    if (scope != SHADER_SCOPE_LOCAL)
    {
        entry.set_index = (u32)scope;
        entry.offset = is_sampler ? 0 : is_global ? shader->global_ubo_size
                                                  : shader->ubo_size;
        entry.size = is_sampler ? 0 : size;
    }
    else
    {
        if (entry.scope == SHADER_SCOPE_LOCAL && !shader->use_push_constants)
        {
            DERROR("cannot add a locally-scoped uniform for a shader that does not support loacals");
            return false;
        }

        entry.set_index = INVALID_ID;
        range r = get_aligned_range(shader->push_constant_size, size, 4);
        entry.offset = r.offset;
        entry.size = r.size;
        shader->config.push_constant_range[shader->config.push_constant_range_count] = r;
        shader->config.push_constant_range_count++;
        shader->push_constant_size += r.size;
    }

    if (!hash_table_set(&shader->uniform_look_up, uniform_name, &entry.index))
    {
        DERROR("failed to add uniform");
        return false;
    }
    shader->uniforms[shader->uniform_count] = entry;
    shader->uniform_count++;
    if (!is_sampler)
    {
        if (entry.scope == SHADER_SCOPE_GLOBAL)
        {
            shader->global_ubo_size += entry.size;
        }
        else if (entry.scope == SHADER_SCOPE_INSTANCE)
        {
            shader->ubo_size += entry.size;
        }
    }
    *out_location = entry.index;
    return true;
};

range get_aligned_range(u64 push_const_size, u32 size, u32 alignment_value)
{
    range push_range;
    push_range.offset = 0;
    push_range.size = 64;
    return push_range;
}
