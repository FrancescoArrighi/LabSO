//Header globale
#include "myheader.h"

#ifndef USEFUL_FUN_H
#define  USEFUL_FUN_H

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
