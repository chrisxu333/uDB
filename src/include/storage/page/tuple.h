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
                data_ = new char[size];
                memset(data_, 0, size);
                memcpy(data_, data, size);
            }
            ~Tuple(){
                if(data_ != nullptr) delete[] data_;
            }
            char* GetTupleData(){
                return data_;
            }
        private:
            int size_;
            char* data_{nullptr};
    };
} // namespace udb
#endif //UDB_TUPLE_H