#include "include/storage/page/b_tree_leaf_page.h"

namespace udb
{
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::Init(page_id_t page_id, page_id_t next_page_id, page_id_t parent_id, int max_size){
        page_id_ = page_id;
        next_page_id_ = next_page_id;
        parent_page_id_ = parent_id;
        max_size_ = max_size;
        page_type_ = IndexPageType::LEAF_PAGE;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    KeyType BTreeLeafPage<KeyType, ValueType, KeyComparator>::KeyAt(int index) const{
        return keys_[index].first;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    ValueType BTreeLeafPage<KeyType, ValueType, KeyComparator>::ValueAt(int index) const{
        return keys_[index].second;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::SetKeyAt(int index, const KeyType& nkey){
        keys_[index].first = nkey;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    ValueType BTreeLeafPage<KeyType, ValueType, KeyComparator>::LookUp(const KeyType& key, const KeyComparator& comparator) const{
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
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::Insert(const KeyType& key, const ValueType& value, BufferPool* buffer_pool){
        // simple, just add a new pair in the end.
        std::pair<KeyType, ValueType> tmp = {key, value};
        keys_[size_] = tmp;
        size_++;
        if(size_ == max_size_ - 1) {
            LOG_DEBUG("Leaf split.");
            Split(buffer_pool);
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::Split(BufferPool* buffer_pool){
        // 0. if there's no parent for this page right now, create one.
        if(parent_page_id_ == INVALID_PAGE_ID){
            Page* new_parent_page = buffer_pool->NewPage();
            BTreeInternalPage<KeyType, page_id_t, KeyComparator>* parent_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(new_parent_page->GetData());
            parent_page->Init(new_parent_page->GetPageId(), INVALID_PAGE_ID, max_size_);
            new_parent_page->SetDirty();
            SetParentPageId(parent_page->GetPageId());
            parent_page->Insert(0, page_id_, buffer_pool);
        }
        // 1. create a new sibling page.
        Page* new_page = buffer_pool->NewPage();
        // 2. copy the half from original to sibling page
        BTreeLeafPage<KeyType, ValueType, KeyComparator>* new_leaf_page = reinterpret_cast<BTreeLeafPage<KeyType, ValueType, KeyComparator>*>(new_page->GetData());
        new_leaf_page->Init(new_page->GetPageId(), INVALID_PAGE_ID, parent_page_id_, max_size_);
        new_page->SetDirty();
        copyHalfTo(new_leaf_page);

        // set next page id of current page to this new sibling page.
        next_page_id_ = new_leaf_page->GetPageId();
        
        // Get parent page and insert the old [M/2] in there.
        Page* p_page = buffer_pool->GetPage(parent_page_id_);
        BTreeInternalPage<KeyType, page_id_t, KeyComparator>* parent_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(p_page->GetData());
        parent_page->Insert(new_leaf_page->KeyAt(0), new_leaf_page->GetPageId(), buffer_pool);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::copyHalfTo(BTreeLeafPage<KeyType, ValueType, KeyComparator>* dest){
        size_t mvsize = size_ / 2;  // we only want the second half.
        memmove(dest->keys_, keys_ + mvsize, (size_ - mvsize) * sizeof(std::pair<KeyType, ValueType>));
        dest->SetSize(size_ - mvsize);
        SetSize(mvsize);
    }

    template class BTreeLeafPage<int, RID, IntComparator>;

} // namespace udb
