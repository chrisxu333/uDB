#include "include/storage/index/b_tree.h"
#include "include/storage/index/int_comparator.h"
#include "include/common/rid.h"
#include "include/storage/index/ktype/Integer.h"
#include "include/storage/disk_manager.h"
#include <gtest/gtest.h>

namespace udb
{
    TEST(test_b_plus_tree, test_b_tree_leaf_page){
        DiskManager* disk_manager = new DiskManager(".file.udb");
        BufferPool* buffer_pool = new BufferPool(5, disk_manager);

        Page* page = buffer_pool->NewPage();
        BTreeLeafPage<int, RID, IntComparator>* btree_page = reinterpret_cast<BTreeLeafPage<int, RID, IntComparator>*>(page->GetData());
        btree_page->Init(page->GetPageId());
        page->SetDirty();

        // Scenario 1: page hasn't get a chance to flush out yet.
        Page* old_page = buffer_pool->GetPage(page->GetPageId());
        btree_page = reinterpret_cast<BTreeLeafPage<int, RID, IntComparator>*>(page->GetData());

        ASSERT_EQ(btree_page->GetPageId(), page->GetPageId());
        ASSERT_EQ(btree_page->GetParentPageId(), INVALID_PAGE_ID);
        ASSERT_TRUE(btree_page->isLeafPage());

        // Scenario 2: page has been flushed out and now read back.
        for(size_t i = 0; i < 10; ++i){
            Page* page = buffer_pool->NewPage();
            btree_page = reinterpret_cast<BTreeLeafPage<int, RID, IntComparator>*>(page->GetData());
            btree_page->Init(page->GetPageId());
            page->SetDirty();
        }

        old_page = buffer_pool->GetPage(page->GetPageId());
        btree_page = reinterpret_cast<BTreeLeafPage<int, RID, IntComparator>*>(page->GetData());

        ASSERT_EQ(btree_page->GetPageId(), page->GetPageId());
        ASSERT_EQ(btree_page->GetParentPageId(), INVALID_PAGE_ID);
        ASSERT_TRUE(btree_page->isLeafPage());


        remove(".file.udb");
    }

    TEST(test_b_plus_tree, test_b_tree_internal_page){
        DiskManager* disk_manager = new DiskManager(".file.udb");
        BufferPool* buffer_pool = new BufferPool(5, disk_manager);

        Page* page = buffer_pool->NewPage();
        BTreeInternalPage<int, page_id_t, IntComparator>* btree_page = reinterpret_cast<BTreeInternalPage<int, page_id_t, IntComparator>*>(page->GetData());
        btree_page->Init(page->GetPageId());
        page->SetDirty();

        // Scenario 1: page hasn't get a chance to flush out yet.
        Page* old_page = buffer_pool->GetPage(page->GetPageId());
        btree_page = reinterpret_cast<BTreeInternalPage<int, page_id_t, IntComparator>*>(page->GetData());

        ASSERT_EQ(btree_page->GetPageId(), page->GetPageId());
        ASSERT_EQ(btree_page->GetParentPageId(), INVALID_PAGE_ID);
        ASSERT_TRUE(btree_page->isInternalPage());

        // Scenario 2: page has been flushed out and now read back.
        for(size_t i = 0; i < 10; ++i){
            Page* page = buffer_pool->NewPage();
            btree_page = reinterpret_cast<BTreeInternalPage<int, page_id_t, IntComparator>*>(page->GetData());
            btree_page->Init(page->GetPageId());
            page->SetDirty();
        }

        old_page = buffer_pool->GetPage(page->GetPageId());
        btree_page = reinterpret_cast<BTreeInternalPage<int, page_id_t, IntComparator>*>(page->GetData());

        ASSERT_EQ(btree_page->GetPageId(), page->GetPageId());
        ASSERT_EQ(btree_page->GetParentPageId(), INVALID_PAGE_ID);
        ASSERT_TRUE(btree_page->isInternalPage());


        remove(".file.udb");
    }
    // TEST(test_b_plus_tree, test_insert_one){
    //     // init phase
    //     DiskManager* disk_manager = new DiskManager(".file.udb");
    //     BufferPool* buffer_pool = new BufferPool(5, disk_manager);
        
    //     // create a b+ tree
    //     BTree<int, RID, IntComparator> btree(buffer_pool);
    //     RID id(0, 0);
    //     btree.insert(1, id);
    //     btree.vis();

    //     remove(".file.udb");
    // }

    TEST(test_b_plus_tree, test_insert_multiple){
        // init phase
        DiskManager* disk_manager = new DiskManager(".file.udb");
        BufferPool* buffer_pool = new BufferPool(50, disk_manager);
        
        IntComparator cmp;
        // create a b+ tree
        BTree<int, RID, IntComparator> btree(buffer_pool, cmp, 7);
        RID id;
        for(size_t i = 0; i < 25; ++i){
            id.Set(0,i);
            btree.insert(i, id);
        }
        id.Set(0, 200);
        btree.insert(10, id);
        btree.vis();
        remove(".file.udb");
    }
} // namespace udb
