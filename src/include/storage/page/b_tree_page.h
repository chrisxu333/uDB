#ifndef UDB_INDEX_PAGE_H
#define UDB_INDEX_PAGE_H

#include "include/storage/page/page.h"
#include "include/storage/buffer_pool/buffer_pool.h"
#include "include/storage/index/int_comparator.h"
#include "include/common/rid.h"
#include "include/storage/index/ktype/Integer.h"

#include <algorithm>

namespace udb
{
    enum class IndexPageType {INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE};
    enum class MergeMode {APPEND = 0, INSERT};
    /**
     * 
     * B Tree Page Header structure.
     * ==========================
     * |------PageType (4)------|
     * |--------LSN (4)---------|
     * |-----CurrentSize (4)----|
     * |--------MaxSize (4)-----|
     * |---ParentPageId (4)-----|
     * |-------PageId (4)-------|
     * ==========================
     * 
     * */
    class BTreePage{
        public:
            bool isLeafPage() const;
            bool isInternalPage() const;
            void SetPageType(IndexPageType type);

            int GetSize() const;
            void SetSize(int size);

            page_id_t GetPageId() const;
            page_id_t GetParentPageId() const;
            void SetPageId(page_id_t page_id);
            void SetParentPageId(page_id_t parent_page_id);
        // The pointer would store the data in the following way.
        protected:
            IndexPageType page_type_;
            // TODO: Replace this with lsn_t type
            uint32_t lsn_;
            int size_;
            int max_size_;
            page_id_t parent_page_id_;
            page_id_t page_id_;
    };
} // namespace udb
#endif