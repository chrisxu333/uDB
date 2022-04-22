#pragma once
#include "include/common/type.h"
#include "include/common/sysutil/xxhash32.h"
#include "include/storage/buffer_pool/buffer_pool.h"
#include "include/storage/page/hash_table_entry_page.h"
namespace udb
{
    class HashIndex{
        public:
            HashIndex(BufferPool* buffer_pool);

            void add(int key, RID id);

            void remove(int key);

            RID lookup(int key);
            
        private:
            page_id_t header_page_t_;
            BufferPool* buffer_pool_;
    }; 
} // namespace udb
