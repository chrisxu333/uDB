#pragma once

#include <utility>
#include <iostream>
#include "include/storage/page/page.h"
#include "include/storage/buffer_pool/buffer_pool.h"
#include "include/storage/index/int_comparator.h"
#include "include/common/rid.h"
#include "include/storage/index/ktype/Integer.h"

namespace udb
{
	enum class NodeType{
        BPLUS_TREE_LEAF,
        BPLUS_TREE_NON_LEAF = 1,
	};

	enum class SiblingType{
			LEFT_SIBLING,
			RIGHT_SIBLING = 1,
	};

	template<typename KeyType, typename ValueType, typename KeyComparator>
	class BPTreeInternalPage;
	template<typename KeyType, typename ValueType, typename KeyComparator>
	class BPTree;

	class BPTreePage {
		public:
			BPTreePage(NodeType type, int parent_key_idx, int parent_page_id): type_(type), parent_key_idx_(parent_key_idx), parent_page_id_(parent_page_id){}
			~BPTreePage() = default;

			// Getter and Setter
			NodeType GetType(){ return type_; }

			int GetParentIndex(){ return parent_key_idx_; }

			int GetParent(){ return parent_page_id_; }

			int GetCount(){ return count_; }

			int GetPageId(){ return page_id_; }

			void SetType(NodeType type){ type_ = type; }

			void SetParentIndex(int parent_key_idx){ parent_key_idx_ = parent_key_idx; }

			void SetParent(int parent_page_id){ parent_page_id_ = parent_page_id; }

			void SetCount(int count){ count_ = count; }

			void SetPageId(int page_id){ page_id_ = page_id; }
		private:
			NodeType type_;                 // 4 bytes type
			int parent_key_idx_;            // 4 bytes parent_key_idx
			int parent_page_id_;     // 4 bytes parent ptr
			int count_;                     // 4 bytes count
			int page_id_;                   // 4 bytes page_id
	};
} // namespace udb