/*
 *  A libESMTP Example Application.
 *  Copyright (C) 2001,2002  Brian Stafford <brian@stafford.uklinux.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* This program accepts a single file argument followed by a list of
   recipients.  The file is mailed to each of the recipients.

   Error checking is minimal to non-existent, this is just a quick
   and dirty program to give a feel for using libESMTP.
 */


#define UNUSED(x) (void)(x)
#include "mail-file.h"


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>

#include <openssl/ssl.h>

#if !defined (__GNUC__) || __GNUC__ < 2
# define __attribute__(x)
#endif
#define unused      __attribute__((unused))





/* Callback to prnt the recipient status */
void
print_recipient_status (smtp_recipient_t recipient,
			const char *mailbox, void *arg unused)
{
  const smtp_status_t *status;
  printf("\nDEBUG::mail-file:print_recipient_status() --> Start");

  status = smtp_recipient_status (recipient);
  printf("\nDEBUG::mail-file:print_recipient_status() --> %s: %d %s", mailbox, status->code, status->text);
}

/* Callback function to read the message from a file.  Since libESMTP
   does not provide callbacks which translate line endings, one must
   be provided by the application.

   The message is read a line at a time and the newlines converted
   to \r\n.  Unfortunately, RFC 822 states that bare \n and \r are
   acceptable in messages and that individually they do not constitute a
   line termination.  This requirement cannot be reconciled with storing
   messages with Unix line terminations.  RFC 2822 rescues this situation
   slightly by prohibiting lone \r and \n in messages.

   The following code cannot therefore work correctly in all situations.
   Furthermore it is very inefficient since it must search for the \n.
 */
#define BUFLEN	8192

const char *
readlinefp_cb (void **buf, int *len, void *arg)
{
  int octets;

  printf("\nDEBUG::mail-file:readlinefp_cb() --> Start");
  if (*buf == NULL)
    *buf = malloc (BUFLEN);

  if (len == NULL)
    {
      rewind ((FILE *) arg);
      return NULL;
    }

  if (fgets (*buf, BUFLEN - 2, (FILE *) arg) == NULL)
    octets = 0;
  else
    {
      char *p = strchr (*buf, '\0');

      if (p[-1] == '\n' && p[-2] != '\r')
        {
	  strcpy (p - 1, "\r\n");
	  p++;
        }
      octets = p - (char *) *buf;
    }
  *len = octets;
  printf("\nDEBUG::mail-file:readlinefp_cb() --> buf: %s", (char *)*buf);
  return *buf;
}

void
monitor_cb (const char *buf, int buflen, int writing, void *arg)
{
  FILE *fp = arg;

  printf("\nDEBUG::mail-file:monitor_cb() --> Start");

  if (writing == SMTP_CB_HEADERS)
    {
      fputs ("H: ", fp);
      fwrite (buf, 1, buflen, fp);
      return;
    }

 fputs (writing ? "\nC: " : "\nS: ", fp);
 fwrite (buf, 1, buflen, fp);
 if (buf[buflen - 1] != '\n')
   putc ('\n', fp);
}

/* Callback to request user/password info.  Not thread safe. */
//int
//authinteract (auth_client_request_t request, char **result, int fields,
//              void *arg unused)
//{
//  char prompt[64];
//  static char resp[512];
//  char *p, *rp;
//  int i, n, tty;
//  printf("\nDEBUG:::::mail-file:authinteract()");
//  rp = resp;
//  printf("\nDEBUG:::::mail-file:authinteract() Anzahl fields: %d", fields);
//  for (i = 0; i < fields; i++)
//    {
//      printf("\nDEBUG:::::mail-file:authinteract() request[%d].prompt: %s", i, request[i].prompt);
//      printf("\nDEBUG:::::mail-file:authinteract() request[%d].flags: %u", i, request[i].flags);
//      n = snprintf (prompt, sizeof prompt, "%s%s: ", request[i].prompt,
//		    (request[i].flags & AUTH_CLEARTEXT) ? " (not encrypted)"
//		    					: "");
//      if (request[i].flags & AUTH_PASS)
//	result[i] = getpass (prompt);
//      else
//	{
//	  tty = open ("/dev/tty", O_RDWR);
//	  write (tty, prompt, n);
//	  n = read (tty, rp, sizeof resp - (rp - resp));
//	  close (tty);
//	  p = rp + n;
//	  while (isspace (p[-1]))
//	    p--;
//	  *p++ = '\0';
//	  result[i] = rp;
//	  rp = p;
//	}
//      
//        printf("\nDEBUG:::::mail-file:authinteract() result[%d]: %s", i, result[i]);
//    }
//  return 1;
//}

int
authinteract (auth_client_request_t request, char **result, int fields,
              void *arg unused)
{
  UNUSED(request);
  UNUSED(fields);
  printf("\nDEBUG::mail-file:authinteract() --> Start");  

  result[0] = "uhansen01";
  result[1] = "ava030374lon_";
  

  return 1;
}

int
tlsinteract (char *buf, int buflen, int rwflag unused, void *arg unused)
{
  char *pw;
  int len;

  printf("\nDEBUG::mail-file:tlsinteract() --> Start");
  pw = getpass ("certificate password");
  len = strlen (pw);
  if (len + 1 > buflen)
    return 0;
  strcpy (buf, pw);
  return len;
}
int
handle_invalid_peer_certificate(long vfy_result)
{
  const char *k ="rare error";
  printf("\nDEBUG::mail-file:handle_invalid_peer_certificate() --> Start");
  switch(vfy_result) {
  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    k="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT"; break;
  case X509_V_ERR_UNABLE_TO_GET_CRL:
    k="X509_V_ERR_UNABLE_TO_GET_CRL"; break;
  case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
    k="X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE"; break;
  case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
    k="X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE"; break;
  case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
    k="X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY"; break;
  case X509_V_ERR_CERT_SIGNATURE_FAILURE:
    k="X509_V_ERR_CERT_SIGNATURE_FAILURE"; break;
  case X509_V_ERR_CRL_SIGNATURE_FAILURE:
    k="X509_V_ERR_CRL_SIGNATURE_FAILURE"; break;
  case X509_V_ERR_CERT_NOT_YET_VALID:
    k="X509_V_ERR_CERT_NOT_YET_VALID"; break;
  case X509_V_ERR_CERT_HAS_EXPIRED:
    k="X509_V_ERR_CERT_HAS_EXPIRED"; break;
  case X509_V_ERR_CRL_NOT_YET_VALID:
    k="X509_V_ERR_CRL_NOT_YET_VALID"; break;
  case X509_V_ERR_CRL_HAS_EXPIRED:
    k="X509_V_ERR_CRL_HAS_EXPIRED"; break;
  case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
    k="X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD"; break;
  case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
    k="X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD"; break;
  case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
    k="X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD"; break;
  case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
    k="X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD"; break;
  case X509_V_ERR_OUT_OF_MEM:
    k="X509_V_ERR_OUT_OF_MEM"; break;
  case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    k="X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT"; break;
  case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    k="X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN"; break;
  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
    k="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY"; break;
  case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
    k="X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE"; break;
  case X509_V_ERR_CERT_CHAIN_TOO_LONG:
    k="X509_V_ERR_CERT_CHAIN_TOO_LONG"; break;
  case X509_V_ERR_CERT_REVOKED:
    k="X509_V_ERR_CERT_REVOKED"; break;
  case X509_V_ERR_INVALID_CA:
    k="X509_V_ERR_INVALID_CA"; break;
  case X509_V_ERR_PATH_LENGTH_EXCEEDED:
    k="X509_V_ERR_PATH_LENGTH_EXCEEDED"; break;
  case X509_V_ERR_INVALID_PURPOSE:
    k="X509_V_ERR_INVALID_PURPOSE"; break;
  case X509_V_ERR_CERT_UNTRUSTED:
    k="X509_V_ERR_CERT_UNTRUSTED"; break;
  case X509_V_ERR_CERT_REJECTED:
    k="X509_V_ERR_CERT_REJECTED"; break;
  }
  printf("\nDEBUG::mail-file:handle_invalid_peer_certificate() --> SMTP_EV_INVALID_PEER_CERTIFICATE: %ld: %s\n", vfy_result, k);
  return 1; /* Accept the problem */
}

void event_cb (smtp_session_t session, int event_no, void *arg,...)
{
  va_list alist;
  int *ok;
  UNUSED(session);
  va_start(alist, arg);
  switch(event_no) {
  case SMTP_EV_CONNECT:
      printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_CONNECT");
      break;
  case SMTP_EV_MAILSTATUS:
      printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_MAILSTATUS");
      break;
  case SMTP_EV_RCPTSTATUS:
      printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_RCPTSTATUS");
      break;      
  case SMTP_EV_MESSAGEDATA:
      printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_MESSAGEDATA");
      break;
  case SMTP_EV_MESSAGESENT:
      printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_MESSAGESENT");
      break;
  case SMTP_EV_DISCONNECT:
      printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_DISCONNECT");
      break;      
  
  case SMTP_EV_WEAK_CIPHER: {
    int bits;
    printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_WEAK_CIPHER, bits=%d - accepted.\n", bits);
    bits = va_arg(alist, long); ok = va_arg(alist, int*);
    *ok = 1; break;
  }
  case SMTP_EV_STARTTLS_OK:
    printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_STARTTLS_OK");
    puts("\nSMTP_EV_STARTTLS_OK - TLS started here."); break;
  case SMTP_EV_INVALID_PEER_CERTIFICATE: {
    long vfy_result;
    printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_INVALID_PEER_CERTIFICATE");
    vfy_result = va_arg(alist, long); ok = va_arg(alist, int*);
    *ok = handle_invalid_peer_certificate(vfy_result);
    break;
  }
  case SMTP_EV_NO_PEER_CERTIFICATE: {
    printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_NO_PEER_CERTIFICATE");
    ok = va_arg(alist, int*); 
    puts("\nSMTP_EV_NO_PEER_CERTIFICATE - accepted.");
    *ok = 1; break;
  }
  case SMTP_EV_WRONG_PEER_CERTIFICATE: {
    printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_WRONG_PEER_CERTIFICATE");
    ok = va_arg(alist, int*);
    puts("\nSMTP_EV_WRONG_PEER_CERTIFICATE - accepted.");
    *ok = 1; break;
  }
  case SMTP_EV_NO_CLIENT_CERTIFICATE: {
    printf("\nDEBUG::mail-file:event_cb() --> Event SMTP_EV_NO_CLIENT_CERTIFICATE");    
      ok = va_arg(alist, int*); 
    puts("\nSMTP_EV_NO_CLIENT_CERTIFICATE - accepted.");
    *ok = 1; break;
  }
  default:
    printf("\nDEBUG::mail-file:event_cb() --> Got event: %d - ignored.\n", event_no);
  }
  va_end(alist);
}

void
usage (void)
{
    printf("\nDEBUG::usage()");
    fputs ("Copyright (C) 2001  Brian Stafford <brian@stafford.uklinux.net>\n"
	 "\n"
	 "This program is free software; you can redistribute it and/or modify\n"
	 "it under the terms of the GNU General Public License as published\n"
	 "by the Free Software Foundation; either version 2 of the License,\n"
	 "or (at your option) any later version.\n"
	 "\n"
	 "This program is distributed in the hope that it will be useful,\n"
	 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	 "GNU General Public License for more details.\n"
	 "\n"
	 "You should have received a copy of the GNU General Public License\n"
	 "along with this program; if not, write to the Free Software\n"
	 "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
	 "\n"
         "usage: mail-file [options] file mailbox [mailbox ...]\n"
         "\t-h,--host=hostname[:service] -- set SMTP host and service (port)\n"
         "\t-f,--reverse-path=mailbox -- set reverse path\n"
         "\t-s,--subject=text -- set subject of the message\n"
         "\t-n,--notify=success|failure|delay|never -- request DSN\n"
         "\t-d,--mdn -- request MDN\n"
         "\t-m,--monitor -- watch the protocol session with the server\n"
         "\t-c,--crlf -- translate line endings from \\n to CR-LF\n"
         "\t-t,--tls -- use STARTTLS extension if possible\n"
         "\t-T,--require-tls -- require use of STARTTLS extension\n"
         "\t   --noauth -- do not attempt to authenticate to the MSA\n"
         "\t--version -- show version info and exit\n"
         "\t--help -- this message\n"
         "\n"
         "Specify the file argument as \"-\" to read standard input.\n"
         "The input must be in RFC 2822 format, that is, it must consist\n"
         "of a sequence of message headers terminated by a blank line and\n"
         "followed by the message body.  Lines must be terminated with the\n"
         "canonic CR-LF sequence unless the --crlf flag is specified.\n"
         "Total line length must not exceed 1000 characters.\n",
         stderr);
}

void
version (void)
{
  char buf[32];
  
  printf("\nDEBUG::version()");

  smtp_version (buf, sizeof buf, 0);
  printf ("\nlibESMTP version %s\n", buf);
}

