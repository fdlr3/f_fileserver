#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include "fdefines.h"
#include <stdio.h>

char*   get_tag     (const char* _conf_path, char* _dest, const char* _tag);
int     auth_user   (const char* _conf_path, const char* _id, const char* _hash);

#endif