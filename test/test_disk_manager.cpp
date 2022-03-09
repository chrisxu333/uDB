#include "../src/include/disk_manager.h"
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
}
