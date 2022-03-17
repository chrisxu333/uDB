#ifndef UDB_B_TREE_LEAF_PAGE_H
#define UDB_B_TREE_LEAF_PAGE_H
#include "b_tree_page.h"

namespace udb
{
    /*
     * B Tree Leaf Page structure.
     * ==========================
     * |--------Header----------|
     * |--KEY(1) + RID(1)---|
     * |--KEY(2) + RID(2)---|
     * |--------......----------|
     * |--KEY(N) + RID(N)---|
     * ==========================
     * */
    template<typename KeyType, typename ValueType, typename KeyComparator>
    class BTreeLeafPage : public BTreePage{
        public:
    };
    
} // namespace udb
#endif