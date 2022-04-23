#ifndef UDB_SYSUTIL_UMEMORY_H
#define UDB_SYSUTIL_UMEMORY_H
#include <cstring>
namespace udb
{
    /**
     *  This system util class rewrites some of the memory manipulation methods 
     *  provided by the standard library or the operating system to achieve better 
     *  performance.
     * 
     *  umemcpy()
     *  umemmove()
     *  umemset()
     * 
     * */
    class UMemory{
        public:
            void umemcpy(char* dest, char* src, size_t size);
            void umemmove();
            void umemset();
    };
} // namespace udb
#endif