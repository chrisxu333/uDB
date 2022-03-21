#ifndef UDB_INTEGER_H
#define UDB_INTEGER_H
namespace udb
{
    class Integer{
        public:
            explicit Integer(int val);
            ~Integer() = default;
            int GetInt() const;
            friend std::ostream& operator<<(std::ostream& os, const Integer& data);
        private:
            int val_;
    };
} // namespace udb

#endif
