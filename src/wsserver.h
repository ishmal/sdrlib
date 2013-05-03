/**
 * This is a very simple HTTP server that can also handle Websockets
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2013 Bob Jamison
 * 
 *  This file is part of the SdrLib library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef _WSSERVER_H_
#define _WSSERVER_H_


typedef struct WsServer WsServer;

typedef struct ClientInfo ClientInfo;

#define CLIENT_BUFLEN 256
struct ClientInfo
{
    WsServer *server;
    int socket;
    void *context;
    char resourceName[CLIENT_BUFLEN];
    char buf[CLIENT_BUFLEN];
};

typedef void (WsHandlerFunc)(ClientInfo *);


/**
 *
 */
ClientInfo *infoCreate();

/**
 *
 */
void infoDelete(ClientInfo *obj);


/**
 *
 */
int wsSend(ClientInfo *info, char *str);


/**
 *
 */
int wsSendBinary(ClientInfo *info, unsigned char *dat, long len);



/**
 *
 */
int wsRecv(ClientInfo *info, unsigned char *dat, int len);



/**
 *
 */
WsServer *wsCreate(WsHandlerFunc *func, void * context, char *dir, int port);



/**
 *
 */
void wsDelete(WsServer *obj);



/**
 *
 */
int wsServe(WsServer *obj);




#endif  /* _WSSERVER_H_ */