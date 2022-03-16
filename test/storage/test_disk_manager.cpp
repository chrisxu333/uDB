#include "../../src/include/storage/disk_manager.h"
#include "../../src/include/storage/page/table_page.h"
#include <gtest/gtest.h>

namespace udb{
  void fillPage(char* page_data){
    char rchar = 97;
    for(int i = 0; i < PAGE_SIZE-1; ++i){
      page_data[i] = rchar;
      rchar++;
      if(rchar == 123) rchar = 97;
    }
  }

  TEST(test_disk_manager, test_read_write_page){
    udb::DiskManager diskManager(".file.udb");
    char* content = new char[PAGE_SIZE];
    char* res = new char[PAGE_SIZE];

    fillPage(content);

    diskManager.storePage(0, content);
    diskManager.loadPage(0,res);

    diskManager.shutDown();
    delete[] content;
    delete[] res;

    EXPECT_STREQ(content,res);

    remove(".file.udb");
  }

  TEST(test_disk_manager, test_get_file_size){
    udb::DiskManager diskManager(".file.udb");
    char* content = new char[PAGE_SIZE];

    fillPage(content);
    
    diskManager.storePage(0, content);
    
    delete[] content;
    
    EXPECT_EQ(PAGE_SIZE,diskManager.GetFileSize());
    
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
  
  TEST(test_table_page, test_init_table_page){
    TablePage p;
    page_id_t page_id = 1;
    p.Init(1, PAGE_SIZE);

    ASSERT_EQ(p.GetPrevPageId(), INVALID_PAGE_ID);
    ASSERT_EQ(p.GetNextPageId(), INVALID_PAGE_ID);

    char *data = p.GetData();
    ASSERT_EQ(*reinterpret_cast<page_id_t*>(data), page_id);
  }
}
