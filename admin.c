#include "admin.h"
#include <string.h>

/*默认管理员账号*/
static const Admin DEFAULT_ADMIN = {"admin", "123456"};

/*校验管理员登录*/
int verifyAdmin(const char *username, const char *password) {
    if (strcmp(username, DEFAULT_ADMIN.username) == 0 && strcmp(password, DEFAULT_ADMIN.password) == 0) {
        return 1;   /*校验通过*/
    }
    return 0;   /*校验失败*/
}
