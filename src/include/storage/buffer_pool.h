#ifndef UDB_BUFFER_POOL_H
#define UDB_BUFFER_POOL_H
#include "../common/type.h"
#include "./page/page.h"
#include "disk_manager.h"
#include "LRU_cache.h"
#include <map>
#include <list>

namespace udb
{
    class BufferPool{
        public:
            /**
             * @param pool_size
             * @param disk_manager
             * Constructor.
             * */
            explicit BufferPool(size_t pool_size, DiskManager* disk_manager);

            ~BufferPool();

            size_t GetPoolSize();

            /**
             * @param page_id
             * @return Page
             * Get the @page by its @page_id.
             * */
            Page* GetPage(page_id_t page_id);

            /**
             * @return Page
             * Allocate a new page on disk.
             * */
            Page* NewPage();

        private:
            /**
             * @return page_id_t
             * Allocate a new page_id.
             * */
            page_id_t ualloc();
            size_t pool_size_;
            size_t current_size_;
            // Array of pages, consider as frames.
            Page* pages_;
            // Hashmap for quick page_id_t to frame_id_t lookup.
            std::map<page_id_t, frame_id_t> pool_;
            // List of free frames
            std::list<frame_id_t> free_list_;
            // Every new page would be assigned this page id and increment this by one. Shared across whole system.
            // TODO: Later on we would move this to a buffer pool manager so that the new page id would be shared
            // across multiple buffer pools.
            page_id_t page_id_; 
            // Disk manager instance
            DiskManager* disk_manager_;
            // Cache instance
            LruCache* lru_cache_;
    };
} // namespace udb

#endif