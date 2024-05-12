#pragma once
/*===========================================================
 * File: core_memory.h
 * Date: April 30, 2024
 * Creator: Jovanni Djonaj
===========================================================*/
#include "core_types.h"
#include "core_vector.h"

//========================== Begin Types ==========================
typedef enum LogLevel LogLevel;
typedef struct Arena Arena;

typedef enum MemoryTag {
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_TEMPORARY,
    MEMORY_TAG_INTERNAL,
    MEMORY_TAG_STRING,
    MEMORY_TAG_VECTOR,
    MEMORY_TAG_ARENA,
    MEMORY_TAG_COUNT
} MemoryTag;
//=========================== End Types ===========================

//************************* Begin Functions *************************
#ifdef __cplusplus
extern "C" {
#endif
  Boolean memory_tag_is_valid(MemoryTag memory_tag);
  Boolean memory_tag_is_unknown(MemoryTag memory_tag);

  void memory_arena_register(Arena** arena);
  void memory_arena_unregister(Arena** arena);
  void memory_arena_vector_free();


  void* memory_allocate(u64 number_of_bytes, MemoryTag memory_tag);
  void* memory_reallocate(void* data, u64 new_number_of_bytes);
  void* MACRO_memory_free(void* data);

  Boolean memory_byte_compare(const void* buffer_one, const void* buffer_two, u32 buffer_one_size, u32 buffer_two_size);
  void memory_copy(const void* source, void* destination, u32 source_size, u32 destination_size);
  void memory_zero(void* data, u32 data_size_in_bytes);

  u8* memory_advance_new_ptr(const void* data, u32 size_in_bytes);
  u8* memory_retreat_new_ptr(const void* data, u32 size_in_bytes);
  void* MACRO_memory_byte_advance(const void* data, u32 size_in_bytes);
  void* MACRO_memory_byte_retreat(const void* data, u32 size_in_bytes);

  void memory_tag_output(LogLevel log_level);
  void memory_tag_output_arena(LogLevel log_level);
#ifdef __cplusplus
}
#endif
//************************** End Functions **************************


//+++++++++++++++++++++++++++ Begin Macros ++++++++++++++++++++++++++
#ifdef __cplusplus
  #define memory_free(data) data = (decltype(data))MACRO_memory_free(data);
  #define memory_byte_advance(data, size_in_bytes) data = (decltype(data))MACRO_memory_byte_advance(data, size_in_bytes);
  #define memory_byte_retreat(data, size_in_bytes) data = (decltype(data))MACRO_memory_byte_retreat(data, size_in_bytes);
#else 
  #define memory_free(data) data = MACRO_memory_free(data);
  #define memory_byte_advance(data, size_in_bytes) data = MACRO_memory_byte_advance(data, size_in_bytes);
  #define memory_byte_retreat(data, size_in_bytes) data = MACRO_memory_byte_retreat(data, size_in_bytes);
#endif
//++++++++++++++++++++++++++++ End Macros +++++++++++++++++++++++++++