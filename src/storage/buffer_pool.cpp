#include "../include/storage/buffer_pool.h"

namespace udb
{
    BufferPool::BufferPool(size_t pool_size, DiskManager* disk_manager) : 
            pool_size_(pool_size), 
            current_size_(0), 
            disk_manager_(disk_manager), 
            page_id_(0){
        pages_ = new Page[pool_size];
        // initially all frames are in the free_list_.
        for(size_t i = 0; i < pool_size; ++i){
            free_list_.emplace_back(static_cast<frame_id_t>(i));
        }
        lru_cache_ = new LruCache();
    }

    BufferPool::~BufferPool(){
        delete[] pages_;
        delete lru_cache_;
    }

    Page* BufferPool::GetPage(page_id_t page_id){
        auto it = pool_.find(page_id);
        // If found in the pool already, just return it.
        if(it != pool_.end()){
            // move to front
            lru_cache_->update(it->second);
            return &pages_[it->second];
        }
        // Cache not hit, needs to allocate space to read the intended page from disk.
        Page* page;
        frame_id_t fid;
        if(!free_list_.empty()){
            // get new frame from free_list.
            fid = free_list_.front();
            free_list_.pop_front();
            page = &pages_[fid];
            lru_cache_->insert(fid);
        }else{
            fid = lru_cache_->replace();
            page = &pages_[fid];
        }
        // Not cached, needs to get it from the disk.
        // Save mapping relation in the pool.
        pool_[page_id] = fid;
        // update fields of the page.
        page->page_id_ = page_id;
        page->dirty_bit_ = false;
        page->pin_count_ = 0;

        // Load the page from disk.
        disk_manager_->loadPage(page_id, page->data_);
        return page;
    }

    Page* BufferPool::NewPage(){
        Page* page;
        frame_id_t fid;

        page_id_t page_id = ualloc();
        // Find a frame for it.
        if(!free_list_.empty()){
            fid = free_list_.front();
            free_list_.pop_front();
            page = &pages_[fid];
            lru_cache_->insert(fid);
        }else{
            fid = lru_cache_->replace();
            page = &pages_[fid];
        }

        pool_[page_id] = fid;
        page->page_id_ = page_id;
        page->dirty_bit_ = false;
        page->pin_count_ = 0;
        page->meminit();

        disk_manager_->storePage(page_id, page->data_);

        return page;
    }

    size_t BufferPool::GetPoolSize(){
        return pool_size_;
    }

    page_id_t BufferPool::ualloc(){
        return page_id_++;
    }
} // namespace udb
