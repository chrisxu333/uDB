#ifndef UDB_RWLATCH_H
#define UDB_RWLATCH_H

#include <mutex>
#include <condition_variable>

namespace udb
{
    /**
     * Semaphore class for implementing RWlatch.
     * 
     * */
    class Semaphore{
        public:
            Semaphore(int count = 0) : count_(count){}

            void notify(){
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cond_.notify_one();
                    // release the resource.
                    count_++;
                }
            }
            void wait(){
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cond_.wait(lock, [this](){ return count_ > 0; });
                    // acquire one more count_
                    count_--;
                }
            }
        private:
            std::mutex mutex_;
            std::condition_variable cond_;
            int count_;
    };

    class RWlatch{
        public:
            RWlatch(){
                reader_count_ = 0;
                reader_lock_ = new Semaphore(1);
                writer_lock_ = new Semaphore(1);
            }
            ~RWlatch(){
                delete reader_lock_;
                delete writer_lock_;
            }

            void acquireRead(){
                reader_lock_->wait();
                reader_count_++;
                // first reader also has to get writer lock.
                if(reader_count_ == 1) writer_lock_->wait();
                reader_lock_->notify();
            }

            void releaseRead(){
                reader_lock_->wait();
                reader_count_--;
                if(reader_count_ == 0) writer_lock_->notify();
                reader_lock_->notify();
            }

            void acquireWrite(){
                writer_lock_->wait();
            }

            void releaseWrite(){
                writer_lock_->notify();
            }
        private:
            int reader_count_;
            Semaphore* reader_lock_;
            Semaphore* writer_lock_;
    };
} // namespace udb
#endif