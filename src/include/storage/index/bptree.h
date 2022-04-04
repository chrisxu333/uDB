#pragma once
#include "include/storage/page/bptree_page.h"
#include "include/storage/page/bptree_leaf_page.h"
#include "include/storage/page/bptree_internal_page.h"
#include "include/common/type.h"
#include "include/storage/buffer_pool/buffer_pool.h"
#include "stdio.h"

#define MAX_ORDER 64

namespace udb
{
	class BPTree {
	public:
		explicit BPTree(int order, int entries, BufferPool* buffer_pool){
			SetRoot(nullptr);
			SetOrder(order);
			SetEntry(entries);
			buffer_pool_ = buffer_pool;
		}
		~BPTree() = default;

		// Getter and Setter
		int GetOrder() const { return order_; }
		int GetEntry() const { return entries_; }
		int GetLevel() const { return level_; }
		BPTreePage* GetRoot(){ return root_; }

		void SetOrder(int order){ order_ = order;}
		void SetEntry(int entries){ entries_ = entries;}
		void SetLevel(int level){ level_ = level; }
		void SetRoot(BPTreePage* root){ root_ = root;}

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

		bool isEmpty(){
			return root_ == nullptr;
		}

		int insert(int key, int data){
			BPTreePage *node = GetRoot();
			while (node != nullptr) {
				if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
					BPTreeLeafPage *ln = reinterpret_cast<BPTreeLeafPage *>(node);
					return ln->leaf_insert(this, key, data, buffer_pool_);
				} else {
					BPTreeInternalPage *nln = reinterpret_cast<BPTreeInternalPage *>(node);
					int i = nln->key_binary_search(nln->GetChildren() - 1, key);
					if (i >= 0) {
						node = nln->ValueAt(i + 1);
					} else {
						i = -i - 1;
						node = nln->ValueAt(i);
					}
				}
			}

			/* new root */
			Page* root_page = buffer_pool_->NewPage();
			BPTreeLeafPage* root = reinterpret_cast<BPTreeLeafPage*>(root_page->GetData());
			root->Init(root_page->GetPageId());
			
			root->SetKeyAt(0, key);
			root->SetDataAt(0, data);
			root->SetEntry(1);
			SetRoot(reinterpret_cast<BPTreePage *>(root));
			return 0;
		}

		int remove(int key){
			BPTreePage *node = GetRoot();
			while (node != nullptr) {
				if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
					BPTreeLeafPage *ln = reinterpret_cast<BPTreeLeafPage*>(node);
					return ln->leaf_remove(this, key, buffer_pool_);
				} else {
					BPTreeInternalPage *nln = reinterpret_cast<BPTreeInternalPage*>(node);
					int i = nln->key_binary_search(nln->GetChildren() - 1, key);
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
			BPTreePage *root_;
			BufferPool* buffer_pool_;
	};
} // namespace udb