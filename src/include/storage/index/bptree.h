#pragma once
#include "include/storage/page/bptree_leaf_page.h"
#include "include/storage/page/bptree_internal_page.h"
#include "include/common/type.h"
#include "include/storage/buffer_pool/buffer_pool.h"
#include "stdio.h"

#define MAX_ORDER 64

namespace udb
{
	template<typename KeyType, typename ValueType, typename KeyComparator>
	class BPTree {
	public:
		explicit BPTree(int order, int entries, BufferPool* buffer_pool){
			SetOrder(order);
			SetEntry(entries);
			buffer_pool_ = buffer_pool;
			SetRoot(INVALID_PAGE_ID);
		}
		~BPTree() = default;

		// Getter and Setter
		int GetOrder() const { return order_; }
		int GetEntry() const { return entries_; }
		int GetLevel() const { return level_; }
		page_id_t GetRoot(){ return root_; }

		void SetOrder(int order){ order_ = order;}
		void SetEntry(int entries){ entries_ = entries;}
		void SetLevel(int level){ level_ = level; }
		void SetRoot(page_id_t root){ root_ = root;}

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
			return root_ == INVALID_PAGE_ID;
		}

		int insert(KeyType key, ValueType data){
			if(root_ != INVALID_PAGE_ID){
				Page* node_page = buffer_pool_->GetPage(GetRoot());
				BPTreePage* node = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(node_page->GetData());
				node_page->SetDirty();
				while (node != nullptr) {
					if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
						BPTreeLeafPage<KeyType, ValueType, KeyComparator> *ln = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator> *>(node);
						return ln->leaf_insert(this, key, data, buffer_pool_);
					} else {
						BPTreeInternalPage<KeyType, ValueType, KeyComparator> *nln = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator> *>(node);
						int i = nln->key_binary_search(nln->GetChildren() - 1, key);
						if (i >= 0) {
							Page* tmp_node = buffer_pool_->GetPage(nln->ValueAt(i + 1));
							node = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp_node->GetData());
						} else {
							i = -i - 1;
							Page* tmp_node = buffer_pool_->GetPage(nln->ValueAt(i));
							node = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp_node->GetData());
						}
					}
				}
			}

			/* new root */
			Page* root_page = buffer_pool_->NewPage();
			BPTreeLeafPage<KeyType, ValueType, KeyComparator>* root = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(root_page->GetData());
			root->Init(root_page->GetPageId());
			
			root->SetKeyAt(0, key);
			root->SetDataAt(0, data);
			root->SetEntry(1);
			SetRoot(root->GetPageId());
			buffer_pool_->UnPin(root_page->GetPageId());
			return 0;
		}

		int remove(KeyType key){
			Page* node_page = buffer_pool_->GetPage(GetRoot());
			BPTreePage* node = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(node_page->GetData());
			node_page->SetDirty();
			while (node != nullptr) {
				if (node->GetType() == NodeType::BPLUS_TREE_LEAF) {
					BPTreeLeafPage<KeyType, ValueType, KeyComparator> *ln = reinterpret_cast<BPTreeLeafPage<KeyType, ValueType, KeyComparator>*>(node);
					return ln->leaf_remove(this, key, buffer_pool_);
				} else {
					BPTreeInternalPage<KeyType, ValueType, KeyComparator> *nln = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(node);
					int i = nln->key_binary_search(nln->GetChildren() - 1, key);
					if (i >= 0) {
							Page* tmp_node = buffer_pool_->GetPage(nln->ValueAt(i + 1));
							node = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp_node->GetData());
					} else {
							i = -i - 1;
							Page* tmp_node = buffer_pool_->GetPage(nln->ValueAt(i));
							node = reinterpret_cast<BPTreeInternalPage<KeyType, ValueType, KeyComparator>*>(tmp_node->GetData());
					}
				}
			}
			return -1;
		}

	private:
			int order_;
			int entries_;
			int level_;
			page_id_t root_;
			BufferPool* buffer_pool_;
	};
} // namespace udb