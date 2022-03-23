#include "include/storage/page/b_tree_page.h"

namespace udb
{
    bool BTreePage::isLeafPage() const{
        return page_type_ == IndexPageType::LEAF_PAGE;
    }
    bool BTreePage::isInternalPage() const{
        return page_type_ == IndexPageType::INTERNAL_PAGE;
    }
    void BTreePage::SetPageType(IndexPageType type){
        page_type_ = type;
    }

    int BTreePage::GetSize() const{
        return size_;
    }
    void BTreePage::SetSize(int size){
        size_ = size;
    }

    page_id_t BTreePage::GetPageId() const{
        return page_id_;
    }
    page_id_t BTreePage::GetParentPageId() const{
        return parent_page_id_;
    }
    void BTreePage::SetPageId(page_id_t page_id){
        page_id_ = page_id;
    }
    void BTreePage::SetParentPageId(page_id_t parent_page_id){
        parent_page_id_ = parent_page_id;
    }
} // namespace udb

