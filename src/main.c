/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: uli
 *
 * Created on 28. Juli 2017, 15:10
 */
#include "mail-file.h"
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>

#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>



#include <errno.h>
#include <stdarg.h>

#include <openssl/ssl.h>
#define UNUSED(x) (void)(x)


#define MESSAGE_TEXT "Hallo Test 9"
#define HOST "smtp.googlemail.com"
#define FROM "uhansen01@googlemail.com"
#define SUBJECT "Test mail 9"
#define RECIPIENT "info@familie-hansen.name"






int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    sendMail(HOST, FROM, RECIPIENT, SUBJECT, MESSAGE_TEXT);
    exit(0);
}




