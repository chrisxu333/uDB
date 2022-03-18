#include "../include/storage/buffer_pool.h"

namespace udb
{
    BufferPool::BufferPool(size_t pool_size, DiskManager* disk_manager) : pool_size_(pool_size), current_size_(0), disk_manager_(disk_manager), page_id_(0){
        pages_ = new Page[pool_size];
        // initially all frames are in the free_list_.
        for(size_t i = 0; i < pool_size; ++i){
            free_list_.emplace_back(static_cast<frame_id_t>(i));
        }
        // init LRU list.
        head = new Frame();
        tail = new Frame();
        head->nxt_frame_ = tail;
        tail->prev_frame_ = head;

    }
    BufferPool::~BufferPool(){
        delete[] pages_;
        // Delete LRU list.
        while(head != tail){
            Frame* tmp = head;
            head = head->nxt_frame_;
            delete tmp;
        }
        delete tail;
    }
    Page* BufferPool::GetPage(page_id_t page_id){
        auto it = pool_.find(page_id);
        // If found in the pool already, just return it.
        // TODO: Move it to the front of the list.
        if(it != pool_.end()){
            moveToFront(it->second);
            return &pages_[it->second];
        }
        // Cache not hit, needs to allocate space to read the intended page from disk.
        Page* page;
        frame_id_t fid;
        if(!free_list_.empty()){
            fid = free_list_.front();
            free_list_.pop_front();
            page = &pages_[fid];
        }else{
            fid = removeTail();
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
        // add to list
        addFront(fid);
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
        }else{
            fid = removeTail();
        }

        addFront(fid);
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

    void BufferPool::addFront(frame_id_t frame_id){
        Frame* frame = new Frame(frame_id);
        LRU_map_[frame_id] = frame;
        frame->nxt_frame_ = head->nxt_frame_;
        head->nxt_frame_->prev_frame_ = frame;
        head->nxt_frame_ = frame;
        frame->prev_frame_ = head;
    }

    void BufferPool::moveToFront(frame_id_t frame_id){
        Frame* frame = LRU_map_[frame_id];
        frame->prev_frame_->nxt_frame_ = frame->nxt_frame_;
        frame->nxt_frame_->prev_frame_ = frame->prev_frame_;
        frame->nxt_frame_ = head->nxt_frame_;
        head->nxt_frame_->prev_frame_ = frame;
        head->nxt_frame_ = frame;
        frame->prev_frame_ = head;
    } 

    frame_id_t BufferPool::removeTail(){
        Frame* real_tail = tail->prev_frame_->prev_frame_;
        frame_id_t frame_id = tail->prev_frame_->frame_id_;
        tail->prev_frame_ = real_tail;
        delete real_tail->nxt_frame_;
        real_tail->nxt_frame_ = tail;
        LRU_map_.erase(frame_id);
        return frame_id;
    }
} // namespace udb
