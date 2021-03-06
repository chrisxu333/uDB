#ifndef UDB_TYPE_H
#define UDB_TYPE_H
#include <cstdint>

namespace udb{
    static constexpr int PAGE_SIZE = 4096;      // size of a data page in byte
    static constexpr int INVALID_PAGE_ID = -1;  // invalid page id
    static constexpr int INVALID_LSN = -1;      // invalid LSN
    static constexpr int INVALID_SLOT_ID = -1;  // invalid slot id
    
    using page_id_t = int32_t;
    using frame_id_t = int32_t;
    using slot_id_t = int32_t;
    using lsn_t = int32_t;         // log sequence number type
};

#endif  //UDB_TYPE_H