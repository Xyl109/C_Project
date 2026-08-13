#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*用户信息结构体*/
typedef struct {
    char username[20];  /*用户名（唯一标识）*/
    char password[20];  /*密码*/
} User;

/*单向链表节点*/
typedef struct UserNode {
    User data;  /*数据域：一个用户*/
    struct UserNode *next;  /*指针域：指向下一个节点*/
} UserNode;

/*用户链表头指针类型*/
typedef UserNode *UserList;

/*创建空用户链表，返回NULL*/
UserList createUserList(void);

/*尾部插入用户：成功返回1，失败（内存不足）返回0*/
int appendUser(UserList *head, User u);

/*按用户名精确查找：O(n)线性扫描，找到返回节点指针，否则返回NULL*/
UserNode *findUserByName(UserList head, const char *username);

/*按用户名删除节点：找到则摘链并释放内存，返回1；未找到返回0*/
int removeUser(UserList *head, const char *username);

/*按用户名更新密码：找到返回1并写入新密码，未找到返回0*/
int updateUserPassword(UserList head, const char *username, const char *newPassword);

/*返回用户链表长度：O(n)*/
int userListLength(UserList head);

/*遍历用户链表，对每个节点调用visit回调*/
void traverseUserList(UserList head, void (*visit)(User *u));

/*释放整条用户链表，防止内存泄漏*/
void freeUserList(UserList head);

#endif
