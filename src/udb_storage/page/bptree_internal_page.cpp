#include "include/storage/page/bptree_internal_page.h"
#include "include/storage/index/bptree.h"

namespace udb
{
	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeInternalPage<KeyType, ValueType, KeyComparator>::parent_node_build(BPTree<KeyType, ValueType, KeyComparator>* tree, int page_id_left, int page_id_right, KeyType key, int level, BufferPool* buffer_pool){
		Page* left_page = buffer_pool->GetPage(page_id_left);
		BPTreePage* left = reinterpret_cast<BPTreeInternalPage*>(left_page->GetData());
		left_page->SetDirty();
		Page* right_page = buffer_pool->GetPage(page_id_right);
		BPTreePage* right = reinterpret_cast<BPTreeInternalPage*>(right_page->GetData());
		right_page->SetDirty();

		if (left->GetParent() == -1 && right->GetParent() == -1) {
			/* new parent */
			Page* parent_page = buffer_pool->NewPage();
			BPTreeInternalPage *parent = reinterpret_cast<BPTreeInternalPage*>(parent_page->GetData());
			parent_page->SetDirty();
			parent->Init(parent_page->GetPageId());

			parent->SetKeyAt(0, key);
			parent->SetPtrAt(0, left);
			parent->ValueAt(0)->SetParent(parent->GetPageId());
			parent->ValueAt(0)->SetParentIndex(-1);
			parent->SetPtrAt(1, right);
			parent->ValueAt(1)->SetParent(parent->GetPageId());
			parent->ValueAt(1)->SetParentIndex(0);
			parent->SetChildren(2);
			/* update root */
			tree->SetRoot(reinterpret_cast<BPTreePage *>(parent));
			return 0;
		} else if (right->GetParent() == -1) {
			/* trace upwards */
			right->SetParent(left->GetParent());
			Page* lparent_page = buffer_pool->GetPage(left->GetParent());
			BPTreeInternalPage *lparent = reinterpret_cast<BPTreeInternalPage*>(lparent_page->GetData());
			lparent_page->SetDirty();
			return lparent->non_leaf_insert(tree, left->GetPageId(), right->GetPageId(), key, level + 1, buffer_pool);
		} else {
			/* trace upwards */
			left->SetParent(right->GetParent());
			Page* rparent_page = buffer_pool->GetPage(right->GetParent());
			BPTreeInternalPage *rparent = reinterpret_cast<BPTreeInternalPage*>(rparent_page->GetData());
			rparent_page->SetDirty();
			return rparent->non_leaf_insert(tree, left->GetPageId(), right->GetPageId(), key, level + 1, buffer_pool);
		}
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_insert(BPTree<KeyType, ValueType, KeyComparator>* tree, int page_id_l_ch,  int page_id_r_ch, KeyType key, int level, BufferPool* buffer_pool){
			/* search key location */
			int insert = key_binary_search(GetChildren() - 1, key);
			insert = -insert - 1;

			Page* l_ch_page = buffer_pool->GetPage(page_id_l_ch);
			BPTreePage* l_ch = reinterpret_cast<BPTreeInternalPage*>(l_ch_page->GetData());
			l_ch_page->SetDirty();
			Page* r_ch_page = buffer_pool->GetPage(page_id_r_ch);
			BPTreePage* r_ch = reinterpret_cast<BPTreeInternalPage*>(r_ch_page->GetData());
			r_ch_page->SetDirty();

			/* node is full */
			if (GetChildren() == tree->GetOrder()) {
					/* split = [m/2] */
					int split_key;
					int split = (GetChildren() + 1) / 2;
					
					Page* sibling_page = buffer_pool->NewPage();
					BPTreeInternalPage* sibling = reinterpret_cast<BPTreeInternalPage*>(sibling_page->GetData());
					sibling_page->SetDirty();
					sibling->Init(sibling_page->GetPageId());

					if (insert < split) {
							split_key = non_leaf_split_left(sibling, l_ch, r_ch, key, insert, buffer_pool);
					} else if (insert == split) {
							split_key = non_leaf_split_right1(sibling, l_ch, r_ch, key, insert, buffer_pool);
					} else {
							split_key = non_leaf_split_right2(sibling, l_ch, r_ch, key, insert, buffer_pool);
					}
					/* build new parent */
					if (insert < split) {
							return parent_node_build(tree, sibling->GetPageId(),
									GetPageId(), split_key, level, buffer_pool);
					} else {
							return parent_node_build(tree, GetPageId(),
									sibling->GetPageId(), split_key, level, buffer_pool);
					}
			} else {
					non_leaf_simple_insert(l_ch, r_ch, key, insert);
			}

			return 0;
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_simple_insert(BPTreePage *l_ch, BPTreePage *r_ch, KeyType key, int insert){
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

	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeInternalPage<KeyType, ValueType, KeyComparator>::key_binary_search(int len, int target){
		int low = -1;
		int high = len;
		while (low + 1 < high) {
			int mid = low + (high - low) / 2;
			if (target > keys_[mid].first) {
					low = mid;
			} else {
					high = mid;
			}
		}
		if (high >= len || keys_[high].first != target) {
			return -high - 1;
		} else {
			return high;
		}
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_split_left( BPTreeInternalPage<KeyType, ValueType, KeyComparator> *left, BPTreePage *l_ch,  BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool){
		int i, j, order = GetChildren();
		int split_key;
		/* split = [m/2] */
		int split = (order + 1) / 2;
		left->SetLeftSib(GetLeftSib());
		left->SetRightSib(GetPageId());
		if(GetLeftSib() != -1) {
			Page* lsib_page = buffer_pool->GetPage(GetLeftSib());
			BPTreeInternalPage* lsib = reinterpret_cast<BPTreeInternalPage*>(lsib_page->GetData());
			lsib_page->SetDirty();
			lsib->SetRightSib(left->GetPageId());
		}
		SetLeftSib(left->GetPageId());
		/* replicate from sub[0] to sub[split - 1] */
		for (i = 0, j = 0; i < split; i++, j++) {
			if (j == insert) {
				left->SetPtrAt(j, l_ch);
				left->ValueAt(j)->SetParent(left->GetPageId());
				left->ValueAt(j)->SetParentIndex(j - 1);
				left->SetPtrAt(j + 1, r_ch);
				left->ValueAt(j + 1)->SetParent(left->GetPageId());
				left->ValueAt(j + 1)->SetParentIndex(j);
				j++;
			} else {
				left->SetPtrAt(j, ValueAt(i));
				left->ValueAt(j)->SetParent(left->GetPageId());
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
			left->ValueAt(insert)->SetParent(left->GetPageId());
			left->ValueAt(insert)->SetParentIndex(j - 1);
			SetPtrAt(0, r_ch);
			split_key = key;
		} else {
			SetPtrAt(0, ValueAt(split - 1));
			split_key = KeyAt(split - 2);
		}
		ValueAt(0)->SetParent(GetPageId());
		ValueAt(0)->SetParentIndex(-1);
		/* left shift for right node from split - 1 to children - 1 */
		for (i = split - 1, j = 0; i < order - 1; i++, j++) {
			SetKeyAt(j, KeyAt(i));
			SetPtrAt(j + 1, ValueAt(i + 1));
			ValueAt(j + 1)->SetParent(GetPageId());
			ValueAt(j + 1)->SetParentIndex(j);
		}
		SetPtrAt(j, ValueAt(i));
		SetChildren(j + 1);
		return split_key;
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_split_right1(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, BPTreePage *l_ch,  BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool){
		int i, j, order = GetChildren();
		int split_key;
		/* split = [m/2] */
		int split = (order + 1) / 2;

		right->SetRightSib(GetRightSib());
		right->SetLeftSib(GetPageId());
		if(GetRightSib() != -1) {
			Page* rsib_page = buffer_pool->GetPage(GetRightSib());
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rsib = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rsib_page->GetData());
			rsib_page->SetDirty();
			rsib->SetLeftSib(right->GetPageId());
		}
		SetRightSib(right->GetPageId());

		/* split key is key[split - 1] */
		split_key = KeyAt(split - 1);
		/* left node's children always be [split] */
		SetChildren(split);
		/* right node's first sub-node */
		right->SetKeyAt(0, key);
		right->SetPtrAt(0, l_ch);
		right->ValueAt(0)->SetParent(right->GetPageId());
		right->ValueAt(0)->SetParentIndex(-1);
		right->SetPtrAt(1, r_ch);
		right->ValueAt(1)->SetParent(right->GetPageId());
		right->ValueAt(1)->SetParentIndex(0);
		/* insertion point is split point, replicate from key[split] */
		for (i = split, j = 1; i < order - 1; i++, j++) {
			right->SetKeyAt(j,KeyAt(i));
			right->SetPtrAt(j + 1, ValueAt(i + 1));
			right->ValueAt(j + 1)->SetParent(right->GetPageId());
			right->ValueAt(j + 1)->SetParentIndex(j);
		}
		right->SetChildren(j + 1);
		return split_key;
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_split_right2(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, BPTreePage *l_ch,  BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool){
		int i, j, order = GetChildren();
		int split_key;
		/* split = [m/2] */
		int split = (order + 1) / 2;

		right->SetRightSib(GetRightSib());
		right->SetLeftSib(GetPageId());
		if(GetRightSib() != -1){
			Page* rsib_page = buffer_pool->GetPage(GetRightSib());
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rsib = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rsib_page->GetData());
			rsib_page->SetDirty();
			rsib->SetLeftSib(right->GetPageId());
		}
		SetRightSib(right->GetPageId());
		
		/* left node's children always be [split + 1] */
		SetChildren(split + 1);
		/* split as right sibling */
		/* split key is key[split] */
		split_key = KeyAt(split);
		/* right node's first sub-node */
		right->SetPtrAt(0, ValueAt(split + 1));
		right->ValueAt(0)->SetParent(right->GetPageId());
		right->ValueAt(0)->SetParentIndex(-1);
		/* replicate from key[split + 1] to key[order - 1] */
		for (i = split + 1, j = 0; i < order - 1; j++) {
			if (j != insert - split - 1) {
				right->SetKeyAt(j, KeyAt(i));
				right->SetPtrAt(j + 1, ValueAt(i + 1));
				right->ValueAt(j + 1)->SetParent(right->GetPageId());
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
		right->ValueAt(j)->SetParent(right->GetPageId());
		right->ValueAt(j)->SetParentIndex(j - 1);
		right->SetPtrAt(j + 1, r_ch);
		right->ValueAt(j + 1)->SetParent(right->GetPageId());
		right->ValueAt(j + 1)->SetParentIndex(j);
		return split_key;
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_delete(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *node, BufferPool* buffer_pool)
	{
		if(node->GetLeftSib() == -1 && node->GetRightSib() == -1) return;
		else{
			if(node->GetLeftSib() != -1) {
				Page* lsib_page = buffer_pool->GetPage(node->GetLeftSib());
				BPTreeInternalPage<KeyType, ValueType, KeyComparator>* lsib = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(lsib_page->GetData());
				lsib_page->SetDirty();
				lsib->SetRightSib(node->GetRightSib());
			}
			if(node->GetRightSib() != -1) {
				Page* rsib_page = buffer_pool->GetPage(node->GetRightSib());
				BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rsib = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rsib_page->GetData());
				rsib_page->SetDirty();
				rsib->SetLeftSib(node->GetLeftSib());
			}
			//delete node;
		}
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	SiblingType BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_sibling_select(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *l_sib, 
																							BPTreeInternalPage<KeyType, ValueType, KeyComparator> *r_sib, 
																							BPTreeInternalPage<KeyType, ValueType, KeyComparator> *parent, int i)
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

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_shift_from_left(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *left,int parent_key_index, int remove, BufferPool* buffer_pool)
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
		Page* parent_page = buffer_pool->GetPage(GetParent());
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
		parent_page->SetDirty();
		SetKeyAt(0, parent->KeyAt(parent_key_index));
		parent->SetKeyAt(parent_key_index, left->KeyAt(left->GetChildren() - 2));

		/* borrow the last sub-node from left sibling */
		SetPtrAt(0, left->ValueAt(left->GetChildren() - 1));
		ValueAt(0)->SetParent(GetPageId());
		ValueAt(0)->SetParentIndex(-1);
		left->SetChildren(left->GetChildren() - 1);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_merge_into_left(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *left, int parent_key_index, int remove, BufferPool* buffer_pool)
	{
		int i, j;
		/* move parent key down */
		Page* parent_page = buffer_pool->GetPage(GetParent());
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
		parent_page->SetDirty();
		left->SetKeyAt(left->GetChildren() - 1, parent->KeyAt(parent_key_index));
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
					left->ValueAt(i)->SetParent(left->GetPageId());
					left->ValueAt(i)->SetParentIndex(i - 1);
					i++;
			}
		}
		left->SetChildren(i);
		/* delete empty node */
		non_leaf_delete(this, buffer_pool);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_shift_from_right(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, int parent_key_index, BufferPool* buffer_pool)
	{
		int i;
		/* parent key left rotation */
		Page* parent_page = buffer_pool->GetPage(GetParent());
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
		parent_page->SetDirty();
		SetKeyAt(GetChildren() - 1, parent->KeyAt(parent_key_index));
		parent->SetKeyAt(parent_key_index, right->KeyAt(0));
		/* borrow the frist sub-node from right sibling */
		SetPtrAt(GetChildren(), right->ValueAt(0));
		ValueAt(GetChildren())->SetParent(GetPageId());
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

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_merge_from_right(BPTreeInternalPage<KeyType, ValueType, KeyComparator> *right, int parent_key_index, BufferPool* buffer_pool)
	{
		int i, j;
		/* move parent key down */
		Page* parent_page = buffer_pool->GetPage(GetParent());
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
		parent_page->SetDirty();
		SetKeyAt(GetChildren() - 1, parent->KeyAt(parent_key_index));
		SetChildren(GetChildren() + 1);
		/* merge from right sibling */
		for (i = GetChildren() - 1, j = 0; j < right->GetChildren() - 1; i++, j++) {
			SetKeyAt(i, right->KeyAt(j));
		}
		for (i = GetChildren() - 1, j = 0; j < right->GetChildren(); i++, j++) {
			SetPtrAt(i, right->ValueAt(j));
			ValueAt(i)->SetParent(GetPageId());
			ValueAt(i)->SetParentIndex(i - 1);
		}
		SetChildren(i);
		/* delete empty right sibling */
		non_leaf_delete(right, buffer_pool);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_simple_remove(int remove)
	{
		for (; remove < GetChildren() - 2; remove++) {
			SetKeyAt(remove, KeyAt(remove + 1));
			SetPtrAt(remove + 1, ValueAt(remove + 2));
			ValueAt(remove + 1)->SetParentIndex(remove);
		}
		SetChildren(GetChildren() - 1);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_remove(BPTree<KeyType, ValueType, KeyComparator>* tree, int remove, BufferPool* buffer_pool)
	{
		if (GetChildren() <= (tree->GetOrder() + 1) / 2) {
			Page* parent_page = buffer_pool->GetPage(GetParent());
			Page* lsib_page = buffer_pool->GetPage(GetLeftSib());
			Page* rsib_page = buffer_pool->GetPage(GetRightSib());
			BPTreeInternalPage<KeyType, ValueType, KeyComparator> *parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
			parent_page->SetDirty();
			BPTreeInternalPage<KeyType, ValueType, KeyComparator> *l_sib = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(lsib_page->GetData());
			lsib_page->SetDirty();
			BPTreeInternalPage<KeyType, ValueType, KeyComparator> *r_sib = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rsib_page->GetData());
			rsib_page->SetDirty();

			if (GetParent() != -1) {
				/* decide which sibling to be borrowed from */
				int i = GetParentIndex();
				if (non_leaf_sibling_select(l_sib, r_sib, parent, i) == SiblingType::LEFT_SIBLING) {
					if (l_sib->GetChildren() > (tree->GetOrder() + 1) / 2) {
						non_leaf_shift_from_left(l_sib, i, remove, buffer_pool);
					} else {
						non_leaf_merge_into_left(l_sib, i, remove, buffer_pool);
						/* trace upwards */
						parent->non_leaf_remove(tree, i, buffer_pool);
					}
				} else {
					/* remove first in case of overflow during merging with sibling */
					non_leaf_simple_remove(remove);
					if (r_sib->GetChildren() > (tree->GetOrder() + 1) / 2) {
						non_leaf_shift_from_right(r_sib, i + 1, buffer_pool);
					} else {
						non_leaf_merge_from_right(r_sib, i + 1, buffer_pool);
						/* trace upwards */
						parent->non_leaf_remove(tree, i + 1, buffer_pool);
					}
				}
			} else {
				if (GetChildren() == 2) {
					/* delete old root node */
					ValueAt(0)->SetParent(-1);
					tree->SetRoot(ValueAt(0));
					non_leaf_delete(this, buffer_pool);
					tree->SetLevel(tree->GetLevel() - 1);
				} else {
					non_leaf_simple_remove(remove);
				}
			}
		} else {
			non_leaf_simple_remove(remove);
		}
	}

	template class BPTreeInternalPage<int, RID, IntComparator>;
} // namespace udb