#ifndef UDB_RID_H
#define UDB_RID_H
#include "include/common/type.h"
namespace udb
{
    class RID{
        public:
            RID() = default;
            RID(page_id_t page_id, slot_id_t slot_id) : page_id_(page_id), slot_id_(slot_id){}
            ~RID() = default;

            page_id_t GetPageId(){ return page_id_; }
            slot_id_t GetSlotId() {return slot_id_; }

            void Set(page_id_t page_id, slot_id_t slot_id){ 
                page_id_ = page_id;
                slot_id_ = slot_id;
            }

            bool operator==(const RID& rhs){
                return (page_id_ == rhs.page_id_) && (slot_id_ == rhs.slot_id_);
            }

        private:
            page_id_t page_id_;
            slot_id_t slot_id_;
    };

    static RID INVALID_RID(INVALID_PAGE_ID, INVALID_SLOT_ID);
    
} // namespace udb
#endif