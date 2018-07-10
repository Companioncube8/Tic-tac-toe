typedef struct list list;

struct list {
  int fd;
  char name[30];
  struct list *ptr;
};

struct list * init(int a, char name[]);
struct list * addelem(list *lst, int number, char name[]);
struct list  * deletelem(list *lst, list *root);
struct list * deletehead(list *root);
void listprint(list *lst);
struct list * listfind(list *lst, char *name);
