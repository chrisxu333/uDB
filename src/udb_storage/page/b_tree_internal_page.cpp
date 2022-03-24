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
        std::sort(keys_ + 1, keys_ + size_, 
            [comparator](const std::pair<KeyType, ValueType>& lhs, const std::pair<KeyType, ValueType>& rhs){
                return comparator(lhs.first, rhs.first) < 0; 
            });
        if(size_ == max_size_) {
            LOG_DEBUG("Internal split");
            Split(buffer_pool, comparator);
        }
    }


    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Remove(const KeyType& key, BufferPool* buffer_pool, KeyComparator comparator){
        // 1. find the location of index, linear search
        size_t idx = lookUpIndex(key, comparator);
        //if(idx == 0) return;
        // record removed key
        KeyType okey = this->KeyAt(0);
        // 2. remove and rearrange keys_
        for(size_t i = idx; i < size_ - 1; ++i){
            keys_[i] = keys_[i + 1];
        }
        size_--;
        // 3. if idx is 0, then also update it in the parent node.
        Page* parent = nullptr;
        page_id_t parent_id = parent_page_id_;

        while(parent_id != INVALID_PAGE_ID && idx == 0){
            std::cout << parent_id << std::endl;
            parent = buffer_pool->GetPage(parent_id);
            BTreeInternalPage<KeyType, page_id_t, KeyComparator>* parent_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(parent->GetData());
            idx = parent_page->lookUpIndex(okey, comparator);
            // update key in the parent node.
            parent_page->SetKeyAt(idx, KeyAt(0));
            parent_id = parent_page->GetParentPageId();
        }
        // 4. if size_ still larger than [M/2], return
        // 5. otherwise will have to merge it with its siblings.
        if(size_ < (max_size_ / 2) && size_ != 1 && parent_page_id_ != INVALID_PAGE_ID){
            LOG_DEBUG("merge internal");
            Merge(okey, buffer_pool, comparator);
            // 6. now update the parent node.
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
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::Merge(KeyType okey, BufferPool* buffer_pool, KeyComparator comparator){
        // Find parent
        Page* parent_page = buffer_pool->GetPage(this->parent_page_id_);
        BTreeInternalPage<KeyType, page_id_t, KeyComparator>* p_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(parent_page->GetData());
        size_t curpage_idx = p_page->lookUpIndex(this->KeyAt(0), comparator);
        // Try borrow from siblings first.
        // Try to borrow from left sibling.
        // Get left sibling.
        BTreeInternalPage<KeyType, page_id_t, KeyComparator>* lsib_page = nullptr, *rsib_page = nullptr;
        if(curpage_idx != 0){
            Page* lsibling_page = buffer_pool->GetPage(p_page->ValueAt(curpage_idx - 1));
            lsib_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(lsibling_page->GetData());
        }
        if(lsib_page && ((lsib_page->GetSize() - 1) > (this->max_size_ / 2))){
            LOG_DEBUG("borrow left");
            this->Insert(lsib_page->KeyAt(lsib_page->GetSize() - 1), lsib_page->ValueAt(lsib_page->GetSize() - 1), buffer_pool, comparator);
            lsib_page->Remove(lsib_page->KeyAt(lsib_page->GetSize() - 1), buffer_pool, comparator);
            // update parent.
            p_page->SetKeyAt(curpage_idx, this->KeyAt(0));

            Page* this_page = buffer_pool->GetPage(page_id_);
            BTreeInternalPage<KeyType, page_id_t, KeyComparator>* t_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(this_page->GetData());
            updateParentId(t_page, buffer_pool);            
            return;
        }
        //  b. try to borrow from right sibling.
        if(curpage_idx != p_page->GetSize() - 1){
            Page* rsibling_page = buffer_pool->GetPage(p_page->ValueAt(curpage_idx + 1));
            rsib_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(rsibling_page->GetData());
        }
        if(rsib_page != nullptr && ((rsib_page->GetSize() - 1) > (this->max_size_ / 2))){
            LOG_DEBUG("borrow right");
            this->Insert(rsib_page->KeyAt(0), rsib_page->ValueAt(0), buffer_pool, comparator);
            rsib_page->Remove(rsib_page->KeyAt(0), buffer_pool, comparator);
            // Since Remove() API would handle parent update, don't have to do anything here.
            Page* this_page = buffer_pool->GetPage(page_id_);
            BTreeInternalPage<KeyType, page_id_t, KeyComparator>* t_page = reinterpret_cast<BTreeInternalPage<KeyType, page_id_t, KeyComparator>*>(this_page->GetData());
            updateParentId(t_page, buffer_pool);
            return;
        }
        // 2. If borrow won't work, then we have to merge and redistribute.
        if(lsib_page != nullptr){
            LOG_DEBUG("Merge left");
            for(size_t i = 0; i < size_; ++i){
                std::cout << KeyAt(i) << ValueAt(i) << std::endl;
            }
            copyAllTo(lsib_page, MergeMode::APPEND);
            size_t c_idx = p_page->lookUpIndex(this->KeyAt(0), comparator);
            p_page->Remove(p_page->KeyAt(c_idx), buffer_pool, comparator);
            updateParentId(lsib_page, buffer_pool);
            return;
        }
        if(rsib_page != nullptr){
            LOG_DEBUG("Merge right");
            copyAllTo(rsib_page, MergeMode::INSERT);
            size_t c_idx = p_page->lookUpIndex(this->KeyAt(0), comparator);
            p_page->Remove(p_page->KeyAt(c_idx), buffer_pool, comparator);
            updateParentId(rsib_page, buffer_pool);
            return;
        }
    }



    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::copyHalfTo(BTreeInternalPage<KeyType, ValueType, KeyComparator>* dest){
        size_t mvsize = size_ / 2;  // we only want the second half.
        memmove(dest->keys_, keys_ + mvsize, (size_ - mvsize) * sizeof(std::pair<KeyType, ValueType>));
        dest->SetSize(size_ - mvsize);
        SetSize(mvsize);
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTreeInternalPage<KeyType, ValueType, KeyComparator>::copyAllTo(BTreeInternalPage<KeyType, ValueType, KeyComparator>* dest, MergeMode mode){
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
    size_t BTreeInternalPage<KeyType, ValueType, KeyComparator>::lookUpIndex(const KeyType& key, KeyComparator comparator) const{
        for(size_t i = 1; i < size_; ++i){
            if(comparator(KeyAt(i), key) == 0){
                // find the first key that matches the result. 
                // TODO: Support non-unique key in the future.
                return i;
            }
        }
        // Ideally, this return should never be called.
        return 0;
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

