#ifndef UDB_LRU_CACHE_H
#define UDB_LRU_CACHE_H
#include "include/common/type.h"
#include <map>
namespace udb
{
    class LruCache{
        public:
            LruCache();
            ~LruCache();

            /**
             * 
             * */
            void insert(frame_id_t frame_id);

            frame_id_t replace();

            bool update(frame_id_t frame_id);

        private:
            // LRU node struct
            struct Frame{
                frame_id_t frame_id_;
                Frame* nxt_frame_;
                Frame* prev_frame_;
                Frame():frame_id_(-1), nxt_frame_(nullptr), prev_frame_(nullptr){}
                Frame(frame_id_t frame_id):frame_id_(frame_id), nxt_frame_(nullptr), prev_frame_(nullptr){}
            };

            /**
             * @param frame_id
             * Move a frame to the front of LRU list.
             * */
            void moveToFront(Frame* frame);

            /**
             * @param frame_id
             * Add a new frame to the front of LRU list.
             * */
            void addFront(Frame* frame);

            /**
             * @return frame_id_t
             * Remove least recent used from tail of LRU list.
             * */
            void removeNode(Frame* frame);  

            Frame* head;
            Frame* tail;

            std::map<frame_id_t, Frame*> lru_map_;
    };
} // namespace udb

#endif