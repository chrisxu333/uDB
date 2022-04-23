#pragma once
#include "include/storage/page/hash_table_page.h"

namespace udb
{
    /**
     *  Layout
     *  ---------------------
     *  |   Page Id         |
     *  ---------------------
     *  |   Current size    |
     *  ---------------------
     *  |   data            |
     *  ---------------------
     * */
    class HashTableContentPage : public HashTablePage{
        public:
            HashTableContentPage() = default;
            ~HashTableContentPage() = default;
            void init(page_id_t page_id){
                size_ = 0; 
                page_id_ = page_id;
                next_page_id_ = INVALID_PAGE_ID;
                std::fill_n(data_, HASHTABLE_DATA_SIZE, std::make_pair(-1, INVALID_RID));
            }
            /**
             * @param key
             * @param id
             * @param buffer_pool
             * @return tail page of the linked list.
             * */
            page_id_t insert(int key, RID id, BufferPool* buffer_pool){
                data_[size_] = std::make_pair(key, id);
                size_++;
                if(size_ >= HASHTABLE_DATA_SIZE){
                    // expand next page;
                    Page* next_page = buffer_pool->NewPage();
                    HashTableContentPage* npage = reinterpret_cast<HashTableContentPage*>(next_page);
                    next_page->SetDirty();
                    npage->init(next_page->GetPageId());
                    next_page_id_ = next_page->GetPageId();
                    buffer_pool->UnPin(next_page->GetPageId());
                    return next_page_id_;
                }
                return INVALID_PAGE_ID;
            }
            void remove(int key){
                for(size_t i = 0; i < size_; ++i){
                    if(data_[i].first == key){
                        data_[i].second = INVALID_RID;
                    }
                }
            }
    }; 
} // namespace udb
