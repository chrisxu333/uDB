#include "include/disk_manager.h"

udb::DiskManager::DiskManager(const std::string& db_file){
    // store the database file name.
    db_file_name_ = db_file;
    // open the main database file.
    db_file_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    // If no such file exists, create one and reopen.
    if(!db_file_io_.is_open()){
        // First create the specified database file with mode binary, trunc and out.
        db_file_io_.open(db_file_name_, std::ios::binary | std::ios::trunc | std::ios::out);
        db_file_io_.close();
        db_file_io_.open(db_file_name_, std::ios::binary | std::ios::in | std::ios::out);
        if (!db_file_io_.is_open()) {
            throw std::runtime_error("can't open db file");
        }
    }
}

void udb::DiskManager::shutDown(){
    db_file_io_.close();
}

void udb::DiskManager::storePage(page_id_t page_id, char* page_data){
    int offset = page_id * PAGE_SIZE;
    // validate the offset 
    if(offset > GetFileSize()){
        throw std::out_of_range("Page doesn't exist in database file.");
    }
    db_file_io_.seekp(offset);
    db_file_io_.write(page_data, PAGE_SIZE);
    if(db_file_io_.bad()){
        throw std::runtime_error("Bad I/O.");
    }
    // IMPORTANT
    db_file_io_.flush();
}

void udb::DiskManager::loadPage(page_id_t page_id, char* page_data){
    // calculate the offset, i.e. starting point of reading the page
    int offset = page_id * PAGE_SIZE;
    // validate the offset 
    if(offset > GetFileSize()){
        throw std::out_of_range("Page doesn't exist in database file.");
    }
    db_file_io_.seekp(offset);
    // read content starting at offset of size PAGE_SIZE
    db_file_io_.read(page_data, PAGE_SIZE);
    if(db_file_io_.bad()){
        throw std::runtime_error("Bad I/O.");
    }
    // count the number of bytes read
    int rcount = db_file_io_.gcount();
    if(rcount < PAGE_SIZE){
        // the page is not full, fill the rest with 0
        db_file_io_.clear();
        memset(page_data + rcount, 0, PAGE_SIZE - rcount);
    }
}

int udb::DiskManager::GetFileSize() {
  struct stat stat_buf;
  int rc = stat(db_file_name_.c_str(), &stat_buf);
  return rc == 0 ? static_cast<int>(stat_buf.st_size) : -1;
}