#include "include/storage/index/hash_index.h"

namespace udb
{
    HashIndex::HashIndex(BufferPool* buffer_pool){
        buffer_pool_ = buffer_pool;
        Page* initpage = buffer_pool->NewPage();
        HashTableEntryPage* entry_page = reinterpret_cast<HashTableEntryPage*>(initpage->GetData());
        entry_page->init(initpage->GetPageId());
        initpage->SetDirty();
        buffer_pool->UnPin(initpage->GetPageId());
        header_page_t_ = entry_page->GetPageId();
    }

    void HashIndex::add(int key, RID id){
        int* test_ptr = &key;
        uint32_t result = XXHash32::hash(test_ptr, sizeof(int), 0);
        Page* header_page = buffer_pool_->GetPage(header_page_t_);
        HashTableEntryPage* entry = reinterpret_cast<HashTableEntryPage*>(header_page->GetData());
        entry->insert(result % HASH_HEADER_PAGE_PTR_SIZE, key, id, buffer_pool_);
        buffer_pool_->UnPin(header_page->GetPageId());
    }

    void HashIndex::remove(int key){
        int* test_ptr = &key;
        uint32_t result = XXHash32::hash(test_ptr, sizeof(int), 0);
        Page* header_page = buffer_pool_->GetPage(header_page_t_);
        HashTableEntryPage* entry = reinterpret_cast<HashTableEntryPage*>(header_page->GetData());
        entry->remove(result % HASH_HEADER_PAGE_PTR_SIZE, key, buffer_pool_);
        buffer_pool_->UnPin(header_page->GetPageId());
    }

    RID HashIndex::lookup(int key){
        int* test_ptr = &key;
        uint32_t result = XXHash32::hash(test_ptr, sizeof(int), 0);
        Page* header_page = buffer_pool_->GetPage(header_page_t_);
        HashTableEntryPage* entry = reinterpret_cast<HashTableEntryPage*>(header_page->GetData());
        RID res = entry->lookup(result % HASH_HEADER_PAGE_PTR_SIZE, key, buffer_pool_);
        buffer_pool_->UnPin(header_page->GetPageId());
        return res;
    }
} // namespace udb
