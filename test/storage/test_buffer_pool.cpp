#include "../../src/include/storage/buffer_pool.h"
#include <gtest/gtest.h>

namespace udb
{
    extern void fillPage(char* page_data);

    TEST(test_buffer_pool, test_buffer_pool_init){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool buffer_pool(5, disk_manager);

        ASSERT_EQ(buffer_pool.GetPoolSize(), 5);

        delete disk_manager;
        remove("file.db");
    }

    TEST(test_buffer_pool, test_buffer_pool_new_one_page){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool buffer_pool(5, disk_manager);

        Page* page = buffer_pool.NewPage();
        ASSERT_EQ(page->GetPageId(), static_cast<page_id_t>(0));

        delete disk_manager;
        remove("file.db");
    }

    
    TEST(test_buffer_pool, test_buffer_pool_new_page_overflow){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool buffer_pool(5, disk_manager);

        for(size_t i = 0; i < 7; ++i){
            Page* page = buffer_pool.NewPage();
            ASSERT_EQ(page->GetPageId(), static_cast<page_id_t>(i));
        }

        delete disk_manager;
        remove("file.db");
    }

    TEST(test_buffer_pool, test_buffer_pool_get){
        DiskManager* disk_manager = new DiskManager("file.db");
        BufferPool buffer_pool(5, disk_manager);

        char* content = new char[PAGE_SIZE];
        fillPage(content);

        disk_manager->storePage(0, content);

        Page* page = buffer_pool.GetPage(0);
        ASSERT_EQ(page->GetPageId(), static_cast<page_id_t>(0));
        ASSERT_STREQ(page->GetData(), content);

        delete disk_manager;
        delete[] content;
        remove("file.db");
    }

} // namespace udb
