#include "borrow.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

/*获取当前日期到buf（格式 yy-mm-dd）*/
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

/*计算归还日期：当前日期 + BORROW_DAYS 天，写入buf（格式 yy-mm-dd）*/
void computeDueDate(char *buf, int size) {
    time_t now = time(NULL);
    time_t due = now + (time_t)BORROW_DAYS * 24 * 60 * 60;
    struct tm *tm = localtime(&due);
    if (tm == NULL) {
        strncpy(buf, "未知日期", (size_t)size - 1);
        buf[size - 1] = '\0';
        return;
    }
    strftime(buf, (size_t)size, "%y-%m-%d", tm);
}

/*判断是否已逾期：当前日期 > 归还日期（yy-mm-dd 定宽零填充，字符串序即时间序）*/
static int isOverdue(const char *dueDate) {
    char today[20];
    getCurrenDate(today, sizeof(today));
    return strcmp(today, dueDate) > 0;
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
        *head = newNode;
    }else {
        BorrowNode *p = *head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = newNode;
    }
    return 1;
}

/*借阅一本图书*/
BorrowStatus borrowBook(BookList head, BorrowList *records, Blacklist blacklist, const char *key, const char *userName) {
    if (isBlacklisted(blacklist, userName)) {
        return BORROW_BLACKLISTED;
    }
    BookNode *node = findBookById(head, key);
    if (node == NULL) {
        node = findBookByName(head, key);
    }
    if (node == NULL) {
        return BORROW_NOT_FOUND;
    }
    if (node->data.stock <= 0) {
        return BORROW_NO_STOCK;
    }
    BorrowRecord rec;
    memset(&rec, 0, sizeof(rec));
    strncpy(rec.userName, userName, sizeof(rec.userName) - 1);
    strncpy(rec.bookId, node->data.id, sizeof(rec.bookId) - 1);
    strncpy(rec.bookName, node->data.name, sizeof(rec.bookName) - 1);
    getCurrenDate(rec.date, sizeof(rec.date));
    computeDueDate(rec.dueDate, sizeof(rec.dueDate));

    if (!appendRecord(records, rec)) {
        return BORROW_RECORD_FAIL;
    }

    node->data.stock--;

    return BORROW_OK;
}

/*归还一本图书：按照编号或者书名查找，找到后标记借阅记录已归还并将库存加1*/
ReturnStatus returnBook(BookList head, BorrowList *records, Blacklist *blacklist, const char *key) {
    BookNode *node = findBookById(head, key);
    if (node == NULL) {
        node = findBookByName(head, key);
    }
    if (node == NULL) {
        return RETURN_NOT_FOUND;
    }
    BorrowNode *p = *records;
    while (p != NULL) {
        if (strcmp(p->data.bookId, node->data.id) == 0 && !p->data.returned) {
            break;
        }
        p = p->next;
    }
    if (p == NULL) {
        return RETURN_NO_RECORD;
    }
    p->data.returned = 1;
    node->data.stock++;

    if (isOverdue(p->data.dueDate)) {
        recordOverdue(blacklist, p->data.userName);
        return RETURN_OVERDUE;
    }
    return RETURN_OK;
}

/*统计某借阅人未归还的借阅记录条数*/
int countBorrowedByUser(BorrowList records, const char *userName) {
    int n = 0;
    BorrowNode *p = records;
    while (p != NULL) {
        if (strcmp(p->data.userName, userName) == 0 && !p->data.returned) {
            n++;
        }
        p = p->next;
    }
    return n;
}

/*遍历某借阅人未归还的借阅记录：按bookId查回图书节点，逐条调用visit(图书, 借阅日期, 归还日期)*/
void traverseBorrowedByUser(BookList head, BorrowList records, const char *userName, void (*visit)(Book *b, const char *date, const char *dueDate)) {
    BorrowNode *p = records;
    while (p != NULL) {
        if (strcmp(p->data.userName, userName) == 0 && !p->data.returned) {
            BookNode *node = findBookById(head, p->data.bookId);
            if (node != NULL) {
                visit(&node->data, p->data.date, p->data.dueDate);
            }
        }
        p = p->next;
    }
}

/*释放整条借阅记录链表*/
void freeBorrowList(BorrowList head) {
    BorrowNode *p = head;
    while (p != NULL) {
        BorrowNode *temp = p->next;
        free(p);
        p = temp;
    }
}
