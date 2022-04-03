#pragma once

#include "bplus_tree_page.h"

namespace udb
{
    class BPTreeLeafPage {
        public:
            BPTreeLeafPage():lsib_(nullptr), rsib_(nullptr){
                type_ = NodeType::BPLUS_TREE_LEAF;
                parent_key_idx_ = -1;
            }
            ~BPTreeLeafPage() = default;

            // Getter

            BPTreeInternalPage* GetParent(){ return parent_; }
            
            BPTreeLeafPage* GetLeftSib(){ return lsib_; }
            BPTreeLeafPage* GetRightSib(){ return rsib_; }

            int KeyAt(int index){ return key_[index]; }
            int ValueAt(int index){ return data_[index]; }

            NodeType GetType(){ return type_; }

            int GetEntry(){ return entries_; }

            int GetParentIndex(){ return parent_key_idx_; }

            int* GetKeys(){ return key_; }
            // Setter

            void SetEntry(int entries){ entries_ = entries; }

            void SetParent(BPTreeInternalPage* parent){ parent_ = parent; }

            void SetParentIndex(int idx){ parent_key_idx_ = idx; }

            void SetLeftSib(BPTreeLeafPage* lsib){ lsib_ = lsib; }

            void SetRightSib(BPTreeLeafPage* rsib){ rsib_ = rsib; }

            void SetType(NodeType type){ type_ = type; }

            void SetKeyAt(int index, int value){ key_[index] = value; }

            void SetDataAt(int index, int value){ data_[index] = value; }

            // private APIs ==========================================================================
            int key_binary_search(int *arr, int len, int target);

            int parent_node_build(BPlusTree* tree, BPlusTreePage *left, BPlusTreePage *right, int key, int level);

            // public APIs =========================================================================
            int leaf_insert(BPlusTree* tree, int key, int data);

            void leaf_split_left(BPTreeLeafPage *left, int key, int data, int insert);

            void leaf_split_right(BPTreeLeafPage *right, int key, int data, int insert);

            void leaf_simple_insert(int key, int data, int insert);

            int leaf_remove(BPlusTree* tree, int key);

            void leaf_delete(BPTreeLeafPage *node);

            SiblingType leaf_sibling_select( BPTreeLeafPage *l_sib,  BPTreeLeafPage *r_sib,
                                        BPTreeInternalPage *parent, int i);

            void leaf_shift_from_left(BPTreeLeafPage *left,
                                            int parent_key_index, int remove);

            void leaf_merge_into_left(BPTreeLeafPage *left, int remove);

            void leaf_shift_from_right(BPTreeLeafPage *right, int parent_key_index);

            void leaf_merge_from_right(BPTreeLeafPage *right);

            void leaf_simple_remove(int remove);


        private:
            NodeType type_;
            int parent_key_idx_;
            BPTreeInternalPage *parent_;
            BPTreeLeafPage* lsib_;
            BPTreeLeafPage* rsib_;
            int entries_;
            int key_[MAX_ORDER];
            int data_[MAX_ORDER];
    };
} // namespace udb


