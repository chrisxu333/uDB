#include "../../include/storage/page/table_page.h"

namespace udb
{
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

    bool TablePage::GetTuple(const uint32_t slot_num, Tuple *tuple){
        if(slot_num > GetTupleCount()) return false;

        // first get offset of the tuple
        uint32_t offset = GetTupleOffSet(slot_num);
        uint32_t tuple_size = GetTupleSize(slot_num);

        tuple->size_ = tuple_size;
        if(tuple->data_ != nullptr) delete[] tuple->data_;
        tuple->data_ = new char[tuple_size];

        memcpy(tuple->data_, GetData() + offset, tuple_size);

        return true;
    }

    bool TablePage::UpdateTuple(const uint32_t slot_num, const Tuple &ntuple){
        // Get offset and size of the old tuple.
        uint32_t offset = GetTupleOffSet(slot_num);
        uint32_t size = GetTupleSize(slot_num);

        // if page doesn't have enough space, return false
        if(GetFreeSpaceAmount() + size - ntuple.size_ < 0) return false;

        // update data.
        memmove(GetData() + GetFreeSpacePointer() - ntuple.size_ + size, GetData() + GetFreeSpacePointer(), offset - GetFreeSpacePointer());
        SetFreeSpacePointer(GetFreeSpacePointer() - ntuple.size_ + size);
        memcpy(GetData() + offset - ntuple.size_ + size, ntuple.data_, ntuple.size_);
        SetTupleOffset(slot_num, offset - ntuple.size_ + size);
        SetTupleSize(slot_num, ntuple.size_);

        return true;
    }
} // namespace udb


