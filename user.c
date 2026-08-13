#include "user.h"

/*创建空用户链表*/
UserList createUserList(void) {
    return NULL;
}

/*尾部插入一个用户：遍历到表尾再挂载,O(n)*/
int appendUser(UserList *head, User u) {
    UserNode *newNode = (UserNode *)malloc(sizeof(UserNode));
    if (newNode == NULL) {
        return 0;
    }
    newNode->data = u;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
    } else {
        UserNode *p = *head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = newNode;
    }
    return 1;
}

/*按用户名精确查找*/
UserNode *findUserByName(UserList head, const char *username) {
    UserNode *p = head;
    while (p != NULL) {
        if (strcmp(p->data.username, username) == 0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/*按用户名删除节点：找到则摘链并释放内存，返回1；未找到返回0*/
int removeUser(UserList *head, const char *username) {
    UserNode *prev = NULL;
    UserNode *p = *head;
    while (p != NULL) {
        if (strcmp(p->data.username, username) == 0) {
            if (prev == NULL) {
                *head = p->next;
            } else {
                prev->next = p->next;
            }
            free(p);
            return 1;
        }
        prev = p;
        p = p->next;
    }
    return 0;
}

/*按用户名更新密码：找到则写入新密码并返回1，未找到返回0*/
int updateUserPassword(UserList head, const char *username, const char *newPassword) {
    UserNode *node = findUserByName(head, username);
    if (node == NULL) {
        return 0;
    }
    strncpy(node->data.password, newPassword, sizeof(node->data.password) - 1);
    node->data.password[sizeof(node->data.password) - 1] = '\0';
    return 1;
}

/*返回用户链表长度：O(n)*/
int userListLength(UserList head) {
    int n = 0;
    UserNode *p = head;
    while (p != NULL) {
        n++;
        p = p->next;
    }
    return n;
}

/*遍历用户链表，对每个节点的data调用visit回调*/
void traverseUserList(UserList head, void (*visit)(User *u)) {
    UserNode *p = head;
    while (p != NULL) {
        visit(&p->data);
        p = p->next;
    }
}

/*释放整条用户链表，防止内存泄漏*/
void freeUserList(UserList head) {
    UserNode *p = head;
    while (p != NULL) {
        UserNode *temp = p->next;
        free(p);
        p = temp;
    }
}
