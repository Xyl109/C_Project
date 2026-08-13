#include "blacklist.h"

/*创建空黑名单链表*/
Blacklist createBlacklist(void) {
    return NULL;
}

/*按借阅人姓名查找黑名单节点*/
BlacklistNode *findBlacklistByName(Blacklist head, const char *userName) {
    BlacklistNode *p = head;
    while (p != NULL) {
        if (strcmp(p->userName, userName) == 0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/*判断某借阅人是否已被拉入黑名单*/
int isBlacklisted(Blacklist head, const char *userName) {
    BlacklistNode *node = findBlacklistByName(head, userName);
    return node != NULL && node->overdueCount >= MAX_OVERDUE_TIMES;
}

/*累计某借阅人一次超时：已有节点则计数+1，无则新建节点*/
int recordOverdue(Blacklist *head, const char *userName) {
    BlacklistNode *node = findBlacklistByName(*head, userName);
    if (node != NULL) {
        node->overdueCount++;
        return 1;
    }

    BlacklistNode *newNode = (BlacklistNode *)malloc(sizeof(BlacklistNode));
    if (newNode == NULL) {
        return 0;
    }
    strncpy(newNode->userName, userName, sizeof(newNode->userName) - 1);
    newNode->userName[sizeof(newNode->userName) - 1] = '\0';
    newNode->overdueCount = 1;
    newNode->next = *head;
    *head = newNode;
    return 1;
}

/*查询某借阅人当前累计超时次数*/
int overdueCountOf(Blacklist head, const char *userName) {
    BlacklistNode *node = findBlacklistByName(head, userName);
    return node == NULL ? 0 : node->overdueCount;
}

/*释放整条黑名单链表*/
void freeBlacklist(Blacklist head) {
    BlacklistNode *p = head;
    while (p != NULL) {
        BlacklistNode *temp = p->next;
        free(p);
        p = temp;
    }
}
