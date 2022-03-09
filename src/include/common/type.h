#ifndef UDB_TYPE_H
#define UDB_TYPE_H
#include <cstdint>

namespace udb{
    static constexpr int PAGE_SIZE = 4096;      // size of a data page in byte
    
    using page_id_t = int32_t;
};

#endif  //UDB_TYPE_H