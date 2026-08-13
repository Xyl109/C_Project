#ifndef BORROW_H
#define BORROW_H

#include "book.h"

/*借阅记录结构体*/
typedef struct {
    char userName[30];  /*借阅人姓名*/
    char bookId[20];    /*借阅书籍编号*/
    char bookName[50];  /*借阅书籍名称*/
    char date[20];  /*借阅日期*/
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
    BORROW_RECORD_FAIL = 3  /*内存不足，借阅记录创建失败*/
} BorrowStatus;

/*归还图书状态码*/
typedef enum {
    RETURN_OK = 0,  /*归还成功*/
    RETURN_NOT_FOUND = 1,   /*未找到图书*/
    RETURN_NO_RECORD = 2    /*该图书没有未归还的借阅记录*/
} ReturnStatus;

/*创建空借阅记录链表，返回NULL*/
BorrowList createBorrowList(void);

/*按照图书编号精确查找：O(n)线性扫描，找到返回节点指针，否则返回NULL*/
BookNode *findBookById(BookList head, const char *id);

/*按照图书书名精确查找:O(n)线性扫描，找到返回节点指针，否则返回NULL*/
BookNode *findBookByName(BookList head, const char *name);

/*流程*/
/*首先按照编号或者书名精确查找*/
/*未找到就返回BORROW_NOT_FOUND*/
/*库存<=0返回BORROW_NO_STOCK*/
/*先追加借阅记录，再库存减一*/
/*key为书名或者编号，userName为借阅人姓名，日期自动取当前系统时间*/
BorrowStatus borrowBook(BookList head, BorrowList *records, const char *key, const char *userName);

/*流程*/
/*首先按照编号或者书名精确查找*/
/*未找到就返回RETURN_NOT_FOUND*/
/*在借阅记录链表中返回RETURN_NO_RECORD*/
/*无未归还记录返回RETURN_NO_RECORD*/
/*找到后标记该记录已归还，库存加一*/
ReturnStatus returnBook(BookList head, BorrowList *records, const char *key);

/*统计某借阅人未归还的借阅记录条数：O(n)线性扫描*/
int countBorrowedByUser(BorrowList records, const char *userName);

/*遍历某借阅人未归还的借阅记录：按bookId查回图书节点，逐条调用visit(图书, 借阅日期)*/
void traverseBorrowedByUser(BookList head, BorrowList records, const char *userName, void (*visit)(Book *b, const char *date));

/*释放整条借阅记录链表，防止内存泄漏*/
void freeBorrowList(BorrowList head);

#endif
