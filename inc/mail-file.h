/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mail-file.h
 * Author: uli
 *
 * Created on 28. Juli 2017, 14:46
 */

#ifndef MAIL_FILE_H
#define MAIL_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#define _XOPEN_SOURCE 500
#include <auth-client.h>
#include <libesmtp.h>

    #define NO_CRLF  1



    void sendMail(const char *sHost, const char *sFrom, const char *sRecipient, const char *sSubject, const char *sMsgText);
    const char *readlinefp_cb(void **buf, int *len, void *arg);
    void monitor_cb(const char *buf, int buflen, int writing, void *arg);
    void print_recipient_status(smtp_recipient_t recipient, const char *mailbox, void *arg);
    int authinteract(auth_client_request_t request, char **result, int fields, void *arg);
    int tlsinteract(char *buf, int buflen, int rwflag, void *arg);
    void event_cb(smtp_session_t session, int event_no, void *arg, ...);
    void usage(void);
    void version(void);



#ifdef __cplusplus
}
#endif

#endif /* MAIL_FILE_H */

