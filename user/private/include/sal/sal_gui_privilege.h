#ifndef _SAL_GUI_PRIVILEGE_H_
#define _SAL_GUI_PRIVILEGE_H_

enum
{
    PAGE_TYPE_ROOT = 0,
    PAGE_TYPE_MENU,
    PAGE_TYPE_HIDDEN
};


typedef enum
{
    ADMIN_ACCOUNT = 0,
    SUPPORT_ACCOUNT,
    USER_ACCOUNT,
    MAX_USER_TYPE
} USER_ACCOUNT_TYPE;

enum
{
    PAGE_PRIV_NONE = 0,
    PAGE_PRIV_READ,
    PAGE_PRIV_WRITE
};

typedef enum
{
    GUI_ACCESS_NONE   = 0x01,
    GUI_ACCESS_BASIC  = 0x02,
    GUI_ACCESS_EXPERT = 0x04,
    GUI_ACCESS_ADMIN  = 0x08,
    GUI_ACCESS_RSTPWD = 0x10
} GUI_ACCESS_MASK;
typedef struct
{
    int userType;
    int priv;
} priv_info;

#define PAGE_NAME_MAX 256
typedef struct page_info
{
    int id;
    char name[PAGE_NAME_MAX];
    priv_info privilege[MAX_USER_TYPE];
}page_info_tab;


int sal_page_privilege_check(const char *user, const char *page);

#endif

