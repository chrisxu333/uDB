#ifndef UDB_TUPLE_H
#define UDB_TUPLE_H
#include <cstring>

namespace udb
{
    class Tuple{
        friend class TablePage;
        public:
            Tuple() = default;
            Tuple(char* data, int size) : size_(size){
                std::memcpy(data_, data, size);
            }
        private:
            int size_;
            char* data_;
    };
} // namespace udb
#endif //UDB_TUPLE_H