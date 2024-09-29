#include "image_loader.h"

#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "core/logger.h"
#include "resources/resources.inl"

void resource_unload(struct resource_loader*self, resource*resources, memory_tag tag);