#ifndef UDB_TABLE_PAGE_H
#define UDB_TABLE_PAGE_H
#include "page.h"
#include "tuple.h"
#include "../../common/type.h"
#include "../../common/debug_logger.h"
#include <cstring>

namespace udb
{
    /**
     * Structure for a page.
     * ==========================
     * |--------Header----------|
     * |--------Free Space------|
     * |----Inserted Tuples-----|
     * ==========================
     * 
     * Structure for the header.
     * ==========================
     * |--------PageId (4)------|
     * |--------LSN (4)---------|
     * |------PrevPageId (4)----|
     * |------NextPageId (4)----|
     * |--FreeSpacePointer (4)--|
     * |------TupleCount (4)----|
     * |----Tuple_1 offset(4)---|
     * |----Tuple_1 size(4)-----|
     * |----REPEAT PATTERN------|
     * |------------------------|
     * ==========================
     * 
     * */

    class TablePage : public Page{
        public:
            /**
             * 
             * Initialize page header.
             * @param page_id
             *      page id of this table page.
             * @param page_size
             *      page size of this table page.
             * @param prev_page_id
             *      page id of previous table page.
             * 
             * */
            void Init(page_id_t page_id, uint32_t page_size, page_id_t prev_page_id=INVALID_PAGE_ID);

            //===================================Tuple Manipulation=====================================
            /**
             * 
             * Get designated tuple.
             * 
             * */
            bool GetTuple(const uint32_t slot_num, Tuple* tuple);

            /**
             * 
             * Insert new tuple.
             * @param tuple
             *      tuple data to be inserted.
             * 
             * */
            bool InsertTuple(const Tuple &tuple);
            
            /**
             * 
             * Insert new tuple.
             * 
             * */
            bool UpdateTuple(const uint32_t slot_num, const Tuple &ntuple);

            /**
             * 
             * Mask tuple so that it will be deleted and freed later.
             * 
             * */
            bool MarkTupleDelete();

            //===================================== Page Id Manipulation =================================

            /** @return the page ID of this table page */
            page_id_t GetTablePageId() { return *reinterpret_cast<page_id_t *>(GetData()); }

            /** @return the page ID of the previous table page */
            page_id_t GetPrevPageId() { return *reinterpret_cast<page_id_t *>(GetData() + OFFSET_PREV_PAGE_ID); }

            /** @return the page ID of the next table page */
            page_id_t GetNextPageId() { return *reinterpret_cast<page_id_t *>(GetData() + OFFSET_NEXT_PAGE_ID); }
            
            /** Set the page id of the previous page in the table. */
            void SetPrevPageId(page_id_t prev_page_id) {
                memcpy(GetData() + OFFSET_PREV_PAGE_ID, &prev_page_id, sizeof(page_id_t));
            }

            /** Set the page id of the next page in the table. */
            void SetNextPageId(page_id_t next_page_id) {
                memcpy(GetData() + OFFSET_NEXT_PAGE_ID, &next_page_id, sizeof(page_id_t));
            }
        private:
            // helpful size and offset
            static constexpr std::size_t SIZE_PAGE_HEADER = 24;
            static constexpr std::size_t SIZE_TUPLE_HEADER = 8;
            static constexpr std::size_t OFFSET_PAGE_ID = 0;
            static constexpr std::size_t OFFSET_LSN = 4;
            static constexpr std::size_t OFFSET_PREV_PAGE_ID = 8;
            static constexpr std::size_t OFFSET_NEXT_PAGE_ID = 12;
            static constexpr std::size_t OFFSET_FREE_SPACE_POINTER = 16;
            static constexpr std::size_t OFFSET_TUPLE_COUNT = 20;
            static constexpr std::size_t OFFSET_TUPLE_HEADER = 24;
            static constexpr std::size_t OFFSET_TUPLE_SIZE = 28;

            /** @return number of tuple. **/
            uint32_t GetTupleCount(){
                return *reinterpret_cast<uint32_t*>(GetData() + OFFSET_TUPLE_COUNT);
            }

            void SetTupleCount(uint32_t tuple_count){
                memcpy(GetData() + OFFSET_TUPLE_COUNT, &tuple_count, sizeof(uint32_t));
            }

            /** @return free space pointer. **/
            uint32_t GetFreeSpacePointer(){
                return *reinterpret_cast<uint32_t*>(GetData() + OFFSET_FREE_SPACE_POINTER);
            }
            /** Set the free space pointer to the new address pointed by @param.
             *  @param free_space_pointer
             * */
            void SetFreeSpacePointer(uint32_t free_space_pointer){
                memcpy(GetData() + OFFSET_FREE_SPACE_POINTER, & free_space_pointer, sizeof(uint32_t));
            
            }

            uint32_t GetTupleOffSet(uint32_t slot_num){
                return *reinterpret_cast<uint32_t*>(GetData() + OFFSET_TUPLE_HEADER + slot_num * SIZE_TUPLE_HEADER);
            }

            void SetTupleOffset(uint32_t slot_num, uint32_t offset){
                memcpy(GetData() + OFFSET_TUPLE_HEADER + slot_num * SIZE_TUPLE_HEADER, &offset, sizeof(uint32_t));
            }

            uint32_t GetTupleSize(uint32_t slot_num){
                return *reinterpret_cast<uint32_t*>(GetData() + OFFSET_TUPLE_SIZE + slot_num * SIZE_TUPLE_HEADER);
            }
            void SetTupleSize(uint32_t slot_num, uint32_t size){
                memcpy(GetData() + OFFSET_TUPLE_SIZE + slot_num * SIZE_TUPLE_HEADER, &size, sizeof(uint32_t));
            }


            /** @return amount of space left in this page.**/
            uint32_t GetFreeSpaceAmount(){
                return GetFreeSpacePointer() - SIZE_PAGE_HEADER - SIZE_TUPLE_HEADER * GetTupleCount();
            }
    };
} // namespace udb

#endif