#ifndef BOOK_H
#define BOOK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*图书信息结构体*/
typedef struct {
    char id[20];    /*图书编号*/
    char name[50];  /*书名*/
    char author[30];    /*作者*/
    char publisher[50]; /*出版社*/
    int year;   /*出版年份*/
    double price;   /*价格*/
    int total;  /*馆藏总数*/
    int stock;  /*库存数量*/ 
} Book;

/*单向链表节点*/
typedef struct BookNode {
    Book data;  /*数据域：一本图书*/
    struct BookNode *next;  /*指针域：指向下一个节点*/
} BookNode;

/*链表头指针类型*/
typedef BookNode *BookList;

/*链表操作函数声明*/
BookList createList(void);  /*创建空链表，返回NULL*/
int appendBook(BookList *head, Book b); /*尾部插入，成功返回1，失败返回0*/
void genBookId(BookList head, char *buf, int size); /*生成下一个图书编号（如B009）*/
int removeBook(BookList *head, const char *key);    /*按编号或书名删除节点，成功返回1，未找到返回0*/
int listLength(BookList head);  /*返回链表长度*/
void traverseList(BookList head, void (*visit)(Book *b));   /*遍历，对每个节点调用visit*/
void freeList(BookList head);   /*释放整条链表*/

#endif
