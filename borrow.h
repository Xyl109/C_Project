#ifndef BORROW_H
#define BORROW_H

#include "book.h"
#include "blacklist.h"

/*借阅期限（天）：借阅成功之日起可借阅的天数*/
#define BORROW_DAYS 30

/*借阅记录结构体*/
typedef struct {
    char userName[30];  /*借阅人姓名*/
    char bookId[20];    /*借阅书籍编号*/
    char bookName[50];  /*借阅书籍名称*/
    char date[20];      /*借阅日期*/
    char dueDate[20];   /*归还日期（借阅日期 + 借阅期限）*/
    int returned;   /*归还状态：0=未归还，1=已归还*/
} BorrowRecord;

/*借阅记录链表节点*/
typedef struct BorrowNode {
    BorrowRecord data;  /*数据域：一条借阅记录*/
    struct BorrowNode *next;    /*指针域：指向下一个节点*/
} BorrowNode;

/*借阅记录链表头指针*/
typedef BorrowNode *BorrowList;

/*借阅记录状态码*/
typedef enum {
    BORROW_OK = 0,  /*借阅成功*/
    BORROW_NOT_FOUND = 1,   /*未找到图书*/
    BORROW_NO_STOCK = 2,    /*库存为0，无法借阅*/
    BORROW_RECORD_FAIL = 3, /*内存不足，借阅记录创建失败*/
    BORROW_BLACKLISTED = 4  /*借阅人已被拉入黑名单，禁止借阅*/
} BorrowStatus;

/*归还图书状态码*/
typedef enum {
    RETURN_OK = 0,  /*归还成功（未逾期）*/
    RETURN_NOT_FOUND = 1,   /*未找到图书*/
    RETURN_NO_RECORD = 2,   /*该图书没有未归还的借阅记录*/
    RETURN_OVERDUE = 3      /*归还成功但已逾期，已累计一次超时*/
} ReturnStatus;

/*创建空借阅记录链表，返回NULL*/
BorrowList createBorrowList(void);

/*按照图书编号精确查找：O(n)线性扫描，找到返回节点指针，否则返回NULL*/
BookNode *findBookById(BookList head, const char *id);

/*按照图书书名精确查找:O(n)线性扫描，找到返回节点指针，否则返回NULL*/
BookNode *findBookByName(BookList head, const char *name);

/*流程*/
/*首先检查借阅人是否在黑名单（是则返回BORROW_BLACKLISTED）*/
/*然后按照编号或者书名精确查找*/
/*未找到就返回BORROW_NOT_FOUND*/
/*库存<=0返回BORROW_NO_STOCK*/
/*先追加借阅记录，再库存减一*/
/*key为书名或者编号，userName为借阅人姓名，日期自动取当前系统时间，归还日期=借阅日期+借阅期限*/
BorrowStatus borrowBook(BookList head, BorrowList *records, Blacklist blacklist, const char *key, const char *userName);

/*计算归还日期：当前日期 + BORROW_DAYS 天，写入buf（格式 yy-mm-dd）*/
void computeDueDate(char *buf, int size);

/*流程*/
/*首先按照编号或者书名精确查找*/
/*未找到就返回RETURN_NOT_FOUND*/
/*在借阅记录链表中返回RETURN_NO_RECORD*/
/*无未归还记录返回RETURN_NO_RECORD*/
/*找到后标记该记录已归还，库存加一*/
/*若归还日期已过则计入一次超时（recordOverdue），返回RETURN_OVERDUE，否则RETURN_OK*/
ReturnStatus returnBook(BookList head, BorrowList *records, Blacklist *blacklist, const char *key);

/*统计某借阅人未归还的借阅记录条数：O(n)线性扫描*/
int countBorrowedByUser(BorrowList records, const char *userName);

/*遍历某借阅人未归还的借阅记录：按bookId查回图书节点，逐条调用visit(图书, 借阅日期, 归还日期)*/
void traverseBorrowedByUser(BookList head, BorrowList records, const char *userName, void (*visit)(Book *b, const char *date, const char *dueDate));

/*释放整条借阅记录链表，防止内存泄漏*/
void freeBorrowList(BorrowList head);

#endif
