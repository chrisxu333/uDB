#include "../../include/storage/page/b_tree_internal_page.h"
#include <cassert>

namespace udb
{
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Init(page_id_t page_id, page_id_t parent_id, int max_size){
        page_id_ = page_id;
        parent_page_id_ = parent_id;
        max_size_ = max_size;
        page_type_ = IndexPageType::INTERNAL_PAGE;
        size_ = 0;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    KeyType BTreeInternalPage<KeyType, ValueType, KeyComparator>::KeyAt(int index) const{
        return keys_[index].first;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    ValueType BTreeInternalPage<KeyType, ValueType, KeyComparator>::ValueAt(int index) const{
        return keys_[index].second;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::SetKeyAt(int index, const KeyType& nkey){
        keys_[index].first = nkey;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    ValueType BTreeInternalPage<KeyType, ValueType, KeyComparator>::LookUp(const KeyType& key, const KeyComparator& comparator) const{
        // Version 1: linear search
        if(comparator(KeyAt(1), key) > 0) return ValueAt(0);
        for(size_t i = 1; i < size_; ++i){
            if(comparator(KeyAt(i), key) > 0){
                // find the first key that is larger than or equal to given key, return it
                return ValueAt(i - 1);
            }
        }
        // Ideally, this return should never be called.
        return ValueAt(size_ - 1);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Insert(const KeyType& key, const ValueType& value, BufferPool* buffer_pool, KeyComparator comparator){
        std::pair<KeyType, ValueType> tmp = {key, value};
        keys_[size_] = tmp;
        size_++;
        std::sort(keys_, keys_ + size_, 
            [comparator](const std::pair<KeyType, ValueType>& lhs, const std::pair<KeyType, ValueType>& rhs){
                return comparator(lhs.first, rhs.first) < 0; 
            });
        if(size_ == max_size_ - 1) {
            LOG_DEBUG("Internal split");
            Split(buffer_pool, comparator);
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Split(BufferPool* buffer_pool, KeyComparator comparator){
        // 0. if there's no parent for this page right now, create one.
        if(parent_page_id_ == INVALID_PAGE_ID){
            Page* new_parent_page = buffer_pool->NewPage();
            BTreeInternalPage<KeyType, page_id_t, KeyComparator>* parent_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(new_parent_page->GetData());
            parent_page->Init(new_parent_page->GetPageId(), INVALID_PAGE_ID, max_size_);
            new_parent_page->SetDirty();
            SetParentPageId(parent_page->GetPageId());
            parent_page->Insert(0, page_id_, buffer_pool, comparator);
        }
        // // 1. create a new sibling page.
        Page* new_page = buffer_pool->NewPage();
        // // 2. copy the half from original to sibling page
        BTreeInternalPage<KeyType, ValueType, KeyComparator>* page = reinterpret_cast<BTreeInternalPage<KeyType, ValueType, KeyComparator>*>(new_page->GetData());
        page->Init(new_page->GetPageId(), parent_page_id_, max_size_);
        new_page->SetDirty();
        copyHalfTo(page);
        updateParentId(page, buffer_pool);

        // 3. Add pointer from parent page to new sibling page
        Page* p_page = buffer_pool->GetPage(parent_page_id_);
        BTreeInternalPage<KeyType, ValueType, KeyComparator>* parent_page = reinterpret_cast<BTreeInternalPage<KeyType, ValueType, KeyComparator>*>(p_page->GetData());
        parent_page->Insert(page->KeyAt(0), page->GetPageId(), buffer_pool, comparator);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::copyHalfTo(BTreeInternalPage<KeyType, ValueType, KeyComparator>* dest){
        size_t mvsize = size_ / 2;  // we only want the second half.
        memmove(dest->keys_, keys_ + mvsize, (size_ - mvsize) * sizeof(std::pair<KeyType, ValueType>));
        dest->SetSize(size_ - mvsize);
        SetSize(mvsize);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::updateParentId(BTreeInternalPage<KeyType, ValueType, KeyComparator>* page, BufferPool* buffer_pool){
        for(size_t i = 0; i < page->GetSize(); ++i){
            Page* tmp = buffer_pool->GetPage(page->ValueAt(i));
            BTreeInternalPage<KeyType, ValueType, KeyComparator>* btree_page = reinterpret_cast<BTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp->GetData());
            btree_page->SetParentPageId(page->GetPageId());
            tmp->SetDirty();
        }
    }

    template class BTreeInternalPage<int, page_id_t, IntComparator>;

} // namespace udb

