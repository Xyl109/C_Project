#include <stdio.h>
#include <string.h>
#include "book.h"
#include "query.h"
#include "borrow.h"

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
        if (*p >= 0xE0) { w += 2; p += 3; }         /*三字节UTF-8（中文等）*/
        else if (*p >= 0xC0) { w += 1; p += 2; }    /*两字节UTF-8*/
        else { w += 1; p += 1; }                    /*ASCII*/
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
        if (ret == EOF) {   /*输入流结束（如管道关闭）*/
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

/*打印一本在借图书：完整图书信息 + 借阅日期（借阅查询回调）*/
static void printBorrowedBook(Book *b, const char *date) {
    printBook(b);
    printf("借阅日期:%s\n", date);
}

/*图书借阅子菜单*/
static void borrowdMenu(BookList head, BorrowList *records) {
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
            /*输入流结束通道关闭*/
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
                BorrowStatus st = borrowBook(head, records, key, userName);
                switch (st) {
                    case BORROW_OK:
                        printf("借阅成功！库存已减一。\n");
                        break;
                    case BORROW_NOT_FOUND:
                        printf("未找到编号或名称为\"%s\"的图书！\n", key);
                        break;
                    case BORROW_NO_STOCK:
                        printf("该图书已无库存，借阅失败！\n");
                        break;
                    case BORROW_RECORD_FAIL:
                        printf("内存不足，借阅记录创建失败，请稍后重试!\n");
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
                ReturnStatus rs = returnBook(head, records, retKey);
                switch (rs) {
                    case RETURN_OK:
                        printf("归还成功！库存已加一。\n");
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

int main(void) {
    BookList list = createList();
    initSampleData(&list);
    BorrowList records = createBorrowList();    /*借阅记录链表*/

    int choice;
    while (1) {
        printf("\n======图书管理系统======\n");
        printf("1.图书查询\n");
        printf("2.浏览全部图书\n");
        printf("3.图书借阅\n");
        printf("0.退出\n");
        printf("请选择：");
        int ret = scanf("%d", &choice);
        if (ret == EOF) {   /*输入流结束（如管道关闭），退出*/
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
            case 3: borrowdMenu(list, &records);    break;
            case 0:
                freeList(list);
                freeBorrowList(records);
                printf("感谢使用,再见！\n");
                return 0;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
    freeList(list);
    freeBorrowList(records);
    printf("感谢使用,再见！\n");
    return 0;
}
