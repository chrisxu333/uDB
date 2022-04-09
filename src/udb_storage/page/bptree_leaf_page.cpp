#include "include/storage/page/bptree_leaf_page.h"
#include "include/storage/index/bptree.h"

namespace udb
{
	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeLeafPage<KeyType, ValueType, KeyComparator>::key_binary_search(int len, int target){
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
	int BPTreeLeafPage<KeyType, ValueType, KeyComparator>::parent_node_build(BPTree<KeyType, ValueType, KeyComparator>* tree, int page_id_left, int page_id_right, KeyType key, int level, BufferPool* buffer_pool){
			Page* left_page = buffer_pool->GetPage(page_id_left);
			BPTreePage* left = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(left_page->GetData());
			left_page->SetDirty();
			Page* right_page = buffer_pool->GetPage(page_id_right);
			BPTreePage* right = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(right_page->GetData());
			right_page->SetDirty();

			if (left->GetParent() == -1 && right->GetParent() == -1) {
					/* new parent */
					Page* parent_page = buffer_pool->NewPage();
					BPTreeInternalPage<KeyType, ValueType, KeyComparator> *parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
					parent->Init(parent_page->GetPageId());

					parent->SetKeyAt(0, key);
					parent->SetPtrAt(0, left->GetPageId());

					// Page* pzero_page = buffer_pool->GetPage(parent->ValueAt(0));
					// BPTreeInternalPage<KeyType, ValueType, KeyComparator> *pzero = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(pzero_page->GetData());
					// pzero_page->SetDirty();
					// pzero->SetParent(parent->GetPageId());
					// pzero->SetParentIndex(-1);
					// buffer_pool->UnPin(pzero_page->GetPageId());

					// parent->SetPtrAt(1, right->GetPageId());

					// Page* pone_page = buffer_pool->GetPage(parent->ValueAt(1));
					// BPTreeInternalPage<KeyType, ValueType, KeyComparator> *pone = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(pone_page->GetData());
					// pone_page->SetDirty();	
					// pone->SetParent(parent->GetPageId());
					// pone->SetParentIndex(0);
					// buffer_pool->UnPin(pone_page->GetPageId());

					left->SetParent(parent->GetPageId());
					left->SetParentIndex(-1);

					parent->SetPtrAt(1, right->GetPageId());
					right->SetParent(parent->GetPageId());
					right->SetParentIndex(0);

					parent->SetChildren(2);
					/* update root */
					tree->SetRoot(parent->GetPageId());
					buffer_pool->UnPin(left_page->GetPageId());
					buffer_pool->UnPin(right_page->GetPageId());
					return 0;
			} else if (right->GetParent() == -1) {
					/* trace upwards */
					right->SetParent(left->GetParent());
					Page* lparent_page = buffer_pool->GetPage(left->GetParent());
					lparent_page->SetDirty();
					BPTreeInternalPage<KeyType, ValueType, KeyComparator> *lparent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(lparent_page->GetData());
					return lparent->non_leaf_insert(tree, left->GetPageId(), right->GetPageId(), key, level + 1, buffer_pool);
			} else {
					/* trace upwards */
					left->SetParent(right->GetParent());
					Page* rparent_page = buffer_pool->GetPage(right->GetParent());
					BPTreeInternalPage<KeyType, ValueType, KeyComparator> *rparent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rparent_page->GetData());
					rparent_page->SetDirty();
					return rparent->non_leaf_insert(tree, left->GetPageId(), right->GetPageId(), key, level + 1, buffer_pool);
			}
	}

	// public APIs =========================================================================
	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_insert(BPTree<KeyType, ValueType, KeyComparator>* tree, KeyType key, ValueType data, BufferPool* buffer_pool){
			/* search key location */
			int insert = key_binary_search(GetEntry(), key);
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
				Page* sibling_page = buffer_pool->NewPage();
				BPTreeLeafPage<KeyType, ValueType, KeyComparator>* sibling = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(sibling_page->GetData());
				sibling->Init(sibling_page->GetPageId());
				/* sibling leaf replication due to location of insertion */
				if (insert < split) {
					sibling->SetRightSib(GetPageId());
					sibling->SetLeftSib(GetLeftSib());
					if(GetLeftSib() != -1) {
						Page* lsib_page = buffer_pool->GetPage(GetLeftSib());
						BPTreeLeafPage<KeyType, ValueType, KeyComparator> *lsib = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(lsib_page->GetData());
						lsib_page->SetDirty();
						lsib->SetRightSib(sibling->GetPageId());
					}
					SetLeftSib(sibling->GetPageId());
					leaf_split_left(sibling, key, data, insert);
				} else {
					sibling->SetLeftSib(GetPageId());
					sibling->SetRightSib(GetRightSib());
					if(GetRightSib() != -1) {
						Page* rsib_page = buffer_pool->GetPage(GetRightSib());
						BPTreeLeafPage<KeyType, ValueType, KeyComparator> *rsib = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(rsib_page->GetData());
						rsib_page->SetDirty();
						rsib->SetLeftSib(sibling->GetPageId());
					}
					SetRightSib(sibling->GetPageId());
					leaf_split_right(sibling, key, data, insert);
				}

				/* build new parent */
				if (insert < split) {
					return parent_node_build(tree, sibling->GetPageId(),
								GetPageId(), KeyAt(0), 0, buffer_pool);
				} else {
					return parent_node_build(tree, GetPageId(),
								sibling->GetPageId(), sibling->KeyAt(0), 0, buffer_pool);
				}
			} else {
				leaf_simple_insert(key, data, insert);
			}

			return 0;
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_split_left(BPTreeLeafPage<KeyType, ValueType, KeyComparator> *left, KeyType key, ValueType data, int insert){
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

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_split_right(BPTreeLeafPage<KeyType, ValueType, KeyComparator> *right, KeyType key, ValueType data, int insert){
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

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_simple_insert(KeyType key, ValueType data, int insert){
			int i;
			for (i = GetEntry(); i > insert; i--) {
					SetKeyAt(i, KeyAt(i - 1));
					SetDataAt(i, ValueAt(i - 1));
			}
			SetKeyAt(i, key);
			SetDataAt(i, data);
			SetEntry(GetEntry() + 1);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	int BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_remove(BPTree<KeyType, ValueType, KeyComparator>* tree, KeyType key, BufferPool* buffer_pool)
	{
			int remove = key_binary_search(GetEntry(), key);
			if (remove < 0) {
					/* Not exist */
					return -1;
			}

			if (GetEntry() <= (tree->GetEntry() + 1) / 2) {
					Page* parent_page = buffer_pool->GetPage(GetParent());
					Page* lsib_page = buffer_pool->GetPage(GetLeftSib());
					Page* rsib_page = buffer_pool->GetPage(GetRightSib());
					BPTreeInternalPage<KeyType, ValueType, KeyComparator> *parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
					parent_page->SetDirty();
					BPTreeLeafPage<KeyType, ValueType, KeyComparator> *l_sib = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(lsib_page->GetData());
					lsib_page->SetDirty();
					BPTreeLeafPage<KeyType, ValueType, KeyComparator> *r_sib = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(rsib_page->GetData());
					rsib_page->SetDirty();

					if (GetParent() != -1) {		// there is a parent
							/* decide which sibling to be borrowed from */
							int i = GetParentIndex();
							if (leaf_sibling_select(l_sib, r_sib, parent, i) == SiblingType::LEFT_SIBLING) {
									if (l_sib->GetEntry() > (tree->GetEntry() + 1) / 2) {
											leaf_shift_from_left(l_sib, i, remove, buffer_pool);
									} else {
											leaf_merge_into_left(l_sib, remove, buffer_pool);
											/* trace upwards */
											parent->non_leaf_remove(tree, i, buffer_pool);
									}
							} else {
									/* remove first in case of overflow during merging with sibling */
									leaf_simple_remove(remove);
									if (r_sib->GetEntry() > (tree->GetEntry() + 1) / 2) {
											leaf_shift_from_right(r_sib, i + 1, buffer_pool);
									} else {
											leaf_merge_from_right(r_sib, buffer_pool);
											/* trace upwards */
											parent->non_leaf_remove(tree, i + 1, buffer_pool);
									}
							}
					} else {
							if (GetEntry() == 1) {
									/* delete the only last node */
									tree->SetRoot(INVALID_PAGE_ID);
									leaf_delete(this, buffer_pool);
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

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_delete(BPTreeLeafPage<KeyType, ValueType, KeyComparator> *node, BufferPool* buffer_pool)
	{
			if(node->GetLeftSib() != -1) {
				Page* lsib_page = buffer_pool->GetPage(node->GetLeftSib());
				BPTreeLeafPage<KeyType, ValueType, KeyComparator>* lsib = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(lsib_page->GetData());
				lsib_page->SetDirty();
				lsib->SetRightSib(node->GetRightSib());
			}
			if(node->GetRightSib() != -1){
				Page* rsib_page = buffer_pool->GetPage(node->GetRightSib());
				BPTreeLeafPage<KeyType, ValueType, KeyComparator>* rsib = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(rsib_page->GetData());
				rsib_page->SetDirty();
				rsib->SetLeftSib(node->GetLeftSib());
			}
			//delete node;
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	SiblingType BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_sibling_select( BPTreeLeafPage<KeyType, ValueType, KeyComparator> *l_sib,  BPTreeLeafPage<KeyType, ValueType, KeyComparator> *r_sib,
									BPTreeInternalPage<KeyType, ValueType, KeyComparator> *parent, int i)
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

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_shift_from_left(BPTreeLeafPage<KeyType, ValueType, KeyComparator> *left, int parent_key_index, int remove, BufferPool* buffer_pool)
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
			Page* parent_page = buffer_pool->GetPage(GetParent());
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
			parent_page->SetDirty();
			parent->SetKeyAt(parent_key_index, KeyAt(0));
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_merge_into_left(BPTreeLeafPage<KeyType, ValueType, KeyComparator> *left, int remove, BufferPool* buffer_pool)
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
			leaf_delete(this, buffer_pool);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_shift_from_right(BPTreeLeafPage<KeyType, ValueType, KeyComparator> *right, int parent_key_index, BufferPool* buffer_pool)
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
			Page* parent_page = buffer_pool->GetPage(GetParent());
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
			parent_page->SetDirty();
			parent->SetKeyAt(parent_key_index, right->KeyAt(0));
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_merge_from_right(BPTreeLeafPage<KeyType, ValueType, KeyComparator> *right, BufferPool* buffer_pool)
	{
			int i, j;
			/* merge from right sibling */
			for (i = GetEntry(), j = 0; j < right->GetEntry(); i++, j++) {
					SetKeyAt(i, right->KeyAt(j));
					SetDataAt(i, right->ValueAt(j));
			}
			SetEntry(i);
			/* delete right sibling */
			leaf_delete(right, buffer_pool);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator> 
	void BPTreeLeafPage<KeyType, ValueType, KeyComparator>::leaf_simple_remove(int remove)
	{
			for (; remove < GetEntry() - 1; remove++) {
					SetKeyAt(remove, KeyAt(remove + 1));
					SetDataAt(remove, ValueAt(remove + 1));
			}
			SetEntry(GetEntry() - 1);
	}

	template class BPTreeLeafPage<int, RID, IntComparator>;
} // namespace udb