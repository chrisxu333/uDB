#ifndef UDB_INDEX_PAGE_H
#define UDB_INDEX_PAGE_H

#include "page.h"

namespace udb
{
    class IndexPage : public Page{
        public:
            IndexPage() = default;
            ~IndexPage() = default;
        private:
    };
} // namespace udb
#endif