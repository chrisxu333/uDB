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
        size_ = 0;
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
        for(size_t i = 0; i < size_; ++i){
            if(comparator(KeyAt(i), key) == 0){
                // find the first key that matches the result. 
                // TODO: Support non-unique key in the future.
                return ValueAt(i);
            }
        }
        // Ideally, this return should never be called.
        return ValueAt(size_ - 1);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::Insert(const KeyType& key, const ValueType& value, BufferPool* buffer_pool, KeyComparator comparator){
        // simple, just add a new pair in the end.
        std::pair<KeyType, ValueType> tmp = {key, value};
        keys_[size_] = tmp;
        size_++;
        std::sort(keys_, keys_ + size_, 
            [comparator](const std::pair<KeyType, ValueType>& lhs, const std::pair<KeyType, ValueType>& rhs){
                return comparator(lhs.first, rhs.first) < 0; 
            });
        if(size_ == max_size_) {
            LOG_DEBUG("Leaf split.");
            Split(buffer_pool, comparator);
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::Remove(const KeyType& key, BufferPool* buffer_pool, KeyComparator comparator){
        // 1. find the location of index, linear search
        size_t idx = lookUpIndex(key, comparator);
        if(idx == -1) return;
        // record removed key
        KeyType okey = this->KeyAt(0);
        // 2. remove and rearrange keys_
        for(size_t i = idx; i < size_ - 1; ++i){
            keys_[i] = keys_[i + 1];
        }
        this->size_--;
        // 3. if idx is 0, then also update it in the parent node.
        Page* parent;
        page_id_t parent_id = parent_page_id_;

        std::cout << page_id_ << parent_id << idx << std::endl;
        while(parent_id != INVALID_PAGE_ID && idx == 0){
            parent = buffer_pool->GetPage(parent_id);
            BTreeInternalPage<KeyType, page_id_t, KeyComparator>* parent_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(parent->GetData());
            idx = parent_page->lookUpIndex(okey, comparator);
            // update key in the parent node.
            parent_page->SetKeyAt(idx, KeyAt(0));
            parent_id = parent_page->GetParentPageId();
        }
        // 4. if size_ still larger than [M/2], return
        // 5. otherwise will have to merge it with its siblings.
        if(size_ < (max_size_ / 2)){
            LOG_DEBUG("merge ops");
            Merge(buffer_pool, comparator);
            // 6. now update the parent node.
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::Split(BufferPool* buffer_pool, KeyComparator comparator){
        // 0. if there's no parent for this page right now, create one.
        if(parent_page_id_ == INVALID_PAGE_ID){
            Page* new_parent_page = buffer_pool->NewPage();
            BTreeInternalPage<KeyType, page_id_t, KeyComparator>* parent_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(new_parent_page->GetData());
            parent_page->Init(new_parent_page->GetPageId(), INVALID_PAGE_ID, max_size_);
            new_parent_page->SetDirty();
            SetParentPageId(parent_page->GetPageId());
            parent_page->Insert(0, page_id_, buffer_pool, comparator);
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
        parent_page->Insert(new_leaf_page->KeyAt(0), new_leaf_page->GetPageId(), buffer_pool, comparator);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::Merge(BufferPool* buffer_pool, KeyComparator comparator){
        // Find parent
        Page* parent_page = buffer_pool->GetPage(this->parent_page_id_);
        BTreeInternalPage<KeyType, page_id_t, KeyComparator>* p_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(parent_page->GetData());
        size_t curpage_idx = p_page->lookUpIndex(this->KeyAt(0), comparator);
        // Try borrow from siblings first.
        // Try to borrow from left sibling.
        // Get left sibling.
        BTreeLeafPage<KeyType, ValueType, KeyComparator>* lsib_page = nullptr, *rsib_page = nullptr;
        if(curpage_idx != 0){
            Page* lsibling_page = buffer_pool->GetPage(p_page->ValueAt(curpage_idx - 1));
            lsib_page = reinterpret_cast<BTreeLeafPage<KeyType, ValueType, KeyComparator>*>(lsibling_page->GetData());
        }
        if(lsib_page != nullptr && ((lsib_page->GetSize() - 1) > (this->max_size_ / 2))){
            LOG_DEBUG("borrow from left");
            this->Insert(lsib_page->KeyAt(lsib_page->GetSize() - 1), lsib_page->ValueAt(lsib_page->GetSize() - 1), buffer_pool, comparator);
            lsib_page->Remove(lsib_page->KeyAt(lsib_page->GetSize() - 1), buffer_pool, comparator);
            // update parent.
            p_page->SetKeyAt(curpage_idx, this->KeyAt(0));
            return;
        }
        //  b. try to borrow from right sibling.
        if(curpage_idx != p_page->GetSize() - 1){
            LOG_DEBUG("prepare borrow right");
            Page* rsibling_page = buffer_pool->GetPage(p_page->ValueAt(curpage_idx + 1));
            rsib_page = reinterpret_cast<BTreeLeafPage<KeyType, ValueType, KeyComparator>*>(rsibling_page->GetData());
        }
        if(rsib_page != nullptr && ((rsib_page->GetSize() - 1) > (this->max_size_ / 2))){
            LOG_DEBUG("borrow from right");
            this->Insert(rsib_page->KeyAt(0), rsib_page->ValueAt(0), buffer_pool, comparator);
            rsib_page->Remove(rsib_page->KeyAt(0), buffer_pool, comparator);
            // Since Remove() API would handle parent update, don't have to do anything here.
            return;
        }
        // 2. If borrow won't work, then we have to merge and redistribute.
        LOG_DEBUG("Merge");
        size_t c_idx = p_page->lookUpIndex(this->KeyAt(0), comparator);
        // Try to merge with left sibling first.
        if(lsib_page != nullptr){
            copyAllTo(lsib_page, MergeMode::APPEND);
            // SET parent key one level up so that it could propagate through the chain
            p_page->SetKeyAt(c_idx - 1, lsib_page->KeyAt(0));
            p_page->Remove(p_page->KeyAt(c_idx), buffer_pool, comparator);
            return;
        }
        if(rsib_page != nullptr){
            copyAllTo(rsib_page, MergeMode::INSERT);
            // SET parent key one level up so that it could propagate through the chain
            p_page->SetKeyAt(c_idx + 1, rsib_page->KeyAt(0));
            p_page->Remove(p_page->KeyAt(c_idx), buffer_pool, comparator);
            return;
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::copyHalfTo(BTreeLeafPage<KeyType, ValueType, KeyComparator>* dest){
        size_t mvsize = size_ / 2;  // we only want the second half.
        memmove(dest->keys_, keys_ + mvsize, (size_ - mvsize) * sizeof(std::pair<KeyType, ValueType>));
        dest->SetSize(size_ - mvsize);
        SetSize(mvsize);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeLeafPage<KeyType, ValueType, KeyComparator>::copyAllTo(BTreeLeafPage<KeyType, ValueType, KeyComparator>* dest, MergeMode mode){
        if(mode == MergeMode::APPEND){
            // create space for incoming data.
            memcpy(dest->keys_ + dest->GetSize(), keys_, size_ * sizeof(std::pair<KeyType, ValueType>));
            dest->SetSize(dest->GetSize() + size_);
        }else{
            // create space for incoming data.
            memmove(dest->keys_ + size_, dest->keys_, dest->GetSize() * sizeof(std::pair<KeyType, ValueType>));
            memcpy(dest->keys_, keys_, size_ * sizeof(std::pair<KeyType, ValueType>));
            dest->SetSize(dest->GetSize() + size_);
        }
    }


    template<typename KeyType, typename ValueType, typename KeyComparator>
    size_t BTreeLeafPage<KeyType, ValueType, KeyComparator>::lookUpIndex(const KeyType& key, KeyComparator comparator) const{
        for(size_t i = 0; i < size_; ++i){
            if(comparator(KeyAt(i), key) == 0){
                // find the first key that matches the result. 
                // TODO: Support non-unique key in the future.
                return i;
            }
        }
        // Ideally, this return should never be called.
        return -1;
    }

    template class BTreeLeafPage<int, RID, IntComparator>;

} // namespace udb
