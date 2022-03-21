#ifndef UDB_INT_COMPARATOR_H
#define UDB_INT_COMPARATOR_H
namespace udb
{
    class IntComparator{
        public:
            int operator()(const int& lhs, const int& rhs) const{
                if(lhs == rhs) return 0;
                if(lhs < rhs) return -1;
                else return 1;
            }
            static bool cmp(const int& lhs, const int& rhs){
                return lhs < rhs;
            }
    };
} // namespace udb

#endif