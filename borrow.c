#include "borrow.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

/*获取当前日期到buf*/
static void getCurrenDate(char *buf, int size) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    if (tm == NULL) {
        strncpy(buf, "未知日期", (size_t)size - 1);
        buf[size - 1] = '\0';
        return;
    }
    strftime(buf, (size_t)size, "%y-%m-%d", tm);
}

/*创建空借阅记录链表*/
BookList createBorrowList(void) {
    return NULL;
}

/*按照图书编号查找*/
BookNode *findBookById(BookList head, const char *id) {
    BookNode *p = head;
    while (p != NULL) {
        if (strcmp(p->data.id, id) == 0) {
            return p;
        }
        p->next;
    }
    return NULL;
}

/*按照图书名称查找*/
BookNode *findBookByName(BookList head, const char *name) {
    BookNode *p = head;
    while (p != NULL) {
        if (strcmp(p->data.name, name) == 0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/*向借阅记录链表尾部追加一条记录*/
static int appendRecord(BorrowList *head, BorrowRecord rec) {
    BorrowNode *newNode = (BorrowNode *)malloc(sizeof(BorrowNode));
    if (newNode == NULL) {
        return 0;
    }
    newNode->data = rec;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;    /*空链表：新节点即头节点*/
    }else {
        BorrowNode *p = *head;
        while (p->next != NULL) {   /*走点链表尾部*/
            p = p->next;
        }
        p->next = newNode;  /*挂到表尾*/
    }
    return 1;
}

/*借阅一本图书*/
BorrowStatus borrowBook(BookList head, BorrowList *records, const char *key, const char *userName) {
    /*查找书籍，先按编号再按书名*/
    BookNode *node = findBookById(head, key);
    if (node == NULL) {
        node = findBookByName(head, key);
    } else if (node == NULL) {
        return BORROW_NOT_FOUND;    /*未找到*/
    } 
}