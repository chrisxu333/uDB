#include "include/storage/index/b_tree.h"
#include <cassert>

namespace udb
{
    template<typename KeyType, typename ValueType, typename KeyComparator>
    BTree<KeyType, ValueType, KeyComparator>::BTree(BufferPool *buffer_pool, KeyComparator comparator, size_t max_size){
        buffer_pool_ = buffer_pool;
        // Create a new page for btree root.
        Page* page = buffer_pool->NewPage();
        LeafPage* p = reinterpret_cast<LeafPage*>(page->GetData());
        if(max_size != -1) p->Init(page->GetPageId(), INVALID_PAGE_ID, INVALID_PAGE_ID, max_size);
        else p->Init(page->GetPageId());
        page->SetDirty();
        root_page_id_ = p->GetPageId();
        comparator_ = comparator;
    }



    template<typename KeyType, typename ValueType, typename KeyComparator>
    BTreeLeafPage<KeyType, ValueType, KeyComparator>* BTree<KeyType, ValueType, KeyComparator>::findLeafPage(const KeyType& key){
        Page* cur_page = buffer_pool_->GetPage(root_page_id_);
        InternalPage* root = reinterpret_cast<InternalPage*>(cur_page->GetData());
        if(root->isLeafPage()){
            LeafPage* p = reinterpret_cast<LeafPage*>(cur_page->GetData());
            return p;
        }
        // Loop
        while(true){
            page_id_t page_id = root->LookUp(key, comparator_);
            Page* cur_page = buffer_pool_->GetPage(page_id);
            InternalPage* internal_page = reinterpret_cast<InternalPage*>(cur_page->GetData());
            if(internal_page->isLeafPage()){
                LeafPage* p = reinterpret_cast<LeafPage*>(cur_page->GetData());
                return p;
            }else{
                root = internal_page;
            }
        }
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTree<KeyType, ValueType, KeyComparator>::insert(const KeyType& key, const ValueType& value){
        // find which leaf page to insert first.
        LeafPage* target_page = findLeafPage(key);
        // call Insert() on that LeafPage.
        target_page->Insert(key, value, buffer_pool_);
        // Update root page id.
        UpdateRoot();
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTree<KeyType, ValueType, KeyComparator>::UpdateRoot(){
        Page* cur_page = buffer_pool_->GetPage(root_page_id_);
        InternalPage* root = reinterpret_cast<InternalPage*>(cur_page->GetData());
        while(root->GetParentPageId() != INVALID_PAGE_ID){
            cur_page = buffer_pool_->GetPage(root->GetParentPageId());
            root = reinterpret_cast<InternalPage*>(cur_page->GetData());
        }
        root_page_id_ = root->GetPageId();
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTree<KeyType, ValueType, KeyComparator>::remove(){
    }

    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BTree<KeyType, ValueType, KeyComparator>::vis(){
        // print out the tree from root to leaf.
        Page* page = buffer_pool_->GetPage(root_page_id_);
        // First cast to internal page and check page type.
        InternalPage* p = reinterpret_cast<InternalPage*>(page->GetData());
        if(p->isLeafPage()){
            LeafPage* p = reinterpret_cast<LeafPage*>(page->GetData());
            std::cout << "Page "<< p->GetPageId() << ": ";
            // only want to visualize part of the data.
            bool skip = true;
            for(size_t i = 0; i < p->GetSize(); ++i){
                if(i <= 3 || p->GetSize() - i <= 3) std::cout << p->KeyAt(i);
                else if(skip){
                    skip = false;
                    std::cout << " ... ";
                }
            }
            std::cout << std::endl;
            return;
        }
        // The root is not leaf, we need to do a top-down approach.
        std::queue<page_id_t> q;
        q.push(root_page_id_);
        while(!q.empty()){
            std::cout << "level:\t";
            size_t qsize = q.size();
            for(size_t i = 0; i < qsize; ++i){
                Page* page = buffer_pool_->GetPage(q.front());
                q.pop();
                // First cast to internal page and check page type.
                InternalPage* p = reinterpret_cast<InternalPage*>(page->GetData());
                if(p->isLeafPage()){
                    LeafPage* p = reinterpret_cast<LeafPage*>(page->GetData());
                    std::cout << "Page "<< p->GetPageId() << " (" << p->GetParentPageId() << ") (";
                    for(size_t i = 0; i < p->GetSize(); ++i){
                        std::cout << p->KeyAt(i);
                    }
                    std::cout << ") --------";
                }else{
                    std::cout << "Page " << p->GetPageId() << ": ";
                    for(size_t i = 0; i < p->GetSize(); ++i){
                        q.push(p->ValueAt(i));
                        std::cout << "(" << p->KeyAt(i) << " " << p->ValueAt(i) << ")";
                    }
                    std::cout << "----------";
                }
            }
            std::cout << std::endl;
        }
    }

    template class BTree<int, RID, IntComparator>;
} // namespace udb
