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
        if(GetFreeSpaceAmount() < tuple.size_ + SIZE_TUPLE_HEADER) return false;

        // find a slot number to insert
        uint32_t slot_num = GetTupleCount();

        // first increment free space pointer.
        SetFreeSpacePointer(GetFreeSpacePointer() - tuple.size_);
        // insert tuple into page address space starting at FreeSpacePointer
        memcpy(GetData() + GetFreeSpacePointer(), tuple.data_, tuple.size_);

        // set offset of the inserted tuple
        SetTupleOffset(slot_num, GetFreeSpacePointer());
        // set size of the inserted tuple
        SetTupleSize(slot_num, tuple.size_);

        // increment tuple count by 1
        SetTupleCount(GetTupleCount() + 1);

        return true;
    }

    bool TablePage::GetTuple(uint32_t slot_num, Tuple *tuple){
        if(slot_num > GetTupleCount()) return false;
        // first get offset of the tuple
        uint32_t offset = GetTupleOffSet(slot_num);
        uint32_t tuple_size = GetTupleSize(slot_num);
        tuple->size_ = tuple_size;
        if(tuple->data_ == nullptr) tuple->data_ = new char[tuple_size];
        memcpy(tuple->data_, GetData() + offset, tuple_size);
        return true;
    }

    bool UpdateTuple(const Tuple &ntuple, Tuple *tuple){

    }
} // namespace udb


