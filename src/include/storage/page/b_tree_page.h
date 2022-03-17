#ifndef UDB_INDEX_PAGE_H
#define UDB_INDEX_PAGE_H

#include "page.h"

namespace udb
{
    enum class IndexPageType {INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE};
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
            BTreePage() = default;
            ~BTreePage() = default;

            bool isLeafPage() const;
            bool isInternalPage() const;
            void SetPageType(IndexPageType type);

            int GetSize() const;
            void SetSize(int size);

            page_id_t GetPageId() const;
            page_id_t GetParentPageId() const;
            void SetPageId(page_id_t page_id);
            void SetParentPageId(page_id_t parent_page_id);
        protected:
            IndexPageType page_type_;
            int size_;
            int max_size_;
            page_id_t page_id_;
            page_id_t parent_page_id_;
    };
} // namespace udb
#endif