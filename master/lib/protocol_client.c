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
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "protocol.h"
#include "protocol_utils.h"
#include "protocol_client.h"

#define FIXME

typedef struct {
  Proto_Session rpc_session;
  Proto_Session event_session;
  pthread_t EventHandlerTid;
  Proto_MT_Handler session_lost_handler;
  Proto_MT_Handler base_event_handlers[PROTO_MT_EVENT_BASE_RESERVED_LAST 
				       - PROTO_MT_EVENT_BASE_RESERVED_FIRST
				       - 1];
} Proto_Client;

extern Proto_Session *
proto_client_rpc_session(Proto_Client_Handle ch)
{ printf("proto_client_rpc_session\n");
  Proto_Client *c = ch;
  return &(c->rpc_session);
}

extern Proto_Session *
proto_client_event_session(Proto_Client_Handle ch)
{
    printf("proto_client_event_session");
  Proto_Client *c = ch;
  return &(c->event_session);
}

extern int
proto_client_set_session_lost_handler(Proto_Client_Handle ch, Proto_MT_Handler h)
{ printf("proto_client_set_session_lost_handler\n");
  Proto_Client *c = ch;
  c->session_lost_handler = h;
  return 0;
}

extern int
proto_client_set_event_handler(Proto_Client_Handle ch, Proto_Msg_Types mt,
			       Proto_MT_Handler h)
{
    printf("inside proto_client_set_event_handler\n");
     int i;
  Proto_Client *c = ch;

  if (mt>PROTO_MT_EVENT_BASE_RESERVED_FIRST && 
      mt<PROTO_MT_EVENT_BASE_RESERVED_LAST) {
    i=mt - PROTO_MT_EVENT_BASE_RESERVED_FIRST - 1;
    c->base_event_handlers[i]=h;
      printf("succes proto_client_set_event_handler \n");
    return 1;
  } else {
      printf("returned -1 inside proto_client_set_event_handler \n");
    return -1;
  }
}

static int 
proto_client_session_lost_default_hdlr(Proto_Session *s)
{
  fprintf(stderr, "Session lost...:\n");
  proto_session_dump(s);
  return -1;
}

static int 
proto_client_event_null_handler(Proto_Session *s)
{
  fprintf(stderr, 
	  "proto_client_event_null_handler: invoked for session:\n");
  proto_session_dump(s);

  return 1;
}

static void *
proto_client_event_dispatcher(void * arg)
{printf("inside proto_clietnt_event_dispatcher");
  Proto_Client *c;
  Proto_Session *s;
  Proto_Msg_Types mt;
  Proto_MT_Handler hdlr;
  int i;

  pthread_detach(pthread_self());
  printf("Inside pthread_detach\n");
  c = arg;
  s = &c->event_session;


  for (;;) {
    if (proto_session_rcv_msg(s)==1) {
      mt = proto_session_hdr_unmarshall_type(s);
      if (mt > PROTO_MT_EVENT_BASE_RESERVED_FIRST && 
	  mt < PROTO_MT_EVENT_BASE_RESERVED_LAST) {
	i=mt - PROTO_MT_EVENT_BASE_RESERVED_FIRST - 1;
	  hdlr=c->base_event_handlers[i];

         
	if (hdlr(s)<0) goto leave;
      }
    } else {
    proto_client_session_lost_default_hdlr(s);   
      goto leave;
    }
  }
 leave:
  
  close(s->fd);
  return NULL;
}

extern int
proto_client_init(Proto_Client_Handle *ch)
{
  Proto_Msg_Types mt;
  Proto_Client *c;
 
  c = (Proto_Client *)malloc(sizeof(Proto_Client));
  if (c==NULL) return -1;
  bzero(c, sizeof(Proto_Client));

  proto_client_set_session_lost_handler(c, 
			      	proto_client_session_lost_default_hdlr);
  
  for (mt=PROTO_MT_EVENT_BASE_RESERVED_FIRST+1;
       mt<PROTO_MT_EVENT_BASE_RESERVED_LAST; mt++)
    proto_client_set_event_handler(c, mt , proto_client_event_null_handler);
    printf("succes inside proto_client_init\n");
    *ch = c;
  return 1;
}

int
proto_client_connect(Proto_Client_Handle ch, char *host, PortType port)
{
  Proto_Client *c = (Proto_Client *)ch;
  printf("tryies proto_client_connect to rpc\n");
  if (net_setup_connection(&(c->rpc_session.fd), host, port)<0) 
    return -1;
    printf("tryies proto_client_connect to event\n");

  if (net_setup_connection(&(c->event_session.fd), host, port+1)<0) 
    return -2; 

  if (pthread_create(&(c->EventHandlerTid), NULL, 
		     &proto_client_event_dispatcher, c) !=0) {
    fprintf(stderr, 
	    "proto_client_init: create EventHandler thread failed\n");
    perror("proto_client_init:");
    return -3;
  }

  return 0;
}

static void
marshall_mtonly(Proto_Session *s, Proto_Msg_Types mt) {
    printf(" inside marshall_mtonly \n");
Proto_Msg_Hdr h;
  bzero(&h, sizeof(h));
  h.type = mt;
  proto_session_hdr_marshall(s, &h);
   printf("exits marshall_mtonly \n"); 
};

// all rpc's are assume to only reply only with a return code in the body
// eg.  like the null_mes
static int 
do_generic_dummy_rpc(Proto_Client_Handle ch, Proto_Msg_Types mt)
    { printf("inside do_generic_rpc\n");
  int rc;
  Proto_Session *s;
  Proto_Client *c = ch;

       
  s = &c->rpc_session;
  // marshall

  marshall_mtonly(s, mt);
  rc = proto_session_rpc(s);

  if (rc==1) {
    proto_session_body_unmarshall_int(s, 0, &rc);
  } else {
    c->session_lost_handler(s);
    close(s->fd);
    // fprintf(stderr, "Error: Unable to do_generic (%d)\n", errno);

  }
  
  return rc;
}
extern int 
proto_client_hello(Proto_Client_Handle ch)
    {printf("inside proto_client_hello\n");
  return do_generic_dummy_rpc(ch,PROTO_MT_REQ_BASE_HELLO);  
}

extern int 
proto_client_move(Proto_Client_Handle ch, char data)
    { printf("inside proto_client_move\n");
  return do_generic_dummy_rpc(ch,PROTO_MT_REQ_BASE_MOVE);  
}

extern int 
proto_client_goodbye(Proto_Client_Handle ch)
    { printf("inside_proto_goodbye\n");
  return do_generic_dummy_rpc(ch,PROTO_MT_REQ_BASE_GOODBYE);  
}


