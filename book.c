#include "book.h"

/*创建空链表*/
BookList createList(void) {
    return NULL;
}

/*尾部插入一本书：遍历到表尾再挂载,O(n)*/
int appendBook(BookList *head, Book b) {
    BookNode *newNode = (BookNode *)malloc(sizeof(BookNode));
    if (newNode == NULL) {
        return 0;   /*内存分配失败*/
    }
    newNode->data = b;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;    /*空链表：新节点即是头节点*/
    } else {
        BookNode *p = *head;
        while (p->next != NULL) {   /*走到链表末尾*/
            p = p->next;
        }
        p->next = newNode;  /*挂到表尾*/
    }
    return 1;
}

/*生成下一个图书编号：解析现有B%d编号取最大值加1，如B009*/
void genBookId(BookList head, char *buf, int size) {
    int max = 0;
    BookNode *p = head;
    while (p != NULL) {
        int n = 0;
        if (sscanf(p->data.id, "B%d", &n) == 1 && n > max) {
            max = n;
        }
        p = p->next;
    }
    snprintf(buf, (size_t)size, "B%03d", max + 1);
}

/*按编号或书名删除图书节点：找到则摘链并释放内存，返回1；未找到返回0*/
int removeBook(BookList *head, const char *key) {
    BookNode *prev = NULL;
    BookNode *p = *head;
    while (p != NULL) {
        if (strcmp(p->data.id, key) == 0 || strcmp(p->data.name, key) == 0) {
            if (prev == NULL) {
                *head = p->next;    /*删除头节点，更新头指针*/
            } else {
                prev->next = p->next;   /*摘链*/
            }
            free(p);    /*释放节点内存*/
            return 1;
        }
        prev = p;
        p = p->next;
    }
    return 0;   /*未找到*/
}

/*返回链表长度:O(n)*/
int listLength(BookList head) {
    int n = 0;
    BookNode *p = head;
    while (p != NULL) {
        n++;
        p = p->next;
    }
    return n;
}

/*遍历链表，对每个节点的data调用visit回调*/
void traverseList(BookList head, void(*visit)(Book *b)) {
    BookNode *p = head;
    while (p != NULL) {
        visit(&p->data);
        p = p->next;
    }
}

/*释放整条链表，防止内存泄漏*/
void freeList(BookList head) {
    BookNode *p = head;
    while (p != NULL) {
        BookNode *temp = p->next;   /*先保存后继节点*/
        free(p);
        p = temp;
    }
}
