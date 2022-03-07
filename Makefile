CFLAGS = -std=c++1z -lstdc++ -Wall -Werror -I./src/parser/ -L../

LIB_DIR := ./lib/
SRC_DIR := ./src/
INCLUDE_DIR := ./src/include/

OBJ_REWRITE_MODULE := q_rewrite

# Library
LIB_SQL_PARSER := -lsqlparser -Wl,-rpath=${LIB_DIR}

all: ${SRC_DIR}main.cpp ${SRC_DIR}ParserInterface.cpp ${INCLUDE_DIR}ParserInterface.h
	$(CXX) $(CFLAGS) ${SRC_DIR}main.cpp ${SRC_DIR}ParserInterface.cpp -o ${OBJ_REWRITE_MODULE} ${LIB_SQL_PARSER}
clean:
	rm ${OBJ_REWRITE_MODULE}