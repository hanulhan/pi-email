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
#define MAIL_HEADER_1 "Return-Path: <"
#define MAIL_HEADER_2 ">\nMIME-Version: 1.0\nContent-Type: text/plain;\n  charset=iso-8859-1\nContent-Transfer-Encoding: 7bit\n\n"

#define MESSAGE_TEXT "Hallo Test 8"
#define HOST "smtp.googlemail.com"
#define FROM "uhansen01@googlemail.com"
#define SUBJECT "Test mail 8"
#define RECIPIENT "info@familie-hansen.name"

#define NO_CRLF  1




void sendMail(const char *sHost, const char *sFrom, const char *sRecipient, const char *sSubject, const char *sMsgText);

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    sendMail(HOST, FROM, RECIPIENT, SUBJECT, MESSAGE_TEXT);
    exit(0);
}

void sendMail(const char *sHost, const char *sFrom, const char *sRecipient, const char *sSubject, const char *sMsgText) {
    smtp_session_t session;
    smtp_message_t message;
    smtp_recipient_t recipient;
    auth_context_t authctx;
    const smtp_status_t *status;
    struct sigaction sa;
    int nocrlf = 0;
    int noauth = 0;
    int to_cc_bcc = 0;
    char *file;
    FILE *fp;
    enum notify_flags notify = Notify_NOTSET;
    char *sMessage = NULL;

    printf("\n#################################################################");
    printf("\nDEBUG::main:sendMail()--> Start");
    printf("\n#################################################################");
    file = "MessageText.txt";


    // Calculate Buffer for MessageText
    size_t bufLen = strlen(MAIL_HEADER_1) + strlen(MAIL_HEADER_2) + strlen(sRecipient) + strlen(sMsgText);
    printf("\nRequired Buffer len: %d", (int) bufLen);

    sMessage = (char *) malloc((int) bufLen);

    if (sMessage == 0) {
        printf("\nError allocating memory for sMessage");
        exit(1);
    }

    strcpy((char *) sMessage, (const char *) MAIL_HEADER_1);
    strcat((char *) sMessage, (const char *) sRecipient);
    strcat((char *) sMessage, (const char *) MAIL_HEADER_2);
    strcat((char *) sMessage, (const char *) sMsgText);


    /* This program sends only one message at a time.  Create an SMTP
       session and add a message to it. */
    auth_client_init();
    session = smtp_create_session();
    message = smtp_add_message(session);

    nocrlf = NO_CRLF;
    printf("\nDEBUG::main:main()--> smtp_set_monitorcb");
    smtp_set_monitorcb(session, monitor_cb, stdout, 1);

#if 0
    //  Option -n
    if (strcmp(optarg, "success") == 0)
        notify |= Notify_SUCCESS;
    else if (strcmp(optarg, "failure") == 0)
        notify |= Notify_FAILURE;
    else if (strcmp(optarg, "delay") == 0)
        notify |= Notify_DELAY;
    else if (strcmp(optarg, "never") == 0)
        notify = Notify_NEVER;
#endif
#if 0
    /* Request MDN sent to the same address as the reverse path */
    // Option -d
    printf("\nDEBUG::main:main()--> smtp_set_header");
    smtp_set_header(message, "Disposition-Notification-To", NULL, NULL);
#endif


#if 1
    // Option -t
    printf("\nDEBUG::main:main()--> smtp_starttls_enable");
    smtp_starttls_enable(session, Starttls_ENABLED);
#endif

#if 0
    // Option -Tase 'T':
    printf("\nDEBUG::main:main()--> smtp_starttls_enable");
    smtp_starttls_enable(session, Starttls_REQUIRED);
#endif

#if 0
    // Option -l
    noauth = 1;
#endif

#if 0
    //    Option -TO
    printf("\nDEBUG::main:main()--> smtp_set_header(To: %s)", optarg);
    smtp_set_header(message, "To", NULL, optarg);
    to_cc_bcc = 1;
#endif

#if 0
    // Option -CC
    printf("\nDEBUG::main:main()--> smtp_set_header(Cc: %s)", optarg);
    smtp_set_header(message, "Cc", NULL, optarg);
    to_cc_bcc = 1;
#endif

#if 0
    // Option BCC
    printf("\nDEBUG::main:main()--> smtp_set_header(Bcc: %s)", optarg);
    smtp_set_header(message, "Bcc", NULL, optarg);
    to_cc_bcc = 1;
#endif




    /* NB.  libESMTP sets timeouts as it progresses through the protocol.
       In addition the remote server might close its socket on a timeout.
       Consequently libESMTP may sometimes try to write to a socket with
       no reader.  Ignore SIGPIPE, then the program doesn't get killed
       if/when this happens. */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);

    /* Set the host running the SMTP server.  LibESMTP has a default port
       number of 587, however this is not widely deployed so the port
       is specified as 25 along with the default MTA host. */
    printf("\nDEBUG::main:main()--> smtp_set_server(%s)", sHost ? sHost : "localhost:25");
    smtp_set_server(session, sHost ? sHost : "localhost:25");

    /* Do what's needed at application level to use authentication.
     */
    authctx = auth_create_context();
    auth_set_mechanism_flags(authctx, AUTH_PLUGIN_PLAIN, 0);
    auth_set_interact_cb(authctx, authinteract, NULL);

    /* Use our callback for X.509 certificate passwords.  If STARTTLS is
       not in use or disabled in configure, the following is harmless. */
    smtp_starttls_set_password_cb(tlsinteract, NULL);
    smtp_set_eventcb(session, event_cb, NULL);

    /* Now tell libESMTP it can use the SMTP AUTH extension.
     */
    if (!noauth)
        smtp_auth_set_context(session, authctx);

    /* Set the reverse path for the mail envelope.  (NULL is ok)
     */
    smtp_set_reverse_path(message, sFrom);

#if 0
    /* The message-id is OPTIONAL but SHOULD be present.  By default
       libESMTP supplies one.  If this is not desirable, the following
       prevents one making its way to the server.
       N.B. It is not possible to prohibit REQUIRED headers.  Furthermore,
            the submission server will probably add a Message-ID header,
            so this cannot prevent the delivered message from containing
            the message-id.  */
    smtp_set_header_option(message, "Message-Id", Hdr_PROHIBIT, 1);
#endif

    /* RFC 2822 doesn't require recipient headers but a To: header would
       be nice to have if not present. */
    if (!to_cc_bcc)
        smtp_set_header(message, "To", NULL, NULL);

    /* Set the Subject: header.  For no reason, we want the supplied subject
       to override any subject line in the message headers. */
    if (sSubject != NULL) {
        smtp_set_header(message, "Subject", sSubject);
        smtp_set_header_option(message, "Subject", Hdr_OVERRIDE, 1);
    }

    /* Open the message file and set the callback to read it.
     */

    printf("\nDEBUG::main:main()--> Create a file");
    fp = fopen(file, "w");

    fprintf(fp, sMsgText);
    fclose(fp);


    if (strcmp(file, "-") == 0)
        fp = stdin;
    else if ((fp = fopen(file, "r")) == NULL) {
        fprintf(stderr, "can't open %s: %s\n", file, strerror(errno));
        exit(1);
    }
    if (nocrlf)
        smtp_set_messagecb(message, readlinefp_cb, fp);
    else
        smtp_set_message_fp(message, fp);


    recipient = smtp_add_recipient(message, (const char *) sRecipient);

    /* Recipient options set here */
    if (notify != Notify_NOTSET)
        smtp_dsn_set_notify(recipient, notify);

    /* Initiate a connection to the SMTP server and transfer the
       message. */
    if (!smtp_start_session(session)) {
        char buf[128];

        fprintf(stderr, "SMTP server problem %s\n",
                smtp_strerror(smtp_errno(), buf, sizeof buf));
    } else {
        /* Report on the success or otherwise of the mail transfer.
         */
        status = smtp_message_transfer_status(message);
        printf("\nDEBUG::main:main()--> %d %s", status->code,
                (status->text != NULL) ? status->text : "\n");
        smtp_enumerate_recipients(message, print_recipient_status, NULL);
    }

    /* Free resources consumed by the program.
     */
    smtp_destroy_session(session);
    auth_destroy_context(authctx);
    fclose(fp);
    auth_client_exit();
    free(sMessage);
}



