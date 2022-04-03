#pragma once
#include "bplus_tree_page.h"

namespace udb
{
    class BPTreeInternalPage {
        public:
            BPTreeInternalPage() : lsib_(nullptr), rsib_(nullptr){
                type_ = NodeType::BPLUS_TREE_NON_LEAF;
                parent_key_idx_ = -1;
            }
            ~BPTreeInternalPage() = default;

            // Getter and Setter
            BPTreeInternalPage* GetParent(){ return parent_; }
            
            BPTreeInternalPage* GetLeftSib(){ return lsib_; }
            BPTreeInternalPage* GetRightSib(){ return rsib_; }

            int KeyAt(int index){ return key_[index]; }
            BPlusTreePage* ValueAt(int index){ return sub_ptr_[index]; }

            NodeType GetType(){ return type_; }

            int GetChildren(){ return children_; }

            int GetParentIndex(){ return parent_key_idx_; }

            int* GetKeys(){ return key_; }

            void SetChildren(int children){ children_ = children; }

            void SetParent(BPTreeInternalPage* parent){ parent_ = parent; }

            void SetParentIndex(int idx){ parent_key_idx_ = idx; }

            void SetLeftSib(BPTreeInternalPage* lsib){ lsib_ = lsib; }

            void SetRightSib(BPTreeInternalPage* rsib){ rsib_ = rsib; }

            void SetType(NodeType type){ type_ = type; }

            void SetKeyAt(int index, int value){ key_[index] = value; }

            void SetPtrAt(int index, BPlusTreePage * value){ sub_ptr_[index] = value; }

            // private APIs ===========================================

            int parent_node_build(BPlusTree* tree, BPlusTreePage *left, BPlusTreePage *right, int key, int level);

            int non_leaf_insert(BPlusTree* tree, BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int level);

            void non_leaf_simple_insert(BPlusTreePage *l_ch, BPlusTreePage *r_ch, int key, int insert);

            int key_binary_search(int *arr, int len, int target);

            int non_leaf_split_left( BPTreeInternalPage *left, BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int insert);

            int non_leaf_split_right1(BPTreeInternalPage *right, BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int insert);

            int non_leaf_split_right2(BPTreeInternalPage *right,
                                    BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int insert);

            void non_leaf_delete(BPTreeInternalPage *node);
            
            SiblingType non_leaf_sibling_select(BPTreeInternalPage *l_sib, BPTreeInternalPage *r_sib,
                                            BPTreeInternalPage *parent, int i);

            void non_leaf_shift_from_left(BPTreeInternalPage *left,
                                                int parent_key_index, int remove);

            void non_leaf_merge_into_left(BPTreeInternalPage *left,
                                                int parent_key_index, int remove);

            void non_leaf_shift_from_right(BPTreeInternalPage *right,
                                                int parent_key_index);

            void non_leaf_merge_from_right(BPTreeInternalPage *right,
                                                int parent_key_index);

            void non_leaf_simple_remove(int remove);

            void non_leaf_remove(BPlusTree* tree, int remove);

        private:
            NodeType type_;
            int parent_key_idx_;
            BPTreeInternalPage *parent_;
            BPTreeInternalPage* lsib_;
            BPTreeInternalPage* rsib_;
            // list_head link;
            int children_;
            int key_[MAX_ORDER - 1];
            BPlusTreePage *sub_ptr_[MAX_ORDER];
    };
} // namespace udb 