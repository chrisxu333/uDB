#ifndef UDB_B_TREE_INTERNAL_PAGE_H
#define UDB_B_TREE_INTERNAL_PAGE_H
#include "include/storage/page/b_tree_page.h"
#include "include/common/debug_logger.h"
#include <vector>

namespace udb
{
    #define INTERNAL_PAGE_HEADER_SIZE 24
    #define INTERNAL_PAGE_SIZE ((PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(std::pair<KeyType, ValueType>)))
    /*
     * B Tree Internal Page structure.
     * ==========================
     * |--------Header----------|
     * |--KEY(1) + PAGE_ID(1)---|
     * |--KEY(2) + PAGE_ID(2)---|
     * |--------......----------|
     * |--KEY(N) + PAGE_ID(N)---|
     * ==========================
     * */
    template<typename KeyType, typename ValueType, typename KeyComparator>
    class BTreeInternalPage : public BTreePage{
        public:
            BTreeInternalPage() = default;
            ~BTreeInternalPage() = default;

            /**
             * @param page_id
             * @param parent_id
             * @param max_size
             * 
             * Initialize fields inside a b-tree page, including page_id_, parent_page_id_, size_,
             * max_size_, page_type_. 
             * 
             * */
            void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = INTERNAL_PAGE_SIZE);

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
            void Insert(const KeyType& key, const ValueType& value, BufferPool* buffer_pool);

            //============== Split and Merge Methods ==============//
            
            void Split(BufferPool* buffer_pool);
            
        private:
            void copyHalfTo(BTreeInternalPage<KeyType, ValueType, KeyComparator>* page);

            void updateParentId(BTreeInternalPage<KeyType, ValueType, KeyComparator>* page, BufferPool* buffer_pool);
            
            // A KeyType <-> ValueType pairing mapping container. This allows us to 
            // look up key or value at given index.
            std::pair<KeyType, ValueType> keys_[INTERNAL_PAGE_SIZE];
    };
    
} // namespace udb
#endif