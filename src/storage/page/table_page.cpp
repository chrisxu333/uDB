#include "../../include/storage/page/table_page.h"

namespace udb
{
    Page::~Page(){}

    void TablePage::Init(page_id_t page_id, uint32_t page_size, page_id_t prev_page_id){
        memcpy(GetData(), &page_id, sizeof(page_id));
        SetPrevPageId(prev_page_id);
        SetNextPageId(INVALID_PAGE_ID);
        SetFreeSpacePointer(page_size);
        SetTupleCount(0);
    }

    bool TablePage::InsertTuple(const Tuple &tuple){
        // if there's not enough space in this page, return false.
        //if(GetFreeSpaceAmount() < tuple.size_ + SIZE_TUPLE_HEADER) return false;
        // insert tuple into page address space starting at FreeSpacePointer
        //memcpy(GetData() + GetFreeSpacePointer(), tuple.data_, tuple.size_);
    }

    bool TablePage::GetTuple(uint32_t slot_num, Tuple *tuple){
        if(slot_num > GetTupleCount()) return false;
        // first get offset of the tuple
        uint32_t offset = GetTupleOffSet(slot_num);
        uint32_t tuple_size = GetTupleSize(slot_num);
        memcpy(tuple, GetData() + offset, sizeof(tuple_size));
        return true;
    }

    bool UpdateTuple(const Tuple &ntuple, Tuple *tuple){

    }
} // namespace udb


