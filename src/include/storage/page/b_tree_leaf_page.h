#ifndef UDB_B_TREE_LEAF_PAGE_H
#define UDB_B_TREE_LEAF_PAGE_H
#include "include/storage/page/b_tree_internal_page.h"
#include "include/common/debug_logger.h"

namespace udb
{
    #define LEAF_PAGE_HEADER_SIZE 28
    #define LEAF_PAGE_SIZE ((PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / (sizeof(std::pair<KeyType, ValueType>)))
    /*
     * B Tree Leaf Page structure.
     * ==========================
     * |--------Header----------|
     * |------KEY(1) + RID(1)---|
     * |------KEY(2) + RID(2)---|
     * |--------......----------|
     * |------KEY(N) + RID(N)---|
     * ==========================
     * */
    template<typename KeyType, typename ValueType, typename KeyComparator>
    class BTreeLeafPage : public BTreePage{
        public:
            BTreeLeafPage() = default;
            ~BTreeLeafPage() = default;

            /**
             * @param page_id
             * @param parent_id
             * @param max_size
             * 
             * Initialize fields inside a b-tree page, including page_id_, parent_page_id_, size_,
             * max_size_, page_type_. 
             * 
             * */
            void Init(page_id_t page_id, page_id_t next_page_id = INVALID_PAGE_ID, page_id_t parent_id = INVALID_PAGE_ID, int max_size = LEAF_PAGE_SIZE);

            //========== Helper Methods ==========//

            /**
             * @param index
             * Find key at given @index in keys_.
             * */
            KeyType KeyAt(int index) const;

            /**
             * @param index
             * @param nkey
             * Set key at given @index in keys_ with @nkey.
             * */
            void SetKeyAt(int index, const KeyType& nkey);

            /**
             * @param index
             * Find value at given @index in keys_.
             * */
            ValueType ValueAt(int index) const;

            //========== LoopUp and Manipulation Methods ==========//
            
            /**
             * @param key
             * @param comparator
             * Get the value of given @key using @comparator.
             * */
            ValueType LookUp(const KeyType& key, const KeyComparator &comparator) const;

            /**
             * @param key
             * @param value
             * Insert key-value pair such that the keys_ array is still in ascending order.
             * */
            void Insert(const KeyType& key, const ValueType& value, BufferPool* buffer_pool, KeyComparator comparator);

            //============== Split and Merge Methods ==============//
            
            void Split(BufferPool* buffer_pool, KeyComparator comparator);
            
        private:
            void copyHalfTo(BTreeLeafPage<KeyType, ValueType, KeyComparator>* page);
            page_id_t next_page_id_;
            std::pair<KeyType, ValueType> keys_[LEAF_PAGE_SIZE];
    };
} // namespace udb
#endif