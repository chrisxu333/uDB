#include "include/storage/disk_manager.h"
#include "include/storage/page/table_page.h"
#include <gtest/gtest.h>

namespace udb{
  void fillPage(char* page_data){
    char rchar = 97;
    for(int i = 0; i < PAGE_SIZE-1; ++i){
      page_data[i] = rchar;
      rchar++;
      if(rchar == 123) rchar = 97;
    }
    page_data[PAGE_SIZE - 1] = '\0';
  }

  TEST(test_disk_manager, test_read_write_page){
    DiskManager diskManager(".file.udb");
    char* content = new char[PAGE_SIZE];
    char* res = new char[PAGE_SIZE];

    fillPage(content);

    diskManager.storePage(0, content);
    diskManager.loadPage(0,res);

    EXPECT_STREQ(content,res);

    delete[] content;
    delete[] res;

    remove(".file.udb");
  }

  TEST(test_disk_manager, test_get_file_size){
    DiskManager diskManager(".file.udb");
    char* content = new char[PAGE_SIZE];
    fillPage(content);
    
    diskManager.storePage(0, content);
    EXPECT_EQ(PAGE_SIZE,diskManager.GetFileSize());

    delete[] content;
    remove(".file.udb");
  }

//=====================---TablePage Test---=======================//
  TEST(test_table_page, test_init_table_page){
    TablePage p;
    page_id_t page_id = 1;
    p.Init(1, PAGE_SIZE);

    ASSERT_EQ(p.GetPrevPageId(), INVALID_PAGE_ID);
    ASSERT_EQ(p.GetNextPageId(), INVALID_PAGE_ID);

    char *data = p.GetData();
    ASSERT_EQ(*reinterpret_cast<page_id_t*>(data), page_id);
  }
  
  TEST(test_table_page, test_init){
    TablePage p;
    page_id_t page_id = 1;
    p.Init(1, PAGE_SIZE);

    ASSERT_EQ(p.GetPrevPageId(), INVALID_PAGE_ID);
    ASSERT_EQ(p.GetNextPageId(), INVALID_PAGE_ID);

    char *data = p.GetData();
    ASSERT_EQ(*reinterpret_cast<page_id_t*>(data), page_id);
  }

  TEST(test_table_page, test_insert_get_tuple){
    TablePage p;
    page_id_t page_id = 1;
    p.Init(1, PAGE_SIZE);

    char data[] = {'t','e','s','t','\0'};
    Tuple t(data, 5);

    bool insert_status = p.InsertTuple(t);

    Tuple tuple;
    p.GetTuple(0, &tuple);

    ASSERT_TRUE(insert_status);
    ASSERT_STREQ(tuple.GetTupleData(), data);
  }

  TEST(test_table_page, test_update_tuple){
    TablePage p;
    page_id_t page_id = 1;
    p.Init(1, PAGE_SIZE);

    char data[] = {'t','e','s','t','\0'};
    char ndata[] = {'n','e','w','t','e','s','t','\0'};
    Tuple tuple(data, 5);
    Tuple ntuple(ndata, 8);

    bool insert_status = p.InsertTuple(tuple);
    bool update_status = p.UpdateTuple(0, ntuple);


    Tuple rtuple;
    bool get_status = p.GetTuple(0, &rtuple);

    ASSERT_TRUE(insert_status);
    ASSERT_TRUE(update_status);
    ASSERT_TRUE(get_status);
    ASSERT_STREQ(rtuple.GetTupleData(), ndata);

  }
}
