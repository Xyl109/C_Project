#ifndef QUERY_H
#define QUERY_H

#include "book.h"

/*查询结果：保存所有命中图书节点*/
typedef struct {
    BookNode **nodes;   /*命中节点指针数组*/
    int count;  /*命中数量*/
    int ok; /*1=结果完整, 0=内存不足导致结果不完整*/
} QueryResult;

/*5种条件查询，均基于线性查找遍历链表*/
QueryResult queryByTitle(BookList head, const char *keyword);   /*书名模糊*/
QueryResult queryByAuthor(BookList head, const char *keyword);  /*作者模糊*/
QueryResult queryByPublisher(BookList head, const char *keyword);   /*出版社模糊*/
QueryResult queryByYear(BookList head, int year);   /*年份精确*/

/*库存比较查询：op = 1表示 <=, op=2表示 >=,op = 3 表示==*/
QueryResult queryByStock(BookList head, int op, int value);

/*释放查询结果数组*/
void freeQueryResult(QueryResult *r);

#endif
