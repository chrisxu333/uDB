#include <iostream>
#include <string>

#include "include/storage/index/b_tree.h"
#include "include/storage/index/int_comparator.h"
#include "include/common/rid.h"
#include "include/storage/index/ktype/Integer.h"
#include "include/storage/disk_manager.h"

using namespace udb;

int main(){
    DiskManager* disk_manager = new DiskManager(".file.udb");
    BufferPool* buffer_pool = new BufferPool(50, disk_manager);
    IntComparator cmp;
    // create a b+ tree
    BTree<int, RID, IntComparator> btree(buffer_pool, cmp, 5);
    RID id;
    for(size_t i = 1; i <= 19; ++i){
        id.Set(0,i);
        btree.insert(i, id);
    }
    btree.vis();
    while(true){
        std::string line;
        std::getline(std::cin, line);
        if(line[0] == '+'){
            line = line.substr(1);
            RID rid;
            rid.Set(0, stoi(line));
            btree.insert(stoi(line), rid);
        }else{
            line = line.substr(1);
            btree.remove(stoi(line));
        }
        btree.vis();
    }
    remove(".file.udb");
}