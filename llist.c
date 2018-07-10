#include "llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct list * init(int a, char name[]) {
  struct list *lst;
  lst = (struct list*)malloc(sizeof(struct list));
  lst->fd = a;
  strcpy(lst->name, name);
  lst->ptr = NULL; 
  return(lst);
}

struct list * addelem(list *lst, int number, char name[]) {
  struct list *temp, *p;
  temp = (struct list*)malloc(sizeof(list));
  p = lst->ptr;  
  lst->ptr = temp;  
  temp->fd = number;  
  strcpy(temp->name, name);
  temp->ptr = p;  
  return(temp);
}

struct list  * deletelem(list *lst, list *root) {
  struct list *temp;
  temp = root;
  while(temp->ptr!=lst)  // просматриваем список начиная с корня
    {    // пока не найдем узел, предшествующий lst
      temp = temp->ptr;
    }
  temp->ptr = lst->ptr; // переставляем указатель
  free(lst);  // освобождаем память удаляемого узла
  return(temp);
}

struct list * deletehead(list *root) {
  struct list *temp;
  temp = root->ptr;
  free(root);   // освобождение памяти текущего корня
  return(temp); // новый корень списка
}

void listprint(list *lst) {
  struct list *p;
  p = lst;
  do  {
      printf("%d ",p->fd); // вывод значения элемента p
      printf("%s ",p->name); // вывод значения элемента p
      p = p->ptr; // переход к следующему узлу
    }while(p != NULL);
}
struct list * listfind(list *lst,char *name) {
  struct list *p;
  p = lst;
  do {
	  if (!strcmp(name, p->name)) {
		return(p);
	  }
      p = p->ptr;
  } while(p != NULL);
  
  return(NULL);
}
