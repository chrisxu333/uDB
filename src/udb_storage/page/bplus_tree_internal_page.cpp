#include "include/storage/page/bplus_tree_internal_page.h"
#include "include/storage/index/bplus_tree.h"

namespace udb
{
	int BPTreeInternalPage::parent_node_build(BPlusTree* tree, BPlusTreePage *left, BPlusTreePage *right, int key, int level){
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

	int BPTreeInternalPage::non_leaf_insert(BPlusTree* tree, BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int level){
		/* search key location */
		int insert = key_binary_search(GetKeys(), GetChildren() - 1, key);
		insert = -insert - 1;

		/* node is full */
		if (GetChildren() == tree->GetOrder()) {
				/* split = [m/2] */
				int split_key;
				int split = (GetChildren() + 1) / 2;
				BPTreeInternalPage *sibling = new BPTreeInternalPage();
				if (insert < split) {
						split_key = non_leaf_split_left(sibling, l_ch, r_ch, key, insert);
				} else if (insert == split) {
						split_key = non_leaf_split_right1(sibling, l_ch, r_ch, key, insert);
				} else {
						split_key = non_leaf_split_right2(sibling, l_ch, r_ch, key, insert);
				}
				/* build new parent */
				if (insert < split) {
						return parent_node_build(tree, (BPlusTreePage *)sibling,
								(BPlusTreePage *)this, split_key, level);
				} else {
						return parent_node_build(tree, (BPlusTreePage *)this,
								(BPlusTreePage *)sibling, split_key, level);
				}
		} else {
				non_leaf_simple_insert(l_ch, r_ch, key, insert);
		}

		return 0;
	}


	void BPTreeInternalPage::non_leaf_simple_insert(BPlusTreePage *l_ch, BPlusTreePage *r_ch, int key, int insert){
		int i;
		for (i = GetChildren() - 1; i > insert; i--) {
				SetKeyAt(i, KeyAt(i - 1));
				SetPtrAt(i + 1, ValueAt(i));
				ValueAt(i + 1)->SetParentIndex(i);
		}
		SetKeyAt(i, key);
		SetPtrAt(i, l_ch);
		ValueAt(i)->SetParentIndex(i - 1);
		SetPtrAt(i + 1, r_ch);
		ValueAt(i + 1)->SetParentIndex(i);
		SetChildren(GetChildren() + 1);
	}

	int BPTreeInternalPage::key_binary_search(int *arr, int len, int target){
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

	int BPTreeInternalPage::non_leaf_split_left( BPTreeInternalPage *left, BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int insert){
		int i, j, order = GetChildren();
		int split_key;
		/* split = [m/2] */
		int split = (order + 1) / 2;
		left->SetLeftSib(GetLeftSib());
		left->SetRightSib(this);
		if(GetLeftSib()) GetLeftSib()->SetRightSib(left);
		SetLeftSib(left);
		/* replicate from sub[0] to sub[split - 1] */
		for (i = 0, j = 0; i < split; i++, j++) {
				if (j == insert) {
						left->SetPtrAt(j, l_ch);
						left->ValueAt(j)->SetParent(left);
						left->ValueAt(j)->SetParentIndex(j - 1);
						left->SetPtrAt(j + 1, r_ch);
						left->ValueAt(j + 1)->SetParent(left);
						left->ValueAt(j + 1)->SetParentIndex(j);
						j++;
				} else {
						left->SetPtrAt(j, ValueAt(i));
						left->ValueAt(j)->SetParent(left);
						left->ValueAt(j)->SetParentIndex(j - 1);
				}
		}
		left->SetChildren(split);
		/* replicate from key[0] to key[split - 2] */
		for (i = 0, j = 0; i < split - 1; j++) {
				if (j == insert) {
						left->SetKeyAt(j, key);
				} else {
						left->SetKeyAt(j, KeyAt(i));
						i++;
				}
		}
		if (insert == split - 1) {
				left->SetKeyAt(insert, key);
				left->SetPtrAt(insert, l_ch);
				left->ValueAt(insert)->SetParent(left);
				left->ValueAt(insert)->SetParentIndex(j - 1);
				SetPtrAt(0, r_ch);
				split_key = key;
		} else {
				SetPtrAt(0, ValueAt(split - 1));
				split_key = KeyAt(split - 2);
		}
		ValueAt(0)->SetParent(this);
		ValueAt(0)->SetParentIndex(-1);
		/* left shift for right node from split - 1 to children - 1 */
		for (i = split - 1, j = 0; i < order - 1; i++, j++) {
				SetKeyAt(j, KeyAt(i));
				SetPtrAt(j + 1, ValueAt(i + 1));
				ValueAt(j + 1)->SetParent(this);
				ValueAt(j + 1)->SetParentIndex(j);
		}
		SetPtrAt(j, ValueAt(i));
		SetChildren(j + 1);
		return split_key;
	}


	int BPTreeInternalPage::non_leaf_split_right1(BPTreeInternalPage *right, BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int insert){
		int i, j, order = GetChildren();
		int split_key;
		/* split = [m/2] */
		int split = (order + 1) / 2;

		right->SetRightSib(GetRightSib());
		right->SetLeftSib(this);
		if(GetRightSib()) GetRightSib()->SetLeftSib(right);
		SetRightSib(right);

		/* split key is key[split - 1] */
		split_key = KeyAt(split - 1);
		/* left node's children always be [split] */
		SetChildren(split);
		/* right node's first sub-node */
		right->SetKeyAt(0, key);
		right->SetPtrAt(0, l_ch);
		right->ValueAt(0)->SetParent(right);
		right->ValueAt(0)->SetParentIndex(-1);
		right->SetPtrAt(1, r_ch);
		right->ValueAt(1)->SetParent(right);
		right->ValueAt(1)->SetParentIndex(0);
		/* insertion point is split point, replicate from key[split] */
		for (i = split, j = 1; i < order - 1; i++, j++) {
				right->SetKeyAt(j,KeyAt(i));
				right->SetPtrAt(j + 1, ValueAt(i + 1));
				right->ValueAt(j + 1)->SetParent(right);
				right->ValueAt(j + 1)->SetParentIndex(j);
		}
		right->SetChildren(j + 1);
		return split_key;
	}

	int BPTreeInternalPage::non_leaf_split_right2(BPTreeInternalPage *right,
								BPlusTreePage *l_ch,  BPlusTreePage *r_ch, int key, int insert){
		int i, j, order = GetChildren();
		int split_key;
		/* split = [m/2] */
		int split = (order + 1) / 2;

		right->SetRightSib(GetRightSib());
		right->SetLeftSib(this);
		if(GetRightSib())GetRightSib()->SetLeftSib(right);
		SetRightSib(right);
		
		/* left node's children always be [split + 1] */
		SetChildren(split + 1);
		/* split as right sibling */
		/* split key is key[split] */
		split_key = KeyAt(split);
		/* right node's first sub-node */
		right->SetPtrAt(0, ValueAt(split + 1));
		right->ValueAt(0)->SetParent(right);
		right->ValueAt(0)->SetParentIndex(-1);
		/* replicate from key[split + 1] to key[order - 1] */
		for (i = split + 1, j = 0; i < order - 1; j++) {
				if (j != insert - split - 1) {
						right->SetKeyAt(j, KeyAt(i));
						right->SetPtrAt(j + 1, ValueAt(i + 1));
						right->ValueAt(j + 1)->SetParent(right);
						right->ValueAt(j + 1)->SetParentIndex(j);
						i++;
				}
		}
		/* reserve a hole for insertion */
		if (j > insert - split - 1) {
				right->SetChildren(j + 1);
		} else {
				right->SetChildren(j + 2);
		}
		/* insert new key and sub-node */
		j = insert - split - 1;
		right->SetKeyAt(j, key);
		right->SetPtrAt(j, l_ch);
		right->ValueAt(j)->SetParent(right);
		right->ValueAt(j)->SetParentIndex(j - 1);
		right->SetPtrAt(j + 1, r_ch);
		right->ValueAt(j + 1)->SetParent(right);
		right->ValueAt(j + 1)->SetParentIndex(j);
		return split_key;
	}

	void BPTreeInternalPage::non_leaf_delete(BPTreeInternalPage *node)
	{
			if(node->GetLeftSib() == nullptr && node->GetRightSib() == nullptr) delete node;
			else{
					if(node->GetLeftSib()) node->GetLeftSib()->SetRightSib(node->GetRightSib());
					if(node->GetRightSib()) node->GetRightSib()->SetLeftSib(node->GetLeftSib());
					delete node;
			}
	}


	SiblingType BPTreeInternalPage::non_leaf_sibling_select(BPTreeInternalPage *l_sib, BPTreeInternalPage *r_sib,
									BPTreeInternalPage *parent, int i)
	{
			if (i == -1) {
					/* the frist sub-node, no left sibling, choose the right one */
					return SiblingType::RIGHT_SIBLING;
			} else if (i == parent->GetChildren() - 2) {
					/* the last sub-node, no right sibling, choose the left one */
					return SiblingType::LEFT_SIBLING;
			} else{
					/* if both left and right sibling found, choose the one with more entries */
					return l_sib->GetChildren() >= r_sib->GetChildren() ? SiblingType::LEFT_SIBLING : SiblingType::RIGHT_SIBLING;
			}
	}

	void BPTreeInternalPage::non_leaf_shift_from_left(BPTreeInternalPage *left,int parent_key_index, int remove)
	{
			int i;
			/* node's elements right shift */
			for (i = remove; i > 0; i--) {
					SetKeyAt(i, KeyAt(i - 1));
			}
			for (i = remove + 1; i > 0; i--) {
					SetPtrAt(i, ValueAt(i - 1));
					ValueAt(i)->SetParentIndex(i - 1);
			}
			/* parent key right rotation */
			SetKeyAt(0, GetParent()->KeyAt(parent_key_index));
			GetParent()->SetKeyAt(parent_key_index, left->KeyAt(left->GetChildren() - 2));
			/* borrow the last sub-node from left sibling */
			SetPtrAt(0, left->ValueAt(left->GetChildren() - 1));
			ValueAt(0)->SetParent(this);
			ValueAt(0)->SetParentIndex(-1);
			left->SetChildren(left->GetChildren() - 1);
	}

	void BPTreeInternalPage::non_leaf_merge_into_left(BPTreeInternalPage *left, int parent_key_index, int remove)
	{
			int i, j;
			/* move parent key down */
			left->SetKeyAt(left->GetChildren() - 1, GetParent()->KeyAt(parent_key_index));
			/* merge into left sibling */
			for (i = left->GetChildren(), j = 0; j < GetChildren() - 1; j++) {
					if (j != remove) {
							left->SetKeyAt(i,KeyAt(j));
							i++;
					}
			}
			for (i = left->GetChildren(), j = 0; j < GetChildren(); j++) {
					if (j != remove + 1) {
							left->SetPtrAt(i, ValueAt(j));
							left->ValueAt(i)->SetParent(left);
							left->ValueAt(i)->SetParentIndex(i - 1);
							i++;
					}
			}
			left->SetChildren(i);
			/* delete empty node */
			non_leaf_delete(this);
	}

	void BPTreeInternalPage::non_leaf_shift_from_right(BPTreeInternalPage *right,
											int parent_key_index)
	{
			int i;
			/* parent key left rotation */
			SetKeyAt(GetChildren() - 1, GetParent()->KeyAt(parent_key_index));
			GetParent()->SetKeyAt(parent_key_index, right->KeyAt(0));
			/* borrow the frist sub-node from right sibling */
			SetPtrAt(GetChildren(), right->ValueAt(0));
			ValueAt(GetChildren())->SetParent(this);
			ValueAt(GetChildren())->SetParentIndex(GetChildren() - 1);
			SetChildren(GetChildren() + 1);
			/* left shift in right sibling */
			for (i = 0; i < right->GetChildren() - 2; i++) {
					right->SetKeyAt(i, right->KeyAt(i + 1));
			}
			for (i = 0; i < right->GetChildren() - 1; i++) {
					right->SetPtrAt(i, right->ValueAt(i + 1));
					right->ValueAt(i)->SetParentIndex(i - 1);
			}
			right->SetChildren(right->GetChildren() - 1);
	}

	void BPTreeInternalPage::non_leaf_merge_from_right(BPTreeInternalPage *right,
											int parent_key_index)
	{
			int i, j;
			/* move parent key down */
			SetKeyAt(GetChildren() - 1, GetParent()->KeyAt(parent_key_index));
			SetChildren(GetChildren() + 1);
			/* merge from right sibling */
			for (i = GetChildren() - 1, j = 0; j < right->GetChildren() - 1; i++, j++) {
					SetKeyAt(i, right->KeyAt(j));
			}
			for (i = GetChildren() - 1, j = 0; j < right->GetChildren(); i++, j++) {
					SetPtrAt(i, right->ValueAt(j));
					ValueAt(i)->SetParent(this);
					ValueAt(i)->SetParentIndex(i - 1);
			}
			SetChildren(i);
			/* delete empty right sibling */
			non_leaf_delete(right);
	}

	void BPTreeInternalPage::non_leaf_simple_remove(int remove)
	{
			for (; remove < GetChildren() - 2; remove++) {
					SetKeyAt(remove, KeyAt(remove + 1));
					SetPtrAt(remove + 1, ValueAt(remove + 2));
					ValueAt(remove + 1)->SetParentIndex(remove);
			}
			SetChildren(GetChildren() - 1);
	}

	void BPTreeInternalPage::non_leaf_remove(BPlusTree* tree, int remove)
	{
			printf("non leaf remove\n");
			if (GetChildren() <= (tree->GetOrder() + 1) / 2) {
					BPTreeInternalPage *parent = GetParent();
					BPTreeInternalPage *l_sib = GetLeftSib();
					BPTreeInternalPage *r_sib = GetRightSib();
					if (parent != nullptr) {
							/* decide which sibling to be borrowed from */
							int i = GetParentIndex();
							if (non_leaf_sibling_select(l_sib, r_sib, parent, i) == SiblingType::LEFT_SIBLING) {
									printf("choose left sibing\n");
									if (l_sib->GetChildren() > (tree->GetOrder() + 1) / 2) {
											non_leaf_shift_from_left(l_sib, i, remove);
									} else {
											printf("choose merge to left\n");
											non_leaf_merge_into_left(l_sib, i, remove);
											/* trace upwards */
											parent->non_leaf_remove(tree, i);
									}
							} else {
									printf("choose right sibling\n");
									/* remove first in case of overflow during merging with sibling */
									non_leaf_simple_remove(remove);
									if (r_sib->GetChildren() > (tree->GetOrder() + 1) / 2) {
											non_leaf_shift_from_right(r_sib, i + 1);
									} else {
											printf("choose merge to right\n");
											non_leaf_merge_from_right(r_sib, i + 1);
											/* trace upwards */
											parent->non_leaf_remove(tree, i + 1);
									}
							}
					} else {
							if (GetChildren() == 2) {
									/* delete old root node */
									ValueAt(0)->SetParent(nullptr);
									tree->SetRoot(ValueAt(0));
									non_leaf_delete(this);
									tree->SetLevel(tree->GetLevel() - 1);
							} else {
									non_leaf_simple_remove(remove);
							}
					}
			} else {
					non_leaf_simple_remove(remove);
			}
	}
} // namespace udb