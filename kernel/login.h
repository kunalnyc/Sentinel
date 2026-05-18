#ifndef LOGIN_H
#define LOGIN_H

#include <stdint.h>

// Login result
#define LOGIN_SUCCESS  0
#define LOGIN_FAIL    -1

// Run the login screen — returns LOGIN_SUCCESS when authenticated
int login_run(void);

#endif