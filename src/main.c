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





enum { TO = 10, CC, BCC, };

struct option longopts[] = {
    { "help", no_argument, NULL, '?',},
    { "version", no_argument, NULL, 'v',},
    { "host", required_argument, NULL, 'h',},
    { "monitor", no_argument, NULL, 'm',},
    { "crlf", no_argument, NULL, 'c',},
    { "notify", required_argument, NULL, 'n',},
    { "mdn", no_argument, NULL, 'd',},
    { "subject", required_argument, NULL, 's',},
    { "reverse-path", required_argument, NULL, 'f',},
    { "tls", no_argument, NULL, 't',},
    { "require-tls", no_argument, NULL, 'T',},
    { "noauth", no_argument, NULL, 1,},

    { "to", required_argument, NULL, TO,},
    { "cc", required_argument, NULL, CC,},
    { "bcc", required_argument, NULL, BCC,},

    { NULL, 0, NULL, 0,},
};

int
main(int argc, char **argv) {
    smtp_session_t session;
    smtp_message_t message;
    smtp_recipient_t recipient;
    auth_context_t authctx;
    const smtp_status_t *status;
    struct sigaction sa;
    char *host = NULL;
    char *from = NULL;
    char *subject = NULL;
    int nocrlf = 0;
    int noauth = 0;
    int to_cc_bcc = 0;
    char *file;
    FILE *fp;
    int c;
    enum notify_flags notify = Notify_NOTSET;

    printf("\nDEBUG:::::mail-file:main");

    /* This program sends only one message at a time.  Create an SMTP
       session and add a message to it. */
    auth_client_init();
    session = smtp_create_session();
    message = smtp_add_message(session);

    while ((c = getopt_long(argc, argv, "dmch:f:s:n:tTv",
            longopts, NULL)) != EOF)
        switch (c) {
            case 'h':
                host = optarg;
                break;

            case 'f':
                from = optarg;
                break;

            case 's':
                subject = optarg;
                break;

            case 'c':
                nocrlf = 1;
                break;

            case 'm':
                smtp_set_monitorcb(session, monitor_cb, stdout, 1);
                break;

            case 'n':
                if (strcmp(optarg, "success") == 0)
                    notify |= Notify_SUCCESS;
                else if (strcmp(optarg, "failure") == 0)
                    notify |= Notify_FAILURE;
                else if (strcmp(optarg, "delay") == 0)
                    notify |= Notify_DELAY;
                else if (strcmp(optarg, "never") == 0)
                    notify = Notify_NEVER;
                break;

            case 'd':
                /* Request MDN sent to the same address as the reverse path */
                smtp_set_header(message, "Disposition-Notification-To", NULL, NULL);
                break;

            case 't':
                smtp_starttls_enable(session, Starttls_ENABLED);
                break;

            case 'T':
                smtp_starttls_enable(session, Starttls_REQUIRED);
                break;

            case 'v':
                version();
                exit(2);

            case 1:
                noauth = 1;
                break;

            case TO:
                smtp_set_header(message, "To", NULL, optarg);
                to_cc_bcc = 1;
                break;
            case CC:
                smtp_set_header(message, "Cc", NULL, optarg);
                to_cc_bcc = 1;
                break;
            case BCC:
                smtp_set_header(message, "Bcc", NULL, optarg);
                to_cc_bcc = 1;
                break;

            default:
                usage();
                exit(2);
        }

    /* At least two more arguments are needed.
     */
    if (optind > argc - 2) {
        usage();
        exit(2);
    }

    printf("\nDEBUG:::::mail-file:main");
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
    smtp_set_server(session, host ? host : "localhost:25");

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
    smtp_set_reverse_path(message, from);

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
    if (subject != NULL) {
        smtp_set_header(message, "Subject", subject);
        smtp_set_header_option(message, "Subject", Hdr_OVERRIDE, 1);
    }

    /* Open the message file and set the callback to read it.
     */
    file = argv[optind++];
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

    /* Add remaining program arguments as message recipients.
     */
    while (optind < argc) {
        recipient = smtp_add_recipient(message, argv[optind++]);

        /* Recipient options set here */
        if (notify != Notify_NOTSET)
            smtp_dsn_set_notify(recipient, notify);
    }

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
        printf("%d %s", status->code,
                (status->text != NULL) ? status->text : "\n");
        smtp_enumerate_recipients(message, print_recipient_status, NULL);
    }

    /* Free resources consumed by the program.
     */
    smtp_destroy_session(session);
    auth_destroy_context(authctx);
    fclose(fp);
    auth_client_exit();
    exit(0);
}

