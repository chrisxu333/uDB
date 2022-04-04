#pragma once
#include "bptree_page.h"

namespace udb
{
    #define INTERNAL_PAGE_HEADER_SIZE 32
    #define INTERNAL_PAGE_DATA_SIZE ((4096 - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(std::pair<int, int>)))

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

            int KeyAt(int index){ return keys_[index].first; }
            BPTreePage* ValueAt(int index){ return keys_[index].second; }

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


            void SetKeyAt(int index, int value){
                keys_[index].first = value;
            }

            void SetPtrAt(int index, BPTreePage * value){
                keys_[index].second = value;
            }

            // private APIs ===========================================

            int parent_node_build(BPTree* tree, int page_id_left, int page_id_right, int key, int level, BufferPool* buffer_pool);

            int non_leaf_insert(BPTree* tree, int page_id_l_ch,  int page_id_r_ch, int key, int level, BufferPool* buffer_pool);

            void non_leaf_simple_insert(BPTreePage *l_ch, BPTreePage *r_ch, int key, int insert);

            int key_binary_search(int len, int target);

            int non_leaf_split_left( BPTreeInternalPage *left, BPTreePage *l_ch,  BPTreePage *r_ch, int key, int insert, BufferPool* buffer_pool);

            int non_leaf_split_right1(BPTreeInternalPage *right, BPTreePage *l_ch,  BPTreePage *r_ch, int key, int insert, BufferPool* buffer_pool);

            int non_leaf_split_right2(BPTreeInternalPage *right, BPTreePage *l_ch,  BPTreePage *r_ch, int key, int insert, BufferPool* buffer_pool);

            void non_leaf_delete(BPTreeInternalPage *node, BufferPool* buffer_pool);
            
            SiblingType non_leaf_sibling_select(BPTreeInternalPage *l_sib, BPTreeInternalPage *r_sib, BPTreeInternalPage *parent, int i);

            void non_leaf_shift_from_left(BPTreeInternalPage *left, int parent_key_index, int remove, BufferPool* buffer_pool);

            void non_leaf_merge_into_left(BPTreeInternalPage *left, int parent_key_index, int remove, BufferPool* buffer_pool);

            void non_leaf_shift_from_right(BPTreeInternalPage *right, int parent_key_index, BufferPool* buffer_pool);

            void non_leaf_merge_from_right(BPTreeInternalPage *right, int parent_key_index, BufferPool* buffer_pool);

            void non_leaf_simple_remove(int remove);

            void non_leaf_remove(BPTree* tree, int remove, BufferPool* buffer_pool);

        private:
            int lsib_page_id_;
            int rsib_page_id_;
            int children_;
            std::pair<int, BPTreePage*> keys_[INTERNAL_PAGE_DATA_SIZE];
    };
} // namespace udb