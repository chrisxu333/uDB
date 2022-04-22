#pragma once
#include "include/common/type.h"
#include "include/common/rid.h"
#include "include/storage/buffer_pool/buffer_pool.h"

#include <algorithm>
namespace udb
{
    #define HASHTABLE_HEADER_SIZE 12
    #define HASHTABLE_DATA_SIZE (PAGE_SIZE - HASHTABLE_HEADER_SIZE) / sizeof(std::pair<int, RID>)
    class HashTablePage{
        public:
            HashTablePage() = default;
            ~HashTablePage() = default;
            RID lookup(int key, BufferPool* buffer_pool);
        protected:
            page_id_t page_id_;
            page_id_t next_page_id_;
            int size_;
            std::pair<int, RID> data_[HASHTABLE_DATA_SIZE];
    };
} // namespace udb
