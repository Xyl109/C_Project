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
BorrowList createBorrowList(void) {
    return NULL;
}

/*按照图书编号查找*/
BookNode *findBookById(BookList head, const char *id) {
    BookNode *p = head;
    while (p != NULL) {
        if (strcmp(p->data.id, id) == 0) {
            return p;
        }
        p = p->next;
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
    }
    if (node == NULL) {
        return BORROW_NOT_FOUND;    /*未找到*/
    }
    /*检查库存*/
    if (node->data.stock <= 0) {
        return BORROW_NO_STOCK; /*库存为0，借阅失败*/
    }
    /*构造借阅记录*/
    BorrowRecord rec;
    memset(&rec, 0, sizeof(rec));
    strncpy(rec.userName, userName, sizeof(rec.userName) - 1);
    strncpy(rec.bookId, node->data.id, sizeof(rec.bookId) - 1);
    strncpy(rec.bookName, node->data.name, sizeof(rec.bookName) - 1);
    getCurrenDate(rec.date, sizeof(rec.date));

    if (!appendRecord(records, rec)) {
        return BORROW_RECORD_FAIL;  /*记录创建失败，库存未扣减*/
    }

    /*扣减库存*/
    node->data.stock--;

    return BORROW_OK;
}

/*归还一本图书：按照编号或者书名查找，找到后标记借阅记录已归还并将库存加1*/
ReturnStatus returnBook(BookList head, BorrowList *records, const char *key) {
    /*查找书籍，先按编号再按书名*/
    BookNode *node = findBookById(head, key);
    if (node == NULL) {
        node = findBookByName(head, key);
    }
    if (node == NULL) {
        return RETURN_NOT_FOUND;    /*未找到图书*/
    }
    /*查找该图书未归还的借阅记录*/
    BorrowNode *p = *records;
    while (p != NULL) {
        if (strcmp(p->data.bookId, node->data.id) == 0 && !p->data.returned) {
            break;
        }
        p = p->next;
    }
    if (p == NULL) {
        return RETURN_NO_RECORD;    /*没有未归还的借阅记录*/
    }
    /*标记已归还，库存加一*/
    p->data.returned = 1;
    node->data.stock++;
    return RETURN_OK;
}

/*释放整条借阅记录链表*/
void freeBorrowList(BorrowList head) {
    BorrowNode *p = head;
    while (p != NULL) {
        BorrowNode *temp = p->next; /*先保存后继节点*/
        free(p);
        p = temp;
    }
}
