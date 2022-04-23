#include "include/storage/index/hash_index.h"
#include <gtest/gtest.h>

namespace udb
{
    TEST(test_hash_index, test_init){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool* buffer_pool = new BufferPool(300, disk_manager);
        HashIndex* hindex = new HashIndex(buffer_pool);

        delete disk_manager;
        delete buffer_pool;
        delete hindex;
        remove("file.db");
    }

    TEST(test_hash_index, test_bulk_add){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool* buffer_pool = new BufferPool(300, disk_manager);
        HashIndex* hindex = new HashIndex(buffer_pool);

        for(size_t i = 0; i < 1000; ++i){
            RID id(i, 0);
            hindex->add(i, id);
        }

        delete disk_manager;
        delete buffer_pool;
        delete hindex;
        remove("file.db");
    }

    TEST(test_hash_index, test_remove){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool* buffer_pool = new BufferPool(300, disk_manager);
        HashIndex* hindex = new HashIndex(buffer_pool);

        for(size_t i = 0; i < 1000; ++i){
            RID id(i, 0);
            hindex->add(i, id);
        }

        hindex->remove(100);

        delete disk_manager;
        delete buffer_pool;
        delete hindex;
        remove("file.db");
    }

    TEST(test_hash_index, test_lookup){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool* buffer_pool = new BufferPool(300, disk_manager);
        HashIndex* hindex = new HashIndex(buffer_pool);

        for(size_t i = 0; i < 1000; ++i){
            RID id(i, 0);
            hindex->add(i, id);
        }

        for(size_t i = 0; i < 1000; ++i){
            RID id = hindex->lookup(i);
            ASSERT_EQ(id.GetPageId(), i);
        }

        delete disk_manager;
        delete buffer_pool;
        delete hindex;
        remove("file.db");
    }
} // namespace udb

