#ifndef UDB_DISK_MANAGER_H
#define UDB_DISK_MANAGER_H
#include <string>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sys/stat.h>


#include "../common/type.h"


namespace udb{
    /**
     * DiskManager takes care of the allocation and deallocation of pages within a database. It performs the reading and
     * writing of pages to and from disk, providing a logical file layer within the context of a database management system.
     */
    class DiskManager{
        public:
            /**
             * Creates a disk manager that communicate with the specified database file.
             * @param db_file specified for the name of the database file.
             */
            explicit DiskManager(const std::string&);
            ~DiskManager() = default;
            void shutDown();
            void loadPage(page_id_t, char*);
            void storePage(page_id_t, char*);
            inline int GetFileSize();
        private:
            std::fstream db_file_io_;
            std::string db_file_name_;
    };
}//namespace udb

#endif  //UDB_DISK_MANAGER_H