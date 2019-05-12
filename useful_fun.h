//Header globale
#include "myheader.h"

#ifndef USEFUL_FUN_H
#define  USEFUL_FUN_H

//Struct utili
typedef struct pair_int{
    int first;
    int second;
} pair_int;

typedef struct device{
    int id;
    int fd[2];
} device;

typedef struct int_node {
    int val;
    struct int_node * next;
} int_node;

typedef struct int_list{
    int_node * head;
    int n;
} int_list;

typedef struct pair_int_device{
    int integer;
    device * val;
} pair_int_device;

typedef struct device_node{
    device * val;
    struct device_node * next;
} device_node;

typedef struct device_list{
    device_node * head;
    int n;
} device_list;

//Dichiarazioni Funzioni
pair_int get_int(int n, int_list * list);
int_list * create_int_list();
pair_int_device get_device(int n, device_list * list);
device_list * create_device_list();
int insert_int(int val, int n, int_list * list);
int rm_int(int n, int_list * list);
int insert_device(device * val, int n, device_list * list);
int rm(int n, device_list * list);
void str_split(char * str, char ** rt);

#endif
