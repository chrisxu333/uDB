#include "include/storage/page/hash_table_page.h"
#include "include/storage/page/hash_table_content_page.h"

namespace udb
{
    RID HashTablePage::lookup(int key, BufferPool* buffer_pool){
        for(size_t i = 0; i < HASHTABLE_DATA_SIZE; ++i){
            if(data_[i].first == key) return data_[i].second;
        }
        Page* next_page = buffer_pool->GetPage(next_page_id_);
        HashTableContentPage* npage = reinterpret_cast<HashTableContentPage*>(next_page);
        RID res = npage->lookup(key, buffer_pool);
        buffer_pool->UnPin(next_page_id_);
        return res;
    }
} // namespace udb
