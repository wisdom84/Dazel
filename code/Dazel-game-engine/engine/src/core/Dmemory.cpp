#include "Dmemory.h"
#include "platform/platform.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>
 struct memory_stats{
   u64 total_allocated;
   u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
 };
 static const char * memory_tag_string[MEMORY_TAG_MAX_TAGS]={
  "UNKNOWN",
  "ARRAY",
  "LINEAR_ALLOCATOR",
  "DICT",
  "RING_QUEUE",
  "BST",
  "STRING",
  "APPLICATION",
  "JOB",
  "TEXTURE",
  "MATERIAL_INSTANCE",
  "RENDERER",
  "GAME",
  "TRANSFORM",
  "ENTITY",
  "ENTITY_NODE",
  "SCENE"
  "SHADER"
 };
 static struct memory_stats stats;
 void intialize_memory(){
    platform_zero_memory(&stats, sizeof(stats));
 }
 void shutdown_memory(){

 };
 void* Dallocate_memory(u64 size, memory_tag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        DWARNING("Dallocate using MEMORY_TAG_UNKNOWN. Re-class this allocation ");
    }
    stats.total_allocated +=size;
    stats.tagged_allocations[tag] += size;
    void*block = platform_allocate(size, false);
    platform_zero_memory(block, size);
    return block;
 }
 
 void*Dallocate_aligned_memory(u64 size, u64 aligned_szie, memory_tag tag){
       if(tag == MEMORY_TAG_UNKNOWN){
        DWARNING("Dallocate using MEMORY_TAG_UNKNOWN. Re-class this allocation ");
    }
    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;
    void*block = platform_allocate_aligned_memory(size,aligned_szie);
    platform_zero_memory(block, size);
    return block;
  };
 void Dfree_aligned_memory(void*block,memory_tag tag, u64 size){
     if(tag == MEMORY_TAG_UNKNOWN){
        DWARNING("Dfree_memory using MEMORY_TAG_UNKNOWN. Re-class this allocation ");
    }
     stats.total_allocated -=size;
     stats.tagged_allocations[tag] -= size;
     platform_free_aligned_memory(block);
 };

 void Dfree_memory(void*block, memory_tag tag, u64 size){
      if(tag == MEMORY_TAG_UNKNOWN){
        DWARNING("Dfree_memory using MEMORY_TAG_UNKNOWN. Re-class this allocation ");
    }
     stats.total_allocated -=size;
     stats.tagged_allocations[tag] -= size;
     platform_free(block,'f');
 }
 void*Dzero_memory(void*block, u64 size){
   return platform_zero_memory(block,size);
 };
 void*Dmemory_set(u64 size, void*des, int value){
    return platform_set_memory(des,value,size);
 };
 void*Dmemory_copy(void*des, const void*source, u64 size){
   return platform_copy_memory(des,source,size);
 };
u64 get_memory_alloc_count(){
  return stats.total_allocated;
}
 char* get_memory_usage(){
    const u64 gib= 1024 * 1204 * 1024;
    const u64 mib= 1024 * 1024;
    const u64 kib= 1024;
    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset= strlen(buffer);
    for(int i=0; i<MEMORY_TAG_MAX_TAGS; i++){
        char unit[4]="xib";
        float amount = 1.0f;
        if(stats.tagged_allocations[i] >= gib){
            unit[0]='G';
            amount = stats.tagged_allocations[i]/(float)gib;
        }
         else if(stats.tagged_allocations[i] >= mib){
            unit[0]='M';
            amount = stats.tagged_allocations[i]/(float)mib;
        }
         else if(stats.tagged_allocations[i] >= kib){
            unit[0]='K';
            amount = stats.tagged_allocations[i]/(float)kib;
        }
        else{
            unit[0]='B';
            amount = (float)stats.tagged_allocations[i];
        }
       int length = snprintf(buffer + offset,8000, "%s: %.2f %s\n", memory_tag_string[i],amount,unit);
       offset += length;
    }
    char * out_string  = _strdup(buffer);
    return out_string;

 };