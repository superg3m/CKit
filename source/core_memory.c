/*===========================================================
 * File: core_memory.c
 * Date: May 03, 2024
 * Creator: Jovanni Djonaj
===========================================================*/
#include "../include/core_memory.h"
#include "../include/core_logger.h"
#include "../include/platform_services.h"
#include "../include/core_assert.h"

//========================== Begin Types ==========================
typedef enum LogLevel LogLevel;
typedef struct Arena Arena;

void arena_output_allocations(Arena* arena, LogLevel log_level);

// Date: May 14, 2024
// TODO(Jovanni): This should be a hashmap
Arena** arena_vector = NULLPTR; 

internal u64 memory_used = 0; 
internal u64 global_memory_tags[MEMORY_TAG_COUNT];
internal Boolean memory_is_initalized = FALSE;

typedef struct MemoryHeader {
	u32 allocation_size_without_header;
    MemoryTag memory_tag;
} MemoryHeader;

char known_memory_tag_strings[MEMORY_TAG_COUNT][MEMORY_TAG_CHARACTER_LIMIT] = {
    "UNKNOWN      : ",
    "TEMPORARY    : ",
    "INTERNAL     : ",
    "STRING       : ",
    "VECTOR       : ",
    "ARENA        : ",
};
//=========================== End Types ===========================

//************************* Begin Functions *************************
Boolean memory_init() {
  memory_is_initalized = TRUE;

  return memory_is_initalized;
}

void _memory_track_add(MemoryHeader header, MemoryTag memory_tag) {
  	global_memory_tags[MEMORY_TAG_INTERNAL] += sizeof(header);
  	global_memory_tags[memory_tag] += (header.allocation_size_without_header);
  	memory_used += sizeof(header) + header.allocation_size_without_header;
}

void _memory_track_remove(MemoryHeader header, MemoryTag memory_tag) {
  	global_memory_tags[MEMORY_TAG_INTERNAL] -= sizeof(header);
  	global_memory_tags[memory_tag] -= (header.allocation_size_without_header);
  	memory_used -= sizeof(header) + header.allocation_size_without_header;
}

void* MACRO_memory_insert_header(void* data, MemoryHeader header) {
  	((MemoryHeader*)data)[0] = header;
  	memory_byte_advance(data, sizeof(header));
  	return data;
}
#define _memory_insert_header(data, header) data = MACRO_memory_insert_header(data, header);

Boolean memory_tag_is_unknown(MemoryTag memory_tag) {
  	return (memory_tag == MEMORY_TAG_UNKNOWN);
}

Boolean memory_tag_is_valid(MemoryTag memory_tag) {
  	return (memory_tag >= 0 && memory_tag < MEMORY_TAG_COUNT);
}

MemoryHeader* _memory_extract_header(void* data) {
	return &((MemoryHeader*)data)[-1];
}

void memory_arena_register(Arena** arena) {
    vector_push(arena_vector, *arena);
}

void memory_arena_unregister(Arena** arena) {
  	for (int i = 0; i < vector_size(arena_vector); i++) {
  	  	if (arena_vector[i] == *arena) {
  	    	// Date: May 12, 2024
  	    	// TODO(Jovanni): THis is not right logic if you have more than one vector you need a remove at or
  	    	// something else that is index based removal.
  	    	LOG_INFO("Unregistered Arena\n");
  	    	vector_pop(arena_vector, Arena*);

  	    	break;
  	  	}
  	}
}

void memory_arena_vector_free() {
	vector_free(arena_vector);
}

void* memory_allocate(u64 byte_allocation_size, MemoryTag memory_tag) {
  assert_in_function(memory_is_initalized, "memory_allocate: call CKit_init() first\n");

  assert_in_function(byte_allocation_size > 0, "Invalid allocation size zero or below\n");
  assert_in_function(memory_tag_is_valid(memory_tag), "memory_allocate: Memory tag is invalid | value: (%d)\n", memory_tag);
  if (memory_tag_is_unknown(memory_tag)) {
      LOG_WARN("memory_allocate: memory tag unknown\n");
  }

  MemoryHeader header;
  memory_zero(&header, sizeof(header));
  header.allocation_size_without_header = byte_allocation_size; 

  // Date: May 11, 2024
  // TODO(Jovanni): Look at this because this is really odd having to subtract sizeof(header)
  header.memory_tag = memory_tag;

  _memory_track_add(header, memory_tag);

  void* data = platform_allocate(sizeof(header) + header.allocation_size_without_header);
  // Date: May 09, 2024
  // TODO(Jovanni): Technically you are repeating work here
  memory_zero(data, sizeof(header) + header.allocation_size_without_header);
  _memory_insert_header(data, header);

  return data;
}

void* memory_reallocate(void* data, u64 new_byte_allocation_size) {
  	LOG_DEBUG("Reallocation Triggered!\n");
  	assert_in_function(data, "memory_reallocation: Data passed is null\n");

  	MemoryHeader header = *_memory_extract_header(data);
  	u32 old_allocation_size = header.allocation_size_without_header;

  	header.allocation_size_without_header = new_byte_allocation_size;
  	void* ret_data = memory_allocate(new_byte_allocation_size, header.memory_tag);
  	_memory_insert_header(ret_data, header);

  	memory_copy(data, ret_data, old_allocation_size, new_byte_allocation_size);
  	memory_free(data);

  	return ret_data;
}

void* MACRO_memory_free(void* data) {
  	assert_in_function(data, "memory_free: Data passed is null in free\n");
  	const MemoryHeader header = *_memory_extract_header(data);
  	assert_in_function(memory_tag_is_valid(header.memory_tag), "memory_free: memory_tag is not valid\n");

  	_memory_track_remove(header, header.memory_tag);

  	memory_byte_retreat(data, sizeof(header));
  	// Date: May 09, 2024
  	// NOTE(Jovanni): Should I actually do this? Should I zero out the memory I free?
  	memory_zero(data, header.allocation_size_without_header);
  	platform_free(data);
  	data = NULLPTR;
  	return data;
}


void memory_output_allocations(LogLevel log_level) {
    if (memory_used == 0) {
        log_output(log_level, "No memory allocations!\n");
        return;
    }
    char out_message[PLATFORM_CHARACTER_LIMIT];
    char out_message2[PLATFORM_CHARACTER_LIMIT];
    char out_message3[PLATFORM_CHARACTER_LIMIT];
    
    log_output(log_level, "========================\n");
    for (int level = 0; level < MEMORY_TAG_COUNT; level++) {
     	if (global_memory_tags[level] == 0) {
    		continue;
     	} 

     	memory_zero(out_message, sizeof(out_message));
     	memory_zero(out_message2, sizeof(out_message2));
     	memory_zero(out_message3, sizeof(out_message3));

     	sprintf(out_message, "%s", known_memory_tag_strings[level]);
     	sprintf(out_message2, "%lld", global_memory_tags[level]);
     	sprintf(out_message3, "%s%s", out_message, out_message2);
     	log_output(log_level, "%s\n", out_message3);
    }
    log_output(log_level, "========================\n");
}

void memory_output_arena_allocations(LogLevel log_level) {
	for (int i = 0; i < vector_size(arena_vector); i++) {
		arena_output_allocations(arena_vector[i], log_level);
	}
}


//************************** End Functions **************************

//+++++++++++++++++++++++++++ Begin Macros ++++++++++++++++++++++++++

//++++++++++++++++++++++++++++ End Macros +++++++++++++++++++++++++++













