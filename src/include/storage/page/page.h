// Base Page class for various type of pages across the system.

#ifndef UDB_PAGE_H
#define UDB_PAGE_H
#include "include/common/type.h"
#include "include/common/rwlatch.h"
#include <cstring>

namespace udb
{
    // Page is the basic storage unit in uDB.
    // A single page contains information like:
    //      1. pin_count: how many transactions are using this page.
    //      2. dirty_bit: whether this page needs to flush back to disk.
    //      3. page_id: a unique physical page id across the whole system.
    //      4. data: data contained by the page.
    // Page opeartion
    class Page{
        friend class BufferPool;
        public:
            Page() = default;
            ~Page() = default;    // force this class to be abstract so no class could inistantiate it.
            /** @return data_ **/
            inline char* GetData(){
                return data_;
            }
            /** @return page_id_ **/
            inline page_id_t GetPageId(){
                return page_id_;
            }
            /** @return pin_count_ **/
            inline int GetPinCount(){
                return pin_count_;
            }

            inline int SetPinCount(int pin_count){
                pin_count_ = pin_count;
            }

            /** @return dirty_bit_ **/
            inline bool isDirty(){
                return dirty_bit_;
            }
            inline void meminit(){
                memset(data_, 0, PAGE_SIZE);
            }
            inline void SetDirty(){
                dirty_bit_ = true;
            }
        private:
            page_id_t page_id_;
            int pin_count_ = 0;
            bool dirty_bit_ = false;
            char data_[PAGE_SIZE];
            RWlatch rwlatch_;
    };
} // namespace udb

#endif  //UDB_PAGE_H

