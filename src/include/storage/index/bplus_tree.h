#pragma once
#include "include/storage/page/bplus_tree_page.h"
#include "include/storage/page/bplus_tree_leaf_page.h"
#include "include/storage/page/bplus_tree_internal_page.h"
#include "stdio.h"

namespace udb
{
class BPlusTree {
public:
	explicit BPlusTree(int order, int entries){
		int i;
		SetRoot(nullptr);
		SetOrder(order);
		SetEntry(entries);
	}
	~BPlusTree() = default;

	// Getter and Setter
	int GetOrder() const { return order_; }
	int GetEntry() const { return entries_; }
	int GetLevel() const { return level_; }
	BPlusTreePage* GetRoot(){ return root_; }

	void SetOrder(int order){ order_ = order;}
	void SetEntry(int entries){ entries_ = entries;}
	void SetLevel(int level){ level_ = level; }
	void SetRoot(BPlusTreePage* root){ root_ = root;}

		// Private APIs ========================================================================

	int key_binary_search(int *arr, int len, int target){
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


		// Public APIs ==================================================================

	int insert(int key, int data){
		BPlusTreePage *node = GetRoot();
		while (node != nullptr) {
			if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
					BPTreeLeafPage *ln = reinterpret_cast<BPTreeLeafPage *>(node);
					return ln->leaf_insert(this, key, data);
			} else {
					BPTreeInternalPage *nln = reinterpret_cast<BPTreeInternalPage *>(node);
					int i = key_binary_search(nln->GetKeys(), nln->GetChildren() - 1, key);
					if (i >= 0) {
							node = nln->ValueAt(i + 1);
					} else {
							i = -i - 1;
							node = nln->ValueAt(i);
					}
			}
		}

		/* new root */
		BPTreeLeafPage *root = new BPTreeLeafPage();
		root->SetKeyAt(0, key);
		root->SetDataAt(0, data);
		root->SetEntry(1);
		SetRoot(reinterpret_cast<BPlusTreePage *>(root));
		return 0;
	}

	int remove(int key){
		BPlusTreePage *node = GetRoot();
		while (node != nullptr) {
			if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
				BPTreeLeafPage *ln = (BPTreeLeafPage *)node;
				return ln->leaf_remove(this, key);
			} else {
				BPTreeInternalPage *nln = (BPTreeInternalPage *)node;
				int i = key_binary_search(nln->GetKeys(), nln->GetChildren() - 1, key);
				if (i >= 0) {
					node = nln->ValueAt(i + 1);
				} else {
					i = -i - 1;
					node = nln->ValueAt(i);
				}
			}
		}
		return -1;
	}

private:
	int order_;
	int entries_;
	int level_;
	BPlusTreePage *root_;
	};
} // namespace udb


