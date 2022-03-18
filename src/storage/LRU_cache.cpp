#include "../include/storage/LRU_cache.h"

namespace udb
{
    LruCache::LruCache(){
        head = new Frame();
        tail = new Frame();
        head->nxt_frame_ = tail;
        tail->prev_frame_ = head;
    }

    LruCache::~LruCache(){
        while(head != tail){
            Frame* tmp = head;
            head = head->nxt_frame_;
            delete tmp;
        }
        delete tail;
    }

    void LruCache::insert(frame_id_t frame_id){
        Frame* frame;
        frame = new Frame(frame_id);
        lru_map_[frame_id] = frame;
        // now do the insert
        addFront(frame);
    }

    frame_id_t LruCache::replace(){
        moveToFront(tail->prev_frame_);
        return tail->prev_frame_->frame_id_;
    }

    bool LruCache::update(frame_id_t frame_id){
        auto it = lru_map_.find(frame_id);
        if(it != lru_map_.end()){
            // move the frame to the front
            moveToFront(it->second);
            return true;
        }
        return false;
    }

    void LruCache::addFront(Frame* frame){
        frame->nxt_frame_ = head->nxt_frame_;
        head->nxt_frame_->prev_frame_ = frame;
        head->nxt_frame_ = frame;
        frame->prev_frame_ = head;
    }

    void LruCache::moveToFront(Frame* frame){
        removeNode(frame);
        addFront(frame);
    } 

    void LruCache::removeNode(Frame* frame){
        frame->prev_frame_->nxt_frame_ = frame->nxt_frame_;
        frame->nxt_frame_->prev_frame_ = frame->prev_frame_;
    }
} // namespace udb
