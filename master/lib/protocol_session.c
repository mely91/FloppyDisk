/******************************************************************************
* Copyright (C) 2011 by Jonathan Appavoo, Boston University
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "net.h"
#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_session.h"

#define FIXME

extern void
proto_session_dump(Proto_Session *s)
{ printf("inside proto_session_dump\n");
  fprintf(stderr, "Session s=%p:\n", s);
  fprintf(stderr, " fd=%d, extra=%p slen=%d, rlen=%d\n shdr:\n  ", 
	  s->fd, s->extra,
	  s->slen, s->rlen);
  proto_dump_msghdr(&(s->shdr));
  fprintf(stderr, " rhdr:\n  ");
  proto_dump_msghdr(&(s->rhdr));
}

extern void
proto_session_init(Proto_Session *s)
{ printf("inside proto_session_init\n");
  if (s) bzero(s, sizeof(Proto_Session));
}

extern void
proto_session_reset_send(Proto_Session *s)
{ printf("inside proto_session_reset_send\n");
  bzero(&s->shdr, sizeof(s->shdr));
  s->slen = 0;
}

extern void
proto_session_reset_receive(Proto_Session *s)
{printf("inside proto_session_reset_receive\n");
  bzero(&s->rhdr, sizeof(s->rhdr));
  s->rlen = 0;
}

static void
proto_session_hdr_marshall_sver(Proto_Session *s, Proto_StateVersion v)
{
    printf(" inside proto_session_hdr_marshall_sver\n");
  s->shdr.sver.raw = htonll(v.raw);
}

static void
proto_session_hdr_unmarshall_sver(Proto_Session *s, Proto_StateVersion *v)
{
    printf(" inside proto_session_hdr_unmarshall_sver\n");
  v->raw = ntohll(s->rhdr.sver.raw);
}

static void
proto_session_hdr_marshall_pstate(Proto_Session *s, Proto_Player_State *ps)
{
    printf(" inside proto_session_hdr_marshall_pstate\n");
  s->shdr.pstate.v0.raw  = htonl(ps->v0.raw);
  s->shdr.pstate.v1.raw  = htonl(ps->v1.raw);
  s->shdr.pstate.v2.raw  = htonl(ps->v2.raw);
  s->shdr.pstate.v3.raw  = htonl(ps->v3.raw);


}

static void
proto_session_hdr_unmarshall_pstate(Proto_Session *s, Proto_Player_State *ps)
{printf("inside proto_session_hdr_unmarshall_pstate\n");
 
  s->rhdr.pstate.v0.raw = ntohl(ps->v0.raw);
  s->rhdr.pstate.v0.raw = ntohl(ps->v1.raw);
  s->rhdr.pstate.v0.raw = ntohl(ps->v2.raw);
  s->rhdr.pstate.v0.raw = ntohl(ps->v3.raw);

  //ps->v0.raw  = ntohl(s->rhdr.pstate.v0.raw);
  //ps->v1.raw  = ntohl(s->rhdr.pstate.v1.raw);
  //ps->v2.raw  = ntohl(s->rhdr.pstate.v2.raw);
  //ps->v3.raw  = ntohl(s->rhdr.pstate.v3.raw);

}

static void
proto_session_hdr_marshall_gstate(Proto_Session *s, Proto_Game_State *gs)
{
    printf(" inside proto_session_hdr_marshall_gstate\n");
 s->shdr.gstate.v0.raw = htonl(gs->v0.raw);
 s->shdr.gstate.v1.raw = htonl(gs->v1.raw);
 s->shdr.gstate.v2.raw = htonl(gs->v2.raw);
 
}

static void
proto_session_hdr_unmarshall_gstate(Proto_Session *s, Proto_Game_State *gs)
{  
    printf("inside proto_session_hdr_unmarshall_gstate\n");
  s->rhdr.gstate.v0.raw = ntohl(gs->v0.raw);
  s->rhdr.gstate.v1.raw = ntohl(gs->v1.raw);
  s->rhdr.gstate.v2.raw = ntohl(gs->v2.raw);


  //gs->v0.raw = ntohl(s->rhdr.gstate.v0.raw);
  //gs->v1.raw = ntohl(s->rhdr.gstate.v1.raw);
  //gs->v2.raw = ntohl(s->rhdr.gstate.v2.raw);
}

static int
proto_session_hdr_unmarshall_blen(Proto_Session *s)
{
    printf(" inside proto_session_hdr_unmarshall_blen\n");
  ntohl(s->shdr.blen);
  return 0;
}

static void
proto_session_hdr_marshall_type(Proto_Session *s, Proto_Msg_Types t)
{
    printf("inside proto_session_hdr_marshall_type\n");
  s->shdr.type = htonl(t);

}

static int
proto_session_hdr_unmarshall_version(Proto_Session *s)
{
  printf("inside proto_session_hdr_unmarshall_version\n");
  ntohl(s->shdr.version);
  return 0;
}

extern Proto_Msg_Types
proto_session_hdr_unmarshall_type(Proto_Session *s)
{
    printf("inside proto_session_hdr_unmarshall_type\n");
    return ntohl(s->rhdr.type);
}

extern void
proto_session_hdr_unmarshall(Proto_Session *s, Proto_Msg_Hdr *h)
{
    printf("inside proto_session_hdr_unmarshall\n ");
  h->version = proto_session_hdr_unmarshall_version(s);
  h->type = proto_session_hdr_unmarshall_type(s);
  proto_session_hdr_unmarshall_sver(s, &h->sver);
  proto_session_hdr_unmarshall_pstate(s, &h->pstate);
  proto_session_hdr_unmarshall_gstate(s, &h->gstate);
  h->blen = proto_session_hdr_unmarshall_blen(s);
}
   
extern void
proto_session_hdr_marshall(Proto_Session *s, Proto_Msg_Hdr *h)
{
    printf("inside proto_session_hdr_marshall\n");
  // ignore the version number and hard code to the version we support
  s->shdr.version = PROTOCOL_BASE_VERSION;
  proto_session_hdr_marshall_type(s, h->type);
  proto_session_hdr_marshall_sver(s, h->sver);
  proto_session_hdr_marshall_pstate(s, &h->pstate);
  proto_session_hdr_marshall_gstate(s, &h->gstate);
  // we ignore the body length as we will explicity set it
  // on the send path to the amount of body data that was
  // marshalled.
}

extern int 
proto_session_body_marshall_ll(Proto_Session *s, long long v)
{
    printf(" inside proto_session_body_marshall_ll\n");
  if (s && ((s->slen + sizeof(long long)) < PROTO_SESSION_BUF_SIZE)) {
    *((int *)(s->sbuf + s->slen)) = htonll(v);
    s->slen+=sizeof(long long);
      printf("succesfull exit from proto_session_body_marshall_ll\n");
    return 1;
  }
    printf("error proto_session_body_marshall_ll\n");
  return -1;
}

extern int 
proto_session_body_unmarshall_ll(Proto_Session *s, int offset, long long *v)
{
    printf("inside proto_session_body_unmarshall_ll\n");
    if (s && ((s->rlen - (offset + (int)sizeof(long long))) >=0 )) {
    *v = *((long long *)(s->rbuf + offset));
    *v = htonl(*v);
      printf("succesfull exit proto_session_body_unmarshall_ll\n");
    return offset + sizeof(long long);
  }
    printf("error proto_session_body_unmarshall_ll\n");
  return -1;
}

extern int 
proto_session_body_marshall_int(Proto_Session *s, int v)
{printf("inside proto_session_body_marshall_int\n");
  if (s && ((s->slen + sizeof(int)) < PROTO_SESSION_BUF_SIZE)) {
    *((int *)(s->sbuf + s->slen)) = htonl(v);
    s->slen+=sizeof(int);
      printf(" sucess proto_session_body_marshall_int\n");
    return 1;
  }
    printf("error proto_session_body_marshall_int\n");
  return -1;
}

extern int 
proto_session_body_unmarshall_int(Proto_Session *s, int offset, int *v)
{printf("inside proto_session_body_unmarshall_int\n");
  if (s && ((s->rlen  - (offset + (int)sizeof(int))) >=0 )) {
    *v = *((int *)(s->rbuf + offset));
    *v = htonl(*v);
      printf("sucess proto_session_body_unmarshall_int\n");
    return offset + sizeof(int);
  }
    printf("error proto_session_body_unmarshall_int\n");
  return -1;
}

extern int 
proto_session_body_marshall_char(Proto_Session *s, char v)
{printf("inside proto_session_body_marshall_char\n");
  if (s && ((s->slen + sizeof(char)) < PROTO_SESSION_BUF_SIZE)) {
    s->sbuf[s->slen] = v;
    s->slen+=sizeof(char);
      printf("sucess proto_session_body_marshall_char\n");
    return 1;
  }
    printf("error proto_session_body_marshall_char\n");
  return -1;
}

extern int 
proto_session_body_unmarshall_char(Proto_Session *s, int offset, char *v)
{printf("inside proto_session_body_unmarshall_char\n");
  if (s && ((s->rlen - (offset + (int)sizeof(char))) >= 0)) {
    *v = s->rbuf[offset];
      printf("sucess proto_session_body_unmarshall_char\n");
    return offset + sizeof(char);
  }
    printf("error proto_session_body_unmarshall_char\n");
  return -1;
}

extern int
proto_session_body_reserve_space(Proto_Session *s, int num, char **space)
{printf("inside proto_session_body_reserve_space\n");
  if (s && ((s->slen + num) < PROTO_SESSION_BUF_SIZE)) {
    *space = &(s->sbuf[s->slen]);
    s->slen += num;
      printf("sucess proto_session_body_reserve_space\n");
    return 1;
  }
  *space = NULL;
    printf("error proto_session_body_reserve_space\n");
  return -1;
}

extern int
proto_session_body_ptr(Proto_Session *s, int offset, char **ptr)
{ printf("inside proto_session_body_ptr\n");
  if (s && ((s->rlen - offset) > 0)) {
    *ptr = &(s->rbuf[offset]);
      printf("success proto_session_body_ptr\n");
      return 1;
  }
    printf("error proto_session_body_ptr\n");
  return -1;
}
	    
extern int
proto_session_body_marshall_bytes(Proto_Session *s, int len, char *data)
{ printf("inside proto_session_body_marshall_bytes\n");
  if (s && ((s->slen + len) < PROTO_SESSION_BUF_SIZE)) {
    memcpy(s->sbuf + s->slen, data, len);
    s->slen += len;
      printf("sucess proto_session_body_marshall_bytes\n");
    return 1;
  }
    printf("error proto_session_body_marshall_bytes\n");
  return -1;
}

extern int
proto_session_body_unmarshall_bytes(Proto_Session *s, int offset, int len, 
				     char *data)
{printf("inside proto_session_body_unmarshall_bytes\n");
  if (s && ((s->rlen - (offset + len)) >= 0)) {
    memcpy(data, s->rbuf + offset, len);
      printf("sucess proto_session_body_unmarshall_bytes\n");
    return offset + len;
  }
    printf("error proto_session_body_unmarshall_bytes\n");
  return -1;
}

// rc < 0 on comm failures
// rc == 1 indicates comm success
extern  int
proto_session_send_msg(Proto_Session *s, int reset)
{printf("inside proto_session_send_msg\n");
  s->shdr.blen = htonl(s->slen);

  // write request
  net_writen(s->fd, s->sbuf, s->slen);
  if (proto_debug()) {
    fprintf(stderr, "%p: proto_session_send_msg: SENT:\n", pthread_self());
    proto_session_dump(s);
  }

  // communication was successfull 
  if (reset) proto_session_reset_send(s);
    printf("sucess proto_session_send_msg\n");
  return 1;
}

extern int
proto_session_rcv_msg(Proto_Session *s)
{
    printf("inside proto_session_rcv_msg\n");
  proto_session_reset_receive(s);

  // read reply
  net_readn(s->fd, s->rbuf, s->rlen);
    //proto_session_unmarshall HERERERERERE
  if (proto_debug()) {
    fprintf(stderr, "%p: proto_session_rcv_msg: RCVED:\n", pthread_self());
    proto_session_dump(s);
  }
    printf("sucess proto_session_rcv_msg\n");
  return 1;
}

extern int
proto_session_rpc(Proto_Session *s)
{  printf("inside proto_session_rpc\n");
  int rc;

  rc = proto_session_send_msg(s, 1);
    printf("inside proto_session_rpc after rc = proto_session_send_msg(s, 1\n");
	if (rc == 1) {
		proto_session_rcv_msg(s);
            printf("inside proto_session_rpc after proto_session_rcv_msg(s)\n");
	}
    printf("proto_session_rpc line before rc is returned\n");
  return rc;
}
