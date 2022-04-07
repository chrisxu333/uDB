#include "include/common/rwlatch.h"
#include <thread>
#include <gtest/gtest.h>

#include <algorithm>

namespace udb
{
    RWlatch rwlatch_;
    int testval = 10;
    int log = testval;

    void reader(){
        rwlatch_.acquireRead();
        // std::cout << "reading: " << testval << std::endl;
        ASSERT_EQ(log, testval);
        log = testval;
        rwlatch_.releaseRead();
    }

    void writer(){
        rwlatch_.acquireWrite();
        // std::cout << "add 1: " << ++testval << std::endl;
        log = testval;
        rwlatch_.releaseWrite();
    }

    TEST(test_rwlatch, test_exec){
        srand(time(0));
        size_t size_ = 50;

        void (*funcarr[2])() = {reader, writer};
        std::thread tarr[size_];
        
        for(int i = 0; i < size_; ++i){
            tarr[i] = std::thread(funcarr[rand() % 2]);
        }

        for(int i = 0; i < size_; ++i){
            tarr[i].join();
        }
    }
} // namespace udb
