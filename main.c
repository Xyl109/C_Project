#include <stdio.h>
#include <string.h>
#include "book.h"
#include "query.h"
#include "borrow.h"
#include "admin.h"
#include "user.h"
#include "blacklist.h"

/*清空缓冲区*/
static void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/*读取一行字符串并去掉末尾换行*/
static void readLine(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
}

/*计算UTF-8字符串的显示宽度：三字节CJK按2列，其余按1列*/
static int displayWidth(const char *s) {
    int w = 0;
    const unsigned char *p = (const unsigned char *)s;
    while (*p != '\0') {
        if (*p >= 0xE0) { w += 2; p += 3; }
        else if (*p >= 0xC0) { w += 1; p += 2; }
        else { w += 1; p += 1; }
    }
    return w;
}

/*按指定显示宽度打印字符串，不足部分补空格（解决中文列错位）*/
static void printPadded(const char *s, int width) {
    printf("%s", s);
    for (int pad = width - displayWidth(s); pad > 0; pad--) {
        putchar(' ');
    }
}

/*显示一本图书的详细信息*/
static void printBook(Book *b) {
    printf("编号:");
    printPadded(b->id, 10);
    printf("书名:");
    printPadded(b->name, 20);
    printf("作者:");
    printPadded(b->author, 12);
    printf("出版社:");
    printPadded(b->publisher, 16);
    printf("年份:%d 价格:%.2f 馆藏:%d 库存:%d\n", b->year, b->price, b->total, b->stock);
}

/*列表展示一本图书：仅显示书名/作者/出版社/出版年份/库存数量（管理员列表展示回调）*/
static void printBookBrief(Book *b) {
    printPadded(b->name, 20);
    printPadded(b->author, 12);
    printPadded(b->publisher, 16);
    printf("%-6d %-6d\n", b->year, b->stock);
}

/*显示查询结果：找到则列出全部，未找到则显示错误信息*/
static void showResult(QueryResult *r, const char *condName) {
    if (r->count == 0) {
        printf("未找到符合条件的图书(条件：%s)!\n", condName);
        if (!r->ok) {
            printf("警告：内存不足，查询结果可能不完整！\n");
        }
        return;
    }
    if (!r->ok) {
        printf("警告：内存不足，查询结果可能不完整！\n");
    }
    printf("共找到%d本图书:\n", r->count);
    for (int i = 0; i < r->count; i++) {
        printBook(&r->nodes[i]->data);
    }
}

/*图书查询子菜单*/
static void queryMenu(BookList head) {
    int choice;
    while (1) {
        printf("\n======图书查询======\n");
        printf("1.按书名查询\n");
        printf("2.按作者查询\n");
        printf("3.按出版社查询\n");
        printf("4.按出版年份查询\n");
        printf("5.按库存数量\n");
        printf("0.返回主菜单\n");
        printf("请选择:");
        int ret = scanf("%d", &choice);
        if (ret == EOF) {
            return;
        }
        if (ret != 1) {
            printf("输入无效，请重新输入！\n");
            clearInput();
            continue;
        }
        clearInput();

        QueryResult r;
        char kw[64];
        int year, op, value;

        switch (choice) {
            case 1:
                printf("请输入书名关键字：");
                readLine(kw, sizeof(kw));
                r = queryByTitle(head, kw);
                showResult(&r, "书名");
                freeQueryResult(&r);
                break;
            case 2:
                printf("请输入作者关键字：");
                readLine(kw, sizeof(kw));
                r = queryByAuthor(head, kw);
                showResult(&r, "作者");
                freeQueryResult(&r);
                break;
            case 3:
                printf("请输入出版社关键字：");
                readLine(kw, sizeof(kw));
                r = queryByPublisher(head, kw);
                showResult(&r, "出版社");
                freeQueryResult(&r);
                break;
            case 4:
                printf("请输入出版年份：");
                if (scanf("%d", &year) != 1) {
                    printf("输入无效，请重新输入！\n");
                    clearInput();
                    break;
                }
                clearInput();
                r = queryByYear(head, year);
                showResult(&r, "出版年份");
                freeQueryResult(&r);
                break;
            case 5:
                printf("比较方式(1:库存<=, 2:库存>=, 3:库存=):");
                if (scanf("%d", &op) != 1 || op < 1 || op > 3) {
                    printf("无效的比较方式，请重新输入！\n");
                    clearInput();
                    break;
                }
                printf("请输入库存数量:");
                if (scanf("%d", &value) != 1 || value < 0) {
                    printf("无效的库存数量，请重新输入！\n");
                    clearInput();
                    break;
                }
                clearInput();
                r = queryByStock(head, op, value);
                showResult(&r, "库存数量");
                freeQueryResult(&r);
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
}

/*打印一本在借图书：完整图书信息 + 借阅日期 + 归还日期（借阅查询回调）*/
static void printBorrowedBook(Book *b, const char *date, const char *dueDate) {
    printBook(b);
    printf("借阅日期:%s\n", date);
    printf("归还日期:%s\n", dueDate);
}

/*图书借阅子菜单*/
static void borrowdMenu(BookList head, BorrowList *records, Blacklist *blacklist) {
    int choice;
    while (1) {
        printf("\n======图书借阅======\n");
        printf("1.借阅图书\n");
        printf("2.归还图书\n");
        printf("3.借阅查询\n");
        printf("0.返回主菜单\n");
        printf("请选择：");
        int ret = scanf("%d", &choice);
        if (ret == EOF) {
            return;
        }
        if (ret != 1) {
            printf("输入无效，请重新输入！\n");
            clearInput();
            continue;
        }
        clearInput();

        switch (choice) {
            case 1:
                printf("请输入需要借阅的书籍书名或者编号：");
                char key[64];
                readLine(key, sizeof(key));
                if (key[0] == '\0') {
                    printf("输入为空，借阅取消！\n");
                    break;
                }
                printf("请输入借阅人姓名：");
                char userName[30];
                readLine(userName, sizeof(userName));
                if (userName[0] == '\0') {
                    printf("输入为空，借阅取消!\n");
                    break;
                }
                BorrowStatus st = borrowBook(head, records, *blacklist, key, userName);
                switch (st) {
                    case BORROW_OK: {
                        char due[20];
                        computeDueDate(due, sizeof(due));
                        printf("借阅成功！库存已减一。请记得到店归还，归还日期:%s。\n", due);
                        break;
                    }
                    case BORROW_NOT_FOUND:
                        printf("未找到编号或名称为\"%s\"的图书！\n", key);
                        break;
                    case BORROW_NO_STOCK:
                        printf("该图书已无库存，借阅失败！\n");
                        break;
                    case BORROW_RECORD_FAIL:
                        printf("内存不足，借阅记录创建失败，请稍后重试!\n");
                        break;
                    case BORROW_BLACKLISTED:
                        printf("借阅人\"%s\"已被拉入借阅黑名单，禁止借阅图书！\n", userName);
                        break;
                    default:
                        printf("未知错误！\n");
                }
                break;
            case 2: {
                printf("请输入需要归还的书籍书名或者编号：");
                char retKey[64];
                readLine(retKey, sizeof(retKey));
                if (retKey[0] == '\0') {
                    printf("输入为空，归还取消！\n");
                    break;
                }
                ReturnStatus rs = returnBook(head, records, blacklist, retKey);
                switch (rs) {
                    case RETURN_OK:
                        printf("归还成功！库存已加一。\n");
                        break;
                    case RETURN_OVERDUE:
                        printf("归还成功！库存已加一，但已超过归还日期，已记录一次超时。\n");
                        break;
                    case RETURN_NOT_FOUND:
                        printf("未找到编号或者名称为\"%s\"的图书！\n", retKey);
                        break;
                    case RETURN_NO_RECORD:
                        printf("该图书没有未归还的借阅记录，归还失败！\n");
                        break;
                    default:
                        printf("未知错误！\n");
                }
                break;
            }
            case 3: {
                printf("请输入借阅人姓名：");
                char queryName[30];
                readLine(queryName, sizeof(queryName));
                if (queryName[0] == '\0') {
                    printf("输入为空，查询取消！\n");
                    break;
                }
                int cnt = countBorrowedByUser(*records, queryName);
                if (cnt == 0) {
                    printf("借阅人\"%s\"当前没有未归还的借阅记录！\n", queryName);
                    break;
                }
                printf("借阅人\"%s\"当前共借阅%d本图书:\n", queryName, cnt);
                traverseBorrowedByUser(head, *records, queryName, printBorrowedBook);
                break;
            }
            case 0:
                return;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
}

/*浏览全部图书*/
static void showAll(BookList head) {
    if (head == NULL) {
        printf("当前没有图书数据!\n");
        return;
    }
    printf("当前共有%d本图书:\n", listLength(head));
    traverseList(head, printBook);
}

/*管理员列表展示全部图书：仅列书名/作者/出版社/出版年份/库存数量，空表给出提示*/
static void showBookList(BookList head) {
    if (head == NULL) {
        printf("当前没有图书数据！\n");
        return;
    }
    printf("当前共有%d本图书:\n", listLength(head));
    printPadded("书名", 20);
    printPadded("作者", 12);
    printPadded("出版社", 16);
    printf("%-6s %-6s\n", "年份", "库存");
    traverseList(head, printBookBrief);
}

/*显示一个用户信息：用户名 + 密码*/
static void printUser(User *u) {
    printf("用户名:%s\n", u->username);
    printf("密码:%s\n", u->password);
}

/*管理员列表展示全部用户：仅列用户名/密码，空表给出提示*/
static void showUserList(UserList head) {
    if (head == NULL) {
        printf("当前没有用户数据！\n");
        return;
    }
    printf("当前共有%d个用户:\n", userListLength(head));
    printf("%-20s %s\n", "用户名", "密码");
    traverseUserList(head, printUser);
}

/*内置样例用户*/
static void initSampleUsers(UserList *head) {
    User samples[] = {
        {"zhangsan", "123456"},
        {"lisi", "123456"}
    };
    for (int i = 0; i < (int)(sizeof(samples) / sizeof(samples[0])); i++) {
        if (!appendUser(head, samples[i])) {
            printf("内存不足，样例用户加载不完整！\n");
            return;
        }
    }
}

/*添加新用户：用户名/密码非空，用户名重复则拒绝*/
static void inputNewUser(UserList *head) {
    User u;
    memset(&u, 0, sizeof(u));

    printf("请输入用户名：");
    readLine(u.username, sizeof(u.username));
    if (u.username[0] == '\0') {
        printf("输入为空，添加取消！\n");
        return;
    }
    if (findUserByName(*head, u.username) != NULL) {
        printf("用户名\"%s\"已存在，添加取消！\n", u.username);
        return;
    }

    printf("请输入密码：");
    readLine(u.password, sizeof(u.password));
    if (u.password[0] == '\0') {
        printf("输入为空，添加取消！\n");
        return;
    }

    if (!appendUser(head, u)) {
        printf("内存不足，用户添加失败！\n");
        return;
    }
    printf("用户添加成功！\n");
    printUser(&u);
}

/*删除用户：按用户名查找，找到则删除，未找到提示*/
static void deleteUser(UserList *head) {
    char key[64];
    printf("请输入要删除的用户名：");
    readLine(key, sizeof(key));
    if (key[0] == '\0') {
        printf("输入为空，删除取消！\n");
        return;
    }
    if (removeUser(head, key)) {
        printf("用户删除成功！\n");
    } else {
        printf("未找到用户名为\"%s\"的用户！\n", key);
    }
}

/*更新用户信息：按用户名定位，更新密码（回车保留原值）*/
static void updateUserInfo(UserList head) {
    char key[64];
    printf("请输入要更新的用户名：");
    readLine(key, sizeof(key));
    if (key[0] == '\0') {
        printf("输入为空，更新取消！\n");
        return;
    }
    UserNode *node = findUserByName(head, key);
    if (node == NULL) {
        printf("未找到用户名为\"%s\"的用户！\n", key);
        return;
    }

    printf("当前用户信息：\n");
    printUser(&node->data);

    char buf[64];
    printf("请输入新的密码（回车保留原值）：");
    readLine(buf, sizeof(buf));
    if (buf[0] == '\0') {
        printf("未修改，更新取消！\n");
        return;
    }
    updateUserPassword(head, key, buf);
    printf("用户信息更新成功！\n");
    printUser(&node->data);
}

/*用户信息管理子菜单：添加/删除/更新/查看列表*/
static void userMenu(UserList *head) {
    int choice;
    while (1) {
        printf("\n======用户信息管理======\n");
        printf("1.添加用户\n");
        printf("2.删除用户\n");
        printf("3.更新用户信息\n");
        printf("4.查看用户列表\n");
        printf("0.返回\n");
        printf("请选择：");
        int ret = scanf("%d", &choice);
        if (ret == EOF) {
            return;
        }
        if (ret != 1) {
            printf("输入无效，请重新输入！\n");
            clearInput();
            continue;
        }
        clearInput();

        switch (choice) {
            case 1:
                inputNewUser(head);
                break;
            case 2:
                deleteUser(head);
                break;
            case 3:
                updateUserInfo(*head);
                break;
            case 4:
                showUserList(*head);
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
}

/*内置样例数据*/
static void initSampleData(BookList *head) {
    Book samples[] = {
        {"B001", "算法导论", "Thomas H.cormen", "机械工业出版社", 2009, 128.00, 5, 3},
        {"B002", "数据结构(C语言版)", "严蔚敏", "清华大学出版社", 2011, 49.00, 8, 8},
        {"B003", "深入理解计算机系统", "Randal E. Bryant", "机械工业出版社", 2016, 139.00, 4, 1},
        {"B004", "算法图解", "Aditya Bhargava", "人民邮电出版社", 2017, 49.00, 10, 10},
        {"B005", "C程序设计语言", "Brian W. Kernighan", "机械工业出版社", 2004, 30.00, 6, 0},
        {"B006", "Hello算法", "靳宇栋", "人民邮电出版社", 2024, 129.00, 12, 12},
        {"B007", "计算机网络", "谢希仁", "电子工业出版社", 2017, 45.00, 7, 2},
        {"B008", "数据库系统概论", "王珊", "高等教育出版社", 2014, 52.00, 9, 9}
    };
    for (int i = 0; i < (int)(sizeof(samples) / sizeof(samples[0])); i++) {
        if (!appendBook(head, samples[i])) {
            printf("内存不足，样例数据加载不完整！\n");
            return;
        }
    }
}

/*管理员登录*/
static int adminLogin(void) {
    const int MAX_ATTEMPTS = 3;
    int remaining = MAX_ATTEMPTS;
    char username[20];
    char password[20];
    while (remaining > 0) {
        printf("请输入管理员用户名：");
        readLine(username, sizeof(username));
        printf("请输入密码：");
        readLine(password, sizeof(password));
        if (username[0] == '\0' || password[0] == '\0') {
            printf("用户名或密码不能为空，请重新输入！\n");
            continue;
        }
        if (verifyAdmin(username, password)) {
            printf("管理员登录成功！\n");
            return 1;
        }
        remaining--;
        printf("用户名或密码错误，剩余%d次机会！\n", remaining);
    }
    printf("登录失败次数过多，返回主菜单！\n");
    return 0;
}

/*录入一本新图书：书名/作者/出版社/出版年份/库存数量*/
static void inputNewBook(BookList *head) {
    Book b;
    memset(&b, 0, sizeof(b));

    printf("请输入书名：");
    readLine(b.name, sizeof(b.name));
    if (b.name[0] == '\0') {
        printf("输入为空，录入取消！\n");
        return;
    }

    printf("请输入作者：");
    readLine(b.author, sizeof(b.author));
    if (b.author[0] == '\0') {
        printf("输入为空，录入取消！\n");
        return;
    }

    printf("请输入出版社：");
    readLine(b.publisher, sizeof(b.publisher));
    if (b.publisher[0] == '\0') {
        printf("输入为空，录入取消！\n");
        return;
    }

    printf("请输入出版年份：");
    if (scanf("%d", &b.year) != 1 || b.year <= 0 || b.year >= 2100) {
        printf("出版年份无效，录入取消！\n");
        clearInput();
        return;
    }
    clearInput();

    printf("请输入库存数量：");
    if (scanf("%d", &b.stock) != 1 || b.stock < 0) {
        printf("库存数量无效，录入取消！\n");
        clearInput();
        return;
    }
    clearInput();

    genBookId(*head, b.id, sizeof(b.id));
    b.total = b.stock;
    b.price = 0.00;

    if (!appendBook(head, b)) {
        printf("内存不足，图书录入失败！\n");
        return;
    }
    printf("图书录入成功！编号:%s\n", b.id);
    printBook(&b);
}

/*更新图书信息：按编号或书名定位，逐项更新书名/作者/出版社/出版年份/库存数量*/
static void updateBookInfo(BookList head) {
    char key[64];
    printf("请输入要更新的图书编号或书名：");
    readLine(key, sizeof(key));
    if (key[0] == '\0') {
        printf("输入为空，更新取消！\n");
        return;
    }

    BookNode *node = findBookById(head, key);
    if (node == NULL) {
        node = findBookByName(head, key);
    }
    if (node == NULL) {
        printf("未找到编号或名称为\"%s\"的图书！\n", key);
        return;
    }

    printf("当前图书信息：\n");
    printBook(&node->data);

    char buf[64];

    printf("请输入新的书名（回车保留原值）：");
    readLine(buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(node->data.name, buf, sizeof(node->data.name) - 1);
        node->data.name[sizeof(node->data.name) - 1] = '\0';
    }

    printf("请输入新的作者（回车保留原值）：");
    readLine(buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(node->data.author, buf, sizeof(node->data.author) - 1);
        node->data.author[sizeof(node->data.author) - 1] = '\0';
    }

    printf("请输入新的出版社（回车保留原值）：");
    readLine(buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(node->data.publisher, buf, sizeof(node->data.publisher) - 1);
        node->data.publisher[sizeof(node->data.publisher) - 1] = '\0';
    }

    printf("请输入新的出版年份（回车保留原值）：");
    readLine(buf, sizeof(buf));
    if (buf[0] != '\0') {
        int year = 0;
        if (sscanf(buf, "%d", &year) == 1 && year > 0 && year < 2100) {
            node->data.year = year;
        } else {
            printf("出版年份无效，保留原值！\n");
        }
    }

    printf("请输入新的库存数量（回车保留原值）：");
    readLine(buf, sizeof(buf));
    if (buf[0] != '\0') {
        int stock = 0;
        if (sscanf(buf, "%d", &stock) == 1 && stock >= 0) {
            node->data.stock = stock;
        } else {
            printf("库存数量无效，保留原值！\n");
        }
    }

    printf("图书信息更新成功！\n");
    printBook(&node->data);
}

/*管理员功能子菜单：图书信息录入等（后续功能在此扩展）*/
static void adminMenu(BookList *head, UserList *users) {
    int choice;
    while (1) {
        printf("\n======管理员功能======\n");
        printf("1.图书信息录入\n");
        printf("2.删除图书信息\n");
        printf("3.图书信息更新\n");
        printf("4.图书信息显示\n");
        printf("5.用户信息管理\n");
        printf("0.返回主菜单\n");
        printf("请选择：");
        int ret = scanf("%d", &choice);
        if (ret == EOF) {
            return;
        }
        if (ret != 1) {
            printf("输入无效，请重新输入！\n");
            clearInput();
            continue;
        }
        clearInput();

        switch (choice) {
            case 1:
                inputNewBook(head);
                break;
            case 2: {
                printf("请输入要删除的图书编号或书名：");
                char delKey[64];
                readLine(delKey, sizeof(delKey));
                if (delKey[0] == '\0') {
                    printf("输入为空，删除取消！\n");
                    break;
                }
                if (removeBook(head, delKey)) {
                    printf("图书删除成功！\n");
                } else {
                    printf("未找到编号或名称为\"%s\"的图书！\n", delKey);
                }
                break;
            }
            case 3:
                updateBookInfo(*head);
                break;
            case 4:
                showBookList(*head);
                break;
            case 5:
                userMenu(users);
                break;
            case 0:
                return;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
}

int main(void) {
    BookList list = createList();
    initSampleData(&list);
    BorrowList records = createBorrowList();
    UserList users = createUserList();
    initSampleUsers(&users);
    Blacklist blacklist = createBlacklist();

    int choice;
    while (1) {
        printf("\n======图书管理系统======\n");
        printf("1.图书查询\n");
        printf("2.浏览全部图书\n");
        printf("3.图书借阅\n");
        printf("4.管理员登录\n");
        printf("0.退出\n");
        printf("请选择：");
        int ret = scanf("%d", &choice);
        if (ret == EOF) {
            break;
        }
        if (ret != 1) {
            printf("输入无效，请重新输入！\n");
            clearInput();
            continue;
        }
        clearInput();

        switch (choice) {
            case 1: queryMenu(list);    break;
            case 2: showAll(list);  break;
            case 3: borrowdMenu(list, &records, &blacklist);    break;
            case 4: if (adminLogin()) { adminMenu(&list, &users); }   break;
            case 0:
                freeList(list);
                freeBorrowList(records);
                freeUserList(users);
                freeBlacklist(blacklist);
                printf("感谢使用,再见！\n");
                return 0;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
    freeList(list);
    freeBorrowList(records);
    freeUserList(users);
    freeBlacklist(blacklist);
    printf("感谢使用,再见！\n");
    return 0;
}
