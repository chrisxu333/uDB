#include "include/storage/page/bplus_tree_leaf_page.h"
#include "include/storage/page/bplus_tree_internal_page.h"
#include "include/storage/index/bplus_tree.h"

namespace udb
{
	int BPTreeLeafPage::key_binary_search(int *arr, int len, int target){
		int low = -1;
		int high = len;
		while (low + 1 < high) {
			int mid = low + (high - low) / 2;
			if (target > arr[mid]) {
				low = mid;
			} else {
				high = mid;
			}
		}
		if (high >= len || arr[high] != target) {
			return -high - 1;
		} else {
			return high;
		}
	}

	int BPTreeLeafPage::parent_node_build(BPlusTree* tree, BPlusTreePage *left, BPlusTreePage *right, int key, int level){
		if (left->GetParent() == nullptr && right->GetParent() == nullptr) {
			/* new parent */
			BPTreeInternalPage *parent = new BPTreeInternalPage();
			parent->SetKeyAt(0, key);
			parent->SetPtrAt(0, left);
			parent->ValueAt(0)->SetParent(parent);
			parent->ValueAt(0)->SetParentIndex(-1);
			parent->SetPtrAt(1, right);
			parent->ValueAt(1)->SetParent(parent);
			parent->ValueAt(1)->SetParentIndex(0);
			parent->SetChildren(2);
			/* update root */
			tree->SetRoot(reinterpret_cast<BPlusTreePage *>(parent));
			return 0;
		} else if (right->GetParent() == nullptr) {
				/* trace upwards */
				right->SetParent(left->GetParent());
				return left->GetParent()->non_leaf_insert(tree, left, right, key, level + 1);
		} else {
				/* trace upwards */
				left->SetParent(right->GetParent());
				return right->GetParent()->non_leaf_insert(tree, left, right, key, level + 1);
		}
	}

	// public APIs =========================================================================
	int BPTreeLeafPage::leaf_insert(BPlusTree* tree, int key, int data){
		/* search key location */
		int insert = key_binary_search(GetKeys(), GetEntry(), key);
		if (insert >= 0) {
				/* Already exists */
				return -1;
		}
		insert = -insert - 1;

		/* node full */
		if (GetEntry() == tree->GetEntry()) {
				/* split = [m/2] */
				int split = (tree->GetEntry() + 1) / 2;
				/* splited sibling node */
				BPTreeLeafPage *sibling = new BPTreeLeafPage();
				/* sibling leaf replication due to location of insertion */
				if (insert < split) {
						sibling->SetRightSib(this);
						sibling->SetLeftSib(GetLeftSib());
						if(GetLeftSib()) GetLeftSib()->SetRightSib(sibling);
						SetLeftSib(sibling);
						leaf_split_left(sibling, key, data, insert);
				} else {
						sibling->SetLeftSib(this);
						sibling->SetRightSib(GetRightSib());
						if(GetRightSib()) GetRightSib()->SetLeftSib(sibling);
						SetRightSib(sibling);
						leaf_split_right(sibling, key, data, insert);
				}
				/* build new parent */
				if (insert < split) {
						return parent_node_build(tree, reinterpret_cast<BPlusTreePage *>(sibling),
								reinterpret_cast<BPlusTreePage *>(this), KeyAt(0), 0);
				} else {
						return parent_node_build(tree, reinterpret_cast<BPlusTreePage *>(this),
								reinterpret_cast<BPlusTreePage *>(sibling), sibling->KeyAt(0), 0);
				}
		} else {
				leaf_simple_insert(key, data, insert);
		}

		return 0;
	}

	void BPTreeLeafPage::leaf_split_left(BPTreeLeafPage *left, int key, int data, int insert){
		int i, j;
		/* split = [m/2] */
		int split = (GetEntry() + 1) / 2;
		/* replicate from 0 to key[split - 2] */
		for (i = 0, j = 0; i < split - 1; j++) {
				if (j == insert) {
						left->SetKeyAt(j, key);
						left->SetDataAt(j, data);
				} else {
						left->SetKeyAt(j, KeyAt(i));
						left->SetDataAt(j, ValueAt(i));
						i++;
				}
		}
		if (j == insert) {
				left->SetKeyAt(j, key);
				left->SetDataAt(j, data);
				j++;
		}
		left->SetEntry(j);
		/* left shift for right node */
		for (j = 0; i < GetEntry(); i++, j++) {
				SetKeyAt(j, KeyAt(i));
				SetDataAt(j, ValueAt(i));
		}
		SetEntry(j);
	}

	void BPTreeLeafPage::leaf_split_right(BPTreeLeafPage *right, int key, int data, int insert){
		int i, j;
		/* split = [m/2] */
		int split = (GetEntry() + 1) / 2;
		/* replicate from key[split] */
		for (i = split, j = 0; i < GetEntry(); j++) {
				if (j != insert - split) {
						right->SetKeyAt(j, KeyAt(i));
						right->SetDataAt(j, ValueAt(i));
						i++;
				}
		}
		/* reserve a hole for insertion */
		if (j > insert - split) {
				right->SetEntry(j);
		} else {
				right->SetEntry(j + 1);
		}
		/* insert new key */
		j = insert - split;
		right->SetKeyAt(j, key);
		right->SetDataAt(j, data);
		/* left leaf number */
		SetEntry(split);
	}

	void BPTreeLeafPage::leaf_simple_insert(int key, int data, int insert){
		int i;
		for (i = GetEntry(); i > insert; i--) {
				SetKeyAt(i, KeyAt(i - 1));
				SetDataAt(i, ValueAt(i - 1));
		}
		SetKeyAt(i, key);
		SetDataAt(i, data);
		SetEntry(GetEntry() + 1);
	}

	int BPTreeLeafPage::leaf_remove(BPlusTree* tree,int key)
	{
			int remove = key_binary_search(GetKeys(), GetEntry(), key);
			if (remove < 0) {
					/* Not exist */
					return -1;
			}

			if (GetEntry() <= (tree->GetEntry() + 1) / 2) {
					BPTreeInternalPage *parent = GetParent();
					BPTreeLeafPage *l_sib = GetLeftSib();
					BPTreeLeafPage *r_sib = GetRightSib();
					// if(l_sib != nullptr && l_sib->parent != parent) l_sib = nullptr;
					// if(r_sib != nullptr && r_sib->parent != parent) r_sib = nullptr;
					if (parent != nullptr) {
							/* decide which sibling to be borrowed from */
							int i = GetParentIndex();
							if (leaf_sibling_select(l_sib, r_sib, parent, i) == SiblingType::LEFT_SIBLING) {
									if (l_sib->GetEntry() > (tree->GetEntry() + 1) / 2) {
											leaf_shift_from_left(l_sib, i, remove);
									} else {
											leaf_merge_into_left(l_sib, remove);
											/* trace upwards */
											parent->non_leaf_remove(tree, i);
									}
							} else {
									/* remove first in case of overflow during merging with sibling */
									leaf_simple_remove(remove);
									if (r_sib->GetEntry() > (tree->GetEntry() + 1) / 2) {
											leaf_shift_from_right(r_sib, i + 1);
									} else {
											leaf_merge_from_right(r_sib);
											/* trace upwards */
											parent->non_leaf_remove(tree, i + 1);
									}
							}
					} else {
							if (GetEntry() == 1) {
									/* delete the only last node */
									tree->SetRoot(nullptr);
									leaf_delete(this);
									return 0;
							} else {
									leaf_simple_remove(remove);
							}
					}
			} else {
					leaf_simple_remove(remove);
			}

			return 0;
	}

	void BPTreeLeafPage::leaf_delete(BPTreeLeafPage *node)
	{
			if(node->GetLeftSib()) node->GetLeftSib()->SetRightSib(node->GetRightSib());
			if(node->GetRightSib()) node->GetRightSib()->SetLeftSib(node->GetLeftSib());
			delete node;
	}

	SiblingType BPTreeLeafPage::leaf_sibling_select( BPTreeLeafPage *l_sib,  BPTreeLeafPage *r_sib,
									BPTreeInternalPage *parent, int i)
	{
			if (i == -1) {
					/* the frist sub-node, no left sibling, choose the right one */
					return SiblingType::RIGHT_SIBLING;
			} else if (i == parent->GetChildren() - 2) {
					/* the last sub-node, no right sibling, choose the left one */
					return SiblingType::LEFT_SIBLING;
			} else {
					/* if both left and right sibling found, choose the one with more entries */
					return l_sib->GetEntry() >= r_sib->GetEntry() ? SiblingType::LEFT_SIBLING : SiblingType::RIGHT_SIBLING;
			}
	}

	void BPTreeLeafPage::leaf_shift_from_left(BPTreeLeafPage *left,
									int parent_key_index, int remove)
	{
			/* right shift in leaf node */
			for (; remove > 0; remove--) {
					SetKeyAt(remove, KeyAt(remove - 1));
					SetDataAt(remove, ValueAt(remove - 1));
			}
			/* borrow the last element from left sibling */
			SetKeyAt(0, left->KeyAt(left->GetEntry() - 1));
			SetDataAt(0, left->ValueAt(left->GetEntry() - 1));
			left->SetEntry(left->GetEntry() - 1);
			/* update parent key */
			GetParent()->SetKeyAt(parent_key_index, KeyAt(0));
	}

	void BPTreeLeafPage::leaf_merge_into_left(BPTreeLeafPage *left, int remove)
	{
			int i, j;
			/* merge into left sibling */
			for (i = left->GetEntry(), j = 0; j < GetEntry(); j++) {
					if (j != remove) {
							left->SetKeyAt(i, KeyAt(j));
							left->SetDataAt(i, ValueAt(j));
							i++;
					}
			}
			left->SetEntry(i);
			/* delete merged leaf */
			leaf_delete(this);
	}

	void BPTreeLeafPage::leaf_shift_from_right(BPTreeLeafPage *right, int parent_key_index)
	{
			int i;
			/* borrow the first element from right sibling */
			SetKeyAt(GetEntry(), right->KeyAt(0));
			SetDataAt(GetEntry(), right->ValueAt(0));

			SetEntry(GetEntry() + 1);
			/* left shift in right sibling */
			for (i = 0; i < right->GetEntry() - 1; i++) {
					right->SetKeyAt(i, right->KeyAt(i + 1));
					right->SetDataAt(i, right->ValueAt(i + 1));
			}
			right->SetEntry(right->GetEntry() - 1);
			/* update parent key */
			GetParent()->SetKeyAt(parent_key_index, right->KeyAt(0));
	}

	void BPTreeLeafPage::leaf_merge_from_right(BPTreeLeafPage *right)
	{
			int i, j;
			/* merge from right sibling */
			for (i = GetEntry(), j = 0; j < right->GetEntry(); i++, j++) {
					SetKeyAt(i, right->KeyAt(j));
					SetDataAt(i, right->ValueAt(j));
			}
			SetEntry(i);
			/* delete right sibling */
			leaf_delete(right);
	}

	void BPTreeLeafPage::leaf_simple_remove(int remove)
	{
			for (; remove < GetEntry() - 1; remove++) {
					SetKeyAt(remove, KeyAt(remove + 1));
					SetDataAt(remove, ValueAt(remove + 1));
			}
			SetEntry(GetEntry() - 1);
	}
} // namespace udb