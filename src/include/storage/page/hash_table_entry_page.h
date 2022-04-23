#pragma once
#include "include/common/type.h"
#include "include/common/rid.h"
#include "include/storage/buffer_pool/buffer_pool.h"
#include "include/storage/page/hash_table_head_page.h"

#include <algorithm>

namespace udb
{
    /**
     *          Layout
     *  -------------------------
     *  |   page id(4 bytes)    |
     *  -------------------------
     *  |   data array          |
     *  -------------------------
     * */
    #define HASH_HEADER_PAGE_PTR_SIZE (PAGE_SIZE - 4) / sizeof(page_id_t)
    class HashTableEntryPage{
        public:
            HashTableEntryPage() = default;
            ~HashTableEntryPage() = default;
            page_id_t GetPageId(){ return page_id_; }
            void init(page_id_t page_id){ 
                page_id_ = page_id; 
                std::fill_n(data_, HASH_HEADER_PAGE_PTR_SIZE, INVALID_PAGE_ID);  
            }
            void insert(int idx, int key, RID rid, BufferPool* buffer_pool){
                Page* content_page;
                if(data_[idx] == INVALID_PAGE_ID){
                    content_page = buffer_pool->NewPage();
                    data_[idx] = content_page->GetPageId();
                    HashTableHeadPage* cpage = reinterpret_cast<HashTableHeadPage*>(content_page->GetData());
                    content_page->SetDirty();
                    cpage->init(content_page->GetPageId());
                    cpage->insert(key, rid,buffer_pool);
                }else{
                    content_page = buffer_pool->GetPage(data_[idx]);
                    HashTableHeadPage* cpage = reinterpret_cast<HashTableHeadPage*>(content_page->GetData());
                    content_page->SetDirty();
                    cpage->insert(key, rid, buffer_pool);
                }
                buffer_pool->UnPin(content_page->GetPageId());
            }

            void remove(int idx, int key, BufferPool* buffer_pool){
                Page* content_page;
                if(data_[idx] != INVALID_PAGE_ID){
                    content_page = buffer_pool->GetPage(data_[idx]);
                    HashTableHeadPage* cpage = reinterpret_cast<HashTableHeadPage*>(content_page->GetData());
                    content_page->SetDirty();
                    cpage->remove(key);
                }
                buffer_pool->UnPin(content_page->GetPageId());
            }

            RID lookup(int idx, int key, BufferPool* buffer_pool){
                Page* content_page;
                if(data_[idx] != INVALID_PAGE_ID){
                    content_page = buffer_pool->GetPage(data_[idx]);
                    HashTableHeadPage* cpage = reinterpret_cast<HashTableHeadPage*>(content_page->GetData());
                    RID res = cpage->lookup(key, buffer_pool);
                    buffer_pool->UnPin(content_page->GetPageId());
                    return res;
                }else{
                    return INVALID_RID;
                }
            }
        private:
            page_id_t page_id_;
            page_id_t data_[HASH_HEADER_PAGE_PTR_SIZE];
    };
} // namespace udb
