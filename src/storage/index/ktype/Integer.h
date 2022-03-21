#include "include/storage/index/ktype/Integer.h"

namespace udb
{
    Integer::Integer(int val) : val_(val){}
    int Integer::GetVal() const { return val_; }

    std::ostream& Integer::operator<<(std::ostream& os, const Integer& data){
        os << data.GetInt();
        return os;
    }
} // namespace udb
