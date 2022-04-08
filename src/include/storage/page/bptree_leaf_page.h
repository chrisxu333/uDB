#pragma once

#include "include/storage/page/bptree_internal_page.h"

namespace udb
{
    #define LEAF_PAGE_HEADER_SIZE 32
    #define LEAF_PAGE_DATA_SIZE ((4096 - LEAF_PAGE_HEADER_SIZE) / (sizeof(std::pair<int, int>)))

    template <typename KeyType, typename ValueType, typename KeyComparator>
    class BPTreeLeafPage : public BPTreePage{
        public:
            BPTreeLeafPage(): BPTreePage(NodeType::BPLUS_TREE_LEAF, -1, -1){
                lsib_page_id_ = -1;
                rsib_page_id_ = -1;
            }
            ~BPTreeLeafPage() = default;

            void Init(int page_id){
                lsib_page_id_ = -1;
                rsib_page_id_ = -1;
                SetType(NodeType::BPLUS_TREE_LEAF);
                SetParent(-1);
                SetParentIndex(-1);
                SetPageId(page_id);
            }

            // Getter
            
            int GetLeftSib(){ return lsib_page_id_; }

            int GetRightSib(){ return rsib_page_id_; }

            KeyType KeyAt(int index){ return keys_[index].first; }

            ValueType ValueAt(int index){ return keys_[index].second; }
            
            int GetEntry(){ return entries_; }

            // Setter
            void SetEntry(int entries){
                entries_ = entries;
            }

            void SetLeftSib(int lsib_page_id){
                lsib_page_id_ = lsib_page_id;
            }

            void SetRightSib(int rsib_page_id){
                rsib_page_id_ = rsib_page_id;
            }

            void SetKeyAt(int index, KeyType value){
                keys_[index].first = value;
            }

            void SetDataAt(int index, ValueType value){
                keys_[index].second = value;
            }

            // private APIs ==========================================================================
            int key_binary_search(int len, int target);

            int parent_node_build(BPTree<KeyType, ValueType, KeyComparator>* tree, int page_id_left, int page_id_right, KeyType key, int level, BufferPool* buffer_pool);

            // public APIs =========================================================================
            int leaf_insert(BPTree<KeyType, ValueType, KeyComparator>* tree, KeyType key, ValueType data, BufferPool* buffer_pool);

            void leaf_split_left(BPTreeLeafPage *left, KeyType key, ValueType data, int insert);

            void leaf_split_right(BPTreeLeafPage *right, KeyType key, ValueType data, int insert);

            void leaf_simple_insert(KeyType key, ValueType data, int insert);

            int leaf_remove(BPTree<KeyType, ValueType, KeyComparator>* tree, KeyType key, BufferPool* buffer_pool);

            void leaf_delete(BPTreeLeafPage *node, BufferPool* buffer_pool);

            SiblingType leaf_sibling_select( BPTreeLeafPage<KeyType, ValueType, KeyComparator> *l_sib,  BPTreeLeafPage<KeyType, ValueType, KeyComparator> *r_sib, BPTreeInternalPage<KeyType, ValueType, KeyComparator> *parent, int i);

            void leaf_shift_from_left(BPTreeLeafPage *left, int parent_key_index, int remove, BufferPool* buffer_pool);

            void leaf_merge_into_left(BPTreeLeafPage *left, int remove, BufferPool* buffer_pool);

            void leaf_shift_from_right(BPTreeLeafPage *right, int parent_key_index, BufferPool* buffer_pool);

            void leaf_merge_from_right(BPTreeLeafPage *right, BufferPool* buffer_pool);

            void leaf_simple_remove(int remove);

        private:
            int lsib_page_id_;
            int rsib_page_id_;
            int entries_;   // 4 bytes int
            std::pair<KeyType, ValueType> keys_[LEAF_PAGE_DATA_SIZE];
    };
} // namespace udb