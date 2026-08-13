#include "query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*内部工具：向结果数组追加一个命中节点（动态扩容），成功返回1，失败返回0*/
static int addHit(QueryResult *r, BookNode *node) {
    BookNode **newNodes = (BookNode **)realloc(r->nodes, (size_t)(r->count + 1) * (sizeof(BookNode *)));
    if (newNodes == NULL) {
        r->ok = 0;  /*标记结果不完整*/
        return 0;
    }
    r->nodes = newNodes;
    r->nodes[r->count] = node;
    r->count++;
    return 1;
}

/*按书名模糊查询（包含式，strstr）*/
QueryResult queryByTitle(BookList head, const char *keyword) {
    QueryResult r = {NULL, 0, 1};
    BookNode *p = head;
    while (p != NULL) {
        if (strstr(p->data.name, keyword) != NULL) {
            if (!addHit(&r, p)) break;  /*内存不足，停止扫描*/
        }
        p = p->next;
    }
    return r;
}

/*按作者模糊查询*/
QueryResult queryByAuthor(BookList head, const char *keyword) {
    QueryResult r = {NULL, 0, 1};
    BookNode *p = head;
    while (p != NULL) {
        if (strstr(p->data.author, keyword) != NULL) {
            if (!addHit(&r, p)) break;  /*内存不足，停止扫描*/
        }
        p = p->next;
    }
    return r;
}

/*按出版社模糊查询*/
QueryResult queryByPublisher(BookList head, const char *keyword) {
    QueryResult r = {NULL, 0, 1};
    BookNode *p = head;
    while (p != NULL) {
        if (strstr(p->data.publisher, keyword) != NULL) {
            if (!addHit(&r, p)) break;  /*内存不足，停止扫描*/
        }
        p = p->next;
    }
    return r;
}

/*按出版年份精确查询*/
QueryResult queryByYear(BookList head, int year) {
    QueryResult r = {NULL, 0, 1};
    BookNode *p = head;
    while (p != NULL) {
        if (p->data.year == year) {
            if (!addHit(&r, p)) break;  /*内存不足，停止扫描*/
        }
        p = p->next;
    }
    return r;
}

/*按库存数量比较查询:op 1=<=, 2=>=, 3==*/
QueryResult queryByStock(BookList head, int op, int value) {
    QueryResult r = {NULL, 0, 1};
    BookNode *p = head;
    while (p != NULL) {
        int hit = 0;
        int s = p->data.stock;
        if (op == 1)    hit = (s <= value);
        else if (op == 2)    hit = (s >= value);
        else if (op == 3)    hit = (s == value);
        if (hit) {
            if (!addHit(&r, p)) break;  /*内存不足，停止扫描*/
        }
        p = p->next;
    }
    return r;
}

/*释放查询结果*/
void freeQueryResult(QueryResult *r) {
    free(r->nodes);
    r->nodes = NULL;
    r->count = 0;
    r->ok = 1;
}
