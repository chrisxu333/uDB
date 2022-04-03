#pragma once

#define MAX_ORDER 64

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

	class BPTreeInternalPage;
	class BPlusTree;

	/**
	 * B Plus Tree Header Structure
     * ==========================
     * |------PageType (4)------|
     * |--------LSN (4)---------|
     * |-----CurrentSize (4)----|
     * |---ParentPageIndex (4)--|
     * |-----ParentPtr (4)------|
     * |-------PageId (4)-------|
     * ==========================
	 * */
	class BPlusTreePage {
		public:
			BPlusTreePage() = default;
			~BPlusTreePage() = default;

			// Getter and Setter
			NodeType GetType() const { return type_; }
			int GetParentIndex() const { return parent_key_idx_; }
			BPTreeInternalPage* GetParent(){ return parent_; }
			int GetCount() const { return count_; }
			//page_id_t GetPageId() const { return page_id_; }

			void SetType(NodeType type){ type_ = type; }
			void SetParentIndex(int parent_key_idx){ parent_key_idx_ = parent_key_idx; }
			void SetParent(BPTreeInternalPage* parent){ parent_ = parent; }
			void SetCount(int count){ count_ = count; }
			//void SetPageId(page_id_t page_id){ page_id_ = page_id; }
	private:
			NodeType type_;
			int parent_key_idx_;
			BPTreeInternalPage *parent_;
			// list_head link;
			int count_;
	};
} // namespace udb