#ifndef ADMIN_H
#define ADMIN_H

/*管理员账号信息*/
typedef struct {
    char username[20];  /*用户名*/
    char password[20];  /*密码*/
} Admin;

/*校验管理员登录*/
int verifyAdmin(const char *username, const char *password);

#endif
