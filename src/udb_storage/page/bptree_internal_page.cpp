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
			parent->SetPtrAt(0, left->GetPageId());

			Page* zero_page = buffer_pool->GetPage(parent->ValueAt(0));
			BPTreePage* zero = reinterpret_cast<BPTreeInternalPage*>(zero_page->GetData());
			zero->SetParent(parent->GetPageId());
			zero->SetParentIndex(-1);
			parent->SetPtrAt(1, right->GetPageId());

			
			Page* first_page = buffer_pool->GetPage(parent->ValueAt(1));
			BPTreePage* first = reinterpret_cast<BPTreeInternalPage*>(first_page->GetData());
			first->SetParent(parent->GetPageId());
			first->SetParentIndex(0);

			parent->SetChildren(2);
			/* update root */
			tree->SetRoot(reinterpret_cast<BPTreePage *>(parent)->GetPageId());
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
					non_leaf_simple_insert(l_ch, r_ch, key, insert, buffer_pool);
			}

			return 0;
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_simple_insert(BPTreePage *l_ch, BPTreePage *r_ch, KeyType key, int insert, BufferPool* buffer_pool){
		int i;
		for (i = GetChildren() - 1; i > insert; i--) {
			SetKeyAt(i, KeyAt(i - 1));
			SetPtrAt(i + 1, ValueAt(i));
			Page* tmp_page = buffer_pool->GetPage(ValueAt(i + 1));
			tmp_page->SetDirty();
			BPTreePage* tmp = reinterpret_cast<BPTreeInternalPage*>(tmp_page->GetData());
			tmp->SetParentIndex(i);
			buffer_pool->UnPin(tmp->GetPageId());
		}
		SetKeyAt(i, key);
		SetPtrAt(i, l_ch->GetPageId());

		Page* tmp_page = buffer_pool->GetPage(ValueAt(i));
		tmp_page->SetDirty();
		BPTreePage* tmp = reinterpret_cast<BPTreeInternalPage*>(tmp_page->GetData());
		tmp->SetParentIndex(i - 1);
		buffer_pool->UnPin(tmp_page->GetPageId());

		SetPtrAt(i + 1, r_ch->GetPageId());

		Page* tmpplusone_page = buffer_pool->GetPage(ValueAt(i + 1));
		tmpplusone_page->SetDirty();
		BPTreePage* tmpplusone = reinterpret_cast<BPTreeInternalPage*>(tmpplusone_page->GetData());
		tmpplusone->SetParentIndex(i);
		buffer_pool->UnPin(tmpplusone_page->GetPageId());


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
				left->SetPtrAt(j, l_ch->GetPageId());
				Page* left_page_1 = buffer_pool->GetPage(left->ValueAt(j));
				BPTreePage* left_tree_page_1 = reinterpret_cast<BPTreeInternalPage*>(left_page_1->GetData());
				left_page_1->SetDirty();
				left_tree_page_1->SetParent(left->GetPageId());
				left_tree_page_1->SetParentIndex(j - 1);
				buffer_pool->UnPin(left_page_1->GetPageId());

				left->SetPtrAt(j + 1, r_ch->GetPageId());
				Page* left_page_2 = buffer_pool->GetPage(left->ValueAt(j + 1));
				BPTreePage* left_tree_page_2 = reinterpret_cast<BPTreeInternalPage*>(left_page_2->GetData());
				left_page_2->SetDirty();
				left_tree_page_2->SetParent(left->GetPageId());
				left_tree_page_2->SetParentIndex(j);
				buffer_pool->UnPin(left_page_2->GetPageId());

				j++;
			} else {
				left->SetPtrAt(j, ValueAt(i));
				Page* left_page_1 = buffer_pool->GetPage(left->ValueAt(j));
				BPTreePage* left_tree_page_1 = reinterpret_cast<BPTreeInternalPage*>(left_page_1->GetData());
				left_page_1->SetDirty();
				left_tree_page_1->SetParent(left->GetPageId());
				left_tree_page_1->SetParentIndex(j - 1);
				buffer_pool->UnPin(left_page_1->GetPageId());
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
			left->SetPtrAt(insert, l_ch->GetPageId());
			Page* left_page_1 = buffer_pool->GetPage(left->ValueAt(insert));
			BPTreePage* left_tree_page_1 = reinterpret_cast<BPTreeInternalPage*>(left_page_1->GetData());
			left_page_1->SetDirty();
			left_tree_page_1->SetParent(left->GetPageId());
			left_tree_page_1->SetParentIndex(j - 1);
			buffer_pool->UnPin(left_page_1->GetPageId());

			SetPtrAt(0, r_ch->GetPageId());
			split_key = key;
		} else {
			SetPtrAt(0, ValueAt(split - 1));
			split_key = KeyAt(split - 2);
		}
		Page* left_page_1 = buffer_pool->GetPage(ValueAt(0));
		BPTreePage* left_tree_page_1 = reinterpret_cast<BPTreeInternalPage*>(left_page_1->GetData());
		left_page_1->SetDirty();
		left_tree_page_1->SetParent(GetPageId());
		left_tree_page_1->SetParentIndex(-1);
		buffer_pool->UnPin(left_page_1->GetPageId());

		/* left shift for right node from split - 1 to children - 1 */
		for (i = split - 1, j = 0; i < order - 1; i++, j++) {
			SetKeyAt(j, KeyAt(i));
			SetPtrAt(j + 1, ValueAt(i + 1));
			Page* left_page_1 = buffer_pool->GetPage(ValueAt(j + 1));
			BPTreePage* left_tree_page_1 = reinterpret_cast<BPTreeInternalPage*>(left_page_1->GetData());
			left_page_1->SetDirty();
			left_tree_page_1->SetParent(GetPageId());
			left_tree_page_1->SetParentIndex(j);
			buffer_pool->UnPin(left_page_1->GetPageId());
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
		right->SetPtrAt(0, l_ch->GetPageId());

		Page* rzero_page = buffer_pool->GetPage(right->ValueAt(0));
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rzero = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rzero_page->GetData());
		rzero_page->SetDirty();
		rzero->SetParent(right->GetPageId());
		rzero->SetParentIndex(-1);
		buffer_pool->UnPin(rzero_page->GetPageId());

		right->SetPtrAt(1, r_ch->GetPageId());
		Page* rfirst_page = buffer_pool->GetPage(right->ValueAt(1));
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rfirst = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rfirst_page->GetData());
		rfirst_page->SetDirty();
		rfirst->SetParent(right->GetPageId());
		rfirst->SetParentIndex(0);
		buffer_pool->UnPin(rfirst_page->GetPageId());

		/* insertion point is split point, replicate from key[split] */
		for (i = split, j = 1; i < order - 1; i++, j++) {
			right->SetKeyAt(j,KeyAt(i));
			right->SetPtrAt(j + 1, ValueAt(i + 1));
			
			Page* rplusone_page = buffer_pool->GetPage(right->ValueAt(j + 1));
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rplusone = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rplusone_page->GetData());
			rplusone_page->SetDirty();
			rplusone->SetParent(right->GetPageId());
			rplusone->SetParentIndex(j);
			buffer_pool->UnPin(rplusone_page->GetPageId());
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

		Page* rzero_page = buffer_pool->GetPage(right->ValueAt(0));
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rzero = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rzero_page->GetData());
		rzero_page->SetDirty();
		rzero->SetParent(right->GetPageId());
		rzero->SetParentIndex(-1);
		buffer_pool->UnPin(rzero_page->GetPageId());

		/* replicate from key[split + 1] to key[order - 1] */
		for (i = split + 1, j = 0; i < order - 1; j++) {
			if (j != insert - split - 1) {
				right->SetKeyAt(j, KeyAt(i));
				right->SetPtrAt(j + 1, ValueAt(i + 1));

				Page* rplusone_page = buffer_pool->GetPage(right->ValueAt(j + 1));
				BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rplusone = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rplusone_page->GetData());
				rplusone_page->SetDirty();
				rplusone->SetParent(right->GetPageId());
				rplusone->SetParentIndex(j);
				buffer_pool->UnPin(rplusone_page->GetPageId());

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
		right->SetPtrAt(j, l_ch->GetPageId());

		Page* rj_page = buffer_pool->GetPage(right->ValueAt(j));
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rj = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rj_page->GetData());
		rj_page->SetDirty();
		rj->SetParent(right->GetPageId());
		rj->SetParentIndex(j - 1);
		buffer_pool->UnPin(rj_page->GetPageId());

		right->SetPtrAt(j + 1, r_ch->GetPageId());

		Page* rplusone_page = buffer_pool->GetPage(right->ValueAt(j + 1));
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rplusone = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rplusone_page->GetData());
		rplusone_page->SetDirty();
		rplusone->SetParent(right->GetPageId());
		rplusone->SetParentIndex(j);
		buffer_pool->UnPin(rplusone_page->GetPageId());
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
			Page* cur_page = buffer_pool->GetPage(ValueAt(i));
			BPTreePage* cur = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(cur_page->GetData());
			cur_page->SetDirty();
			cur->SetParentIndex(i - 1);
			buffer_pool->UnPin(cur_page->GetPageId());
		}
		/* parent key right rotation */
		Page* parent_page = buffer_pool->GetPage(GetParent());
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* parent = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(parent_page->GetData());
		parent_page->SetDirty();
		SetKeyAt(0, parent->KeyAt(parent_key_index));
		parent->SetKeyAt(parent_key_index, left->KeyAt(left->GetChildren() - 2));

		/* borrow the last sub-node from left sibling */
		SetPtrAt(0, left->ValueAt(left->GetChildren() - 1));

		Page* rzero_page = buffer_pool->GetPage(ValueAt(0));
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* rzero = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(rzero_page->GetData());
		rzero_page->SetDirty();
		rzero->SetParent(GetPageId());
		rzero->SetParentIndex(-1);
		left->SetChildren(left->GetChildren() - 1);
	}

//todo
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
					Page* ri_page = buffer_pool->GetPage(left->ValueAt(i));
					BPTreeInternalPage<KeyType, ValueType, KeyComparator>* ri = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(ri_page->GetData());
					ri_page->SetDirty();
					ri->SetParent(left->GetPageId());
					ri->SetParentIndex(i - 1);
					buffer_pool->UnPin(ri_page->GetPageId());
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

		Page* children_page = buffer_pool->GetPage(ValueAt(GetChildren()));
		BPTreeInternalPage<KeyType, ValueType, KeyComparator>* children = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(children_page->GetData());
		children_page->SetDirty();
		children->SetParent(GetPageId());
		children->SetParentIndex(GetChildren() - 1);
		buffer_pool->UnPin(children_page->GetPageId());
		SetChildren(GetChildren() + 1);

		/* left shift in right sibling */
		for (i = 0; i < right->GetChildren() - 2; i++) {
			right->SetKeyAt(i, right->KeyAt(i + 1));
		}
		for (i = 0; i < right->GetChildren() - 1; i++) {
			right->SetPtrAt(i, right->ValueAt(i + 1));
			Page* tmp_page = buffer_pool->GetPage(right->ValueAt(i));
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* tmp = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp_page->GetData());
			tmp_page->SetDirty();
			tmp->SetParentIndex(i - 1);
			buffer_pool->UnPin(tmp_page->GetPageId());
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
			Page* tmp_page = buffer_pool->GetPage(ValueAt(i));
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* tmp = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp_page->GetData());
			tmp_page->SetDirty();
			tmp->SetParent(GetPageId());
			tmp->SetParentIndex(i - 1);
			buffer_pool->UnPin(tmp_page->GetPageId());
		}
		SetChildren(i);
		/* delete empty right sibling */
		non_leaf_delete(right, buffer_pool);
	}

	template<typename KeyType, typename ValueType, typename KeyComparator>
	void BPTreeInternalPage<KeyType, ValueType, KeyComparator>::non_leaf_simple_remove(int remove, BufferPool* buffer_pool)
	{
		for (; remove < GetChildren() - 2; remove++) {
			SetKeyAt(remove, KeyAt(remove + 1));
			SetPtrAt(remove + 1, ValueAt(remove + 2));
			Page* tmp_page = buffer_pool->GetPage(ValueAt(remove + 1));
			BPTreeInternalPage<KeyType, ValueType, KeyComparator>* tmp = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp_page->GetData());
			tmp_page->SetDirty();
			tmp->SetParentIndex(remove);
			buffer_pool->UnPin(tmp_page->GetPageId());
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
					non_leaf_simple_remove(remove, buffer_pool);
					if (r_sib->GetChildren() > (tree->GetOrder() + 1) / 2) {
						non_leaf_shift_from_right(r_sib, i + 1, buffer_pool);
					} else {
						non_leaf_merge_from_right(r_sib, i + 1, buffer_pool);
						/* trace upwards */
						parent->non_leaf_remove(tree, i + 1, buffer_pool);
					}
				}
			} else {
				// TODO: Get rid of root as a pointer.
				if (GetChildren() == 2) {
					/* delete old root node */
					Page* newroot_page = buffer_pool->GetPage(ValueAt(0));
					BPTreeInternalPage<KeyType, ValueType, KeyComparator> *newroot = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(newroot_page->GetData());
					newroot_page->SetDirty();
					newroot->SetParent(-1);
					tree->SetRoot(newroot_page->GetPageId());
					buffer_pool->UnPin(newroot_page->GetPageId());
					non_leaf_delete(this, buffer_pool);
					tree->SetLevel(tree->GetLevel() - 1);
				} else {
					non_leaf_simple_remove(remove, buffer_pool);
				}
			}
		} else {
			non_leaf_simple_remove(remove, buffer_pool);
		}
	}

	template class BPTreeInternalPage<int, RID, IntComparator>;
} // namespace udb