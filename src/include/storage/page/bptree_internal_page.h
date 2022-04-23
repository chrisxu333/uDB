#pragma once
#include "bptree_page.h"

namespace udb
{
    #define INTERNAL_PAGE_HEADER_SIZE 32
    #define INTERNAL_PAGE_DATA_SIZE ((4096 - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(std::pair<int, int>)))

    template<typename KeyType, typename ValueType, typename KeyComparator>
    class BPTreeInternalPage : public BPTreePage {
        public:
            BPTreeInternalPage(): BPTreePage(NodeType::BPLUS_TREE_NON_LEAF, -1, -1){
                lsib_page_id_ = -1;
                rsib_page_id_ = -1;
            }
            ~BPTreeInternalPage() = default;

            void Init(int page_id){
                lsib_page_id_ = -1;
                rsib_page_id_ = -1;
                SetType(NodeType::BPLUS_TREE_NON_LEAF);
                SetParent(-1);
                SetParentIndex(-1);
                SetPageId(page_id);
            }

            // Getter and Setter        
            int GetLeftSib(){ return lsib_page_id_; }
            int GetRightSib(){ return rsib_page_id_; }

            KeyType KeyAt(int index){ return keys_[index].first; }
            page_id_t ValueAt(int index){ return keys_[index].second; }

            int GetChildren(){ return children_; }

            void SetChildren(int children){
                children_ = children;
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

            void SetPtrAt(int index, page_id_t value){
                keys_[index].second = value;
            }

            // private APIs ===========================================

            int parent_node_build(BPTree<KeyType, ValueType, KeyComparator>* tree, int page_id_left, int page_id_right, KeyType key, int level, BufferPool* buffer_pool);

            int non_leaf_insert(BPTree<KeyType, ValueType, KeyComparator>* tree, int page_id_l_ch,  int page_id_r_ch, KeyType key, int level, BufferPool* buffer_pool);

            void non_leaf_simple_insert(BPTreePage *l_ch, BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool);

            int key_binary_search(int len, int target);

            int non_leaf_split_left( BPTreeInternalPage<KeyType, ValueType, KeyComparator> *left, BPTreePage *l_ch,  BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool);

            int non_leaf_split_right1(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, BPTreePage *l_ch,  BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool);

            int non_leaf_split_right2(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, BPTreePage *l_ch,  BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool);

            void non_leaf_delete(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *node, BufferPool* buffer_pool);
            
            SiblingType non_leaf_sibling_select(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *l_sib, BPTreeInternalPage<KeyType, ValueType, KeyComparator> *r_sib, BPTreeInternalPage<KeyType, ValueType, KeyComparator> *parent, int i);

            void non_leaf_shift_from_left(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *left, int parent_key_index, int remove, BufferPool* buffer_pool);

            void non_leaf_merge_into_left(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *left, int parent_key_index, int remove, BufferPool* buffer_pool);

            void non_leaf_shift_from_right(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, int parent_key_index, BufferPool* buffer_pool);

            void non_leaf_merge_from_right(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, int parent_key_index, BufferPool* buffer_pool);

            void non_leaf_simple_remove(int remove, BufferPool* buffer_pool);

            void non_leaf_remove(BPTree<KeyType, ValueType, KeyComparator>* tree, int remove, BufferPool* buffer_pool);

        private:
            int lsib_page_id_;
            int rsib_page_id_;
            int children_;
            std::pair<int, page_id_t> keys_[INTERNAL_PAGE_DATA_SIZE];
    };
} // namespace udb