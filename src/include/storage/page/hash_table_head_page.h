#pragma once
#include "include/storage/page/hash_table_page.h"
#include "include/storage/page/hash_table_content_page.h"

namespace udb
{
    class HashTableHeadPage : public HashTablePage{
        public:
            HashTableHeadPage() = default;
            ~HashTableHeadPage() = default;

            void init(page_id_t page_id){
                size_ = 0; 
                page_id_ = page_id;
                next_page_id_ = INVALID_PAGE_ID;
                tail_page_id_ = INVALID_PAGE_ID;
                std::fill_n(data_, HASHTABLE_DATA_SIZE, std::make_pair(-1, INVALID_RID));
            }
            void insert(int key, RID id, BufferPool* buffer_pool){
                if(size_ >= HASHTABLE_DATA_SIZE){
                    if(tail_page_id_ != INVALID_PAGE_ID){
                        Page* tail_page = buffer_pool->GetPage(tail_page_id_);
                        HashTableContentPage* tpage = reinterpret_cast<HashTableContentPage*>(tail_page);
                        page_id_t tail_page_id = tpage->insert(key, id, buffer_pool);
                        buffer_pool->UnPin(tail_page->GetPageId());
                        if(tail_page_id != INVALID_PAGE_ID) tail_page_id_ = tail_page_id;
                    }else{
                        // expand next page;
                        Page* next_page = buffer_pool->NewPage();
                        HashTableContentPage* npage = reinterpret_cast<HashTableContentPage*>(next_page);
                        next_page->SetDirty();
                        npage->init(next_page->GetPageId());
                        next_page_id_ = next_page->GetPageId();
                        tail_page_id_ = next_page->GetPageId();
                        buffer_pool->UnPin(next_page->GetPageId());
                    }
                    
                }
                data_[size_] = std::make_pair(key, id);
                size_++;
            }
            void remove(int key){

            }

        private:
            page_id_t tail_page_id_;
    };
} // namespace udb