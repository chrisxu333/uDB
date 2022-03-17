#include "../../include/storage/page/b_tree_internal_page.h"

namespace udb
{
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = INTERNAL_PAGE_SIZE){
        page_id_ = page_id;
        parent_id_ = INVALID_PAGE_ID;
        max_size_ = max_size;
        page_type_ = IndexPageType::INTERNAL_PAGE;
        size_ = 0;
        keys_.resize(max_size);
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
        size_t i = 0;
        while(i < size_ && comparator(keys_[i].first, key) > 0){
            i++;
        }
        if(i != size_) return keys_[i].second;
        return INVALID_PAGE_ID;
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Insert(const KeyType& key, const ValueType& value){
        if(size_ == max_size_){
            // split
        }else{
            // simple, just add a new pair in the end.
            std::pair<KeyType, ValueType> tmp = {key, value};
            keys_[size_] = tmp;
            size_++;
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Split(){
        // Split process consists of the following:
        // 1. create a new sibling page.
        // 2. copy the half from original to sibling page
        // 3. Add pointer from parent page to new sibling page
        // 4. If parent page overflows, recursively call split on that page until root.
    }

} // namespace udb

