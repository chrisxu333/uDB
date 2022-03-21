#ifndef UDB_B_TREE_H
#define UDB_B_TREE_H
#include "include/common/type.h"
#include "include/storage/buffer_pool/buffer_pool.h"
#include "include/storage/page/b_tree_internal_page.h"
#include "include/storage/page/b_tree_leaf_page.h"
#include <queue>

namespace udb
{
    template<typename KeyType, typename ValueType, typename KeyComparator>
    class BTree{
        using InternalPage = BTreeInternalPage<KeyType, page_id_t, KeyComparator>;
        using LeafPage = BTreeLeafPage<KeyType, ValueType, KeyComparator>;
        public:
            /**
             * On construct, a BTree object should fetch a new page to be the root of the tree.
             * */
            explicit BTree(BufferPool *buffer_pool, KeyComparator comparator, size_t max_size = -1);
            ~BTree() = default;

            /**
             * TODO: Build index on given column in given table.
             * 
             * */
            void build();

            /**
             * 
             * 
             * */
            BTreeLeafPage<KeyType, ValueType, KeyComparator>* findLeafPage(const KeyType& key);

            /**
             * @param key
             * @param value
             * Insert a <T, RID> pair into the tree.
             * */
            void insert(const KeyType& key, const ValueType& value);

            void vis();

        private:
            /**
             * 
             * 
             * 
             * */
            void UpdateRoot();
            KeyComparator comparator_;
            page_id_t root_page_id_;
            BufferPool *buffer_pool_;
    };
} // namespace udb

#endif