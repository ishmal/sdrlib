/**
 * This is a very simple HTTP server that can also handle Websockets
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <errno.h>
#include <sys/stat.h>

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif


static void trace(char *fmt, ...)
{
    fprintf(stdout, "WsServer: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}


static void error(char *fmt, ...)
{
    fprintf(stderr, "WsServer err: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}




/*###################################################################################
##  B A S E    6 4
###################################################################################*/



static char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static void encode64(unsigned char *inbuf, char *outbuf, int len)
{
    char *out = outbuf;
    unsigned char *b = inbuf;
    while (len>0)
        {
        if (len >= 3)
            {
            int b0 = (int)*b++;
            int b1 = (int)*b++;
            int b2 = (int)*b++;
            *out++ = base64[ ((b0       ) >> 2)                      ];
            *out++ = base64[ ((b0 & 0x03) << 4) | ((b1 & 0xf0) >> 4) ];
            *out++ = base64[ ((b1 & 0x0f) << 2) | ((b2 & 0xc0) >> 6) ];
            *out++ = base64[ ((b2 & 0x3f)     )                      ];
            len -= 3;
            }
        else if (len == 2)
            {
            int b0 = (int)*b++;
            int b1 = (int)*b++;
            *out++ = base64[ ((b0 >> 2)       )                      ];
            *out++ = base64[ ((b0 & 0x03) << 4) | ((b1 & 0xf0) >> 4) ];
            *out++ = base64[ ((b1 & 0x0f) << 2)                      ];
            *out++ = '=';
            len -= 2;
            }
        else /* len == 1 */
            {
            int b0 = (int)*b++;
            *out++ = base64[ ((b0       ) >> 2)                      ];
            *out++ = base64[ ((b0 & 0x03) << 4)                      ];
            *out++ = '=';
            *out++ = '=';
            len -= 1;
            }
        }
    
    *out = '\0';
}


static int _dec64(int ch)
{
    int v;
    if (ch >= 'A' && ch <= 'Z')
        v = ch - 'A';
    else if (ch >= 'a' && ch <= 'z')
        v = ch - 'a' + 26;
    else if (ch >= '0' && ch <= '9')
        v = ch - '0' + 52;
    else if (ch == '+')
        v = 62;
    else if (ch == '/')
        v = 63;
    else
        {
        error("Bad base64 character: '%c'" , ch);
        v = -1;
        }
    return v;
}

static int decode64(char *inbuf, unsigned char *outbuf, int len)
{
    char *in = inbuf;
    unsigned char *out = outbuf;
    for (; len>0 ; len -= 4)
        {
        int v0 = _dec64(*in++);
        int v1 = _dec64(*in++);
        int v2 = _dec64(*in++);
        int v3 = _dec64(*in++);
        if (v0<0 || v1<0 || v2<0 || v3<0)
            return -1;
        *out++ = ((v0       ) << 2) | ((v1 & 0x30) >> 4);
        *out++ = ((v1 & 0x0f) << 4) | ((v2 & 0x3c) >> 2);
        *out++ = ((v2 & 0x03) << 6) | ((v3 & 0x3f)     );
        }
    return (out - outbuf);
}




/*###################################################################################
##  S H A   1
###################################################################################*/


#if 0
/**
 *
 */
void Sha1::reset()
{
    longNr    = 0;
    byteNr    = 0;

    // Initialize H with the magic constants (see FIPS180 for constants)
    hashBuf[0] = 0x67452301L;
    hashBuf[1] = 0xefcdab89L;
    hashBuf[2] = 0x98badcfeL;
    hashBuf[3] = 0x10325476L;
    hashBuf[4] = 0xc3d2e1f0L;

    for (int i = 0; i < 4; i++)
        inb[i] = 0;

    for (int i = 0; i < 80; i++)
        inBuf[i] = 0;

    clearByteCount();
}


/**
 *
 */
void Sha1::update(unsigned char ch)
{
    incByteCount();

    inb[byteNr++] = (uint32_t)ch;
    if (byteNr >= 4)
        {
        inBuf[longNr++] = inb[0] << 24 | inb[1] << 16 |
                          inb[2] << 8  | inb[3];
        byteNr = 0;
        }
    if (longNr >= 16)
        {
        transform();
        longNr = 0;
        }
}


void Sha1::transform()
{
    uint32_t *W = inBuf;
    uint32_t *H = hashBuf;

    //for (int t = 0; t < 16 ; t++)
    //    printf("%2d %08lx\n", t, W[t]);

    //see 6.1.2
    for (int t = 16; t < 80 ; t++)
        W[t] = SHA_ROTL((W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]), 1);

    uint32_t a = H[0];
    uint32_t b = H[1];
    uint32_t c = H[2];
    uint32_t d = H[3];
    uint32_t e = H[4];

    uint32_t T;

    int t = 0;
    for ( ; t < 20 ; t++)
        {
        //see 4.1.1 for the boolops on B,C, and D
        T = TR32(SHA_ROTL(a,5) + ((b&c)|((~b)&d)) +  //Ch(b,c,d))
                e + 0x5a827999L + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }
    for ( ; t < 40 ; t++)
        {
        T = TR32(SHA_ROTL(a,5) + (b^c^d) + e + 0x6ed9eba1L + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }
    for ( ; t < 60 ; t++)
        {
        T = TR32(SHA_ROTL(a,5) + ((b&c)^(b&d)^(c&d)) +
                e + 0x8f1bbcdcL + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }
    for ( ; t < 80 ; t++)
        {
        T = TR32(SHA_ROTL(a,5) + (b^c^d) +
                e + 0xca62c1d6L + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }

    H[0] = TR32(H[0] + a);
    H[1] = TR32(H[1] + b);
    H[2] = TR32(H[2] + c);
    H[3] = TR32(H[3] + d);
    H[4] = TR32(H[4] + e);
}




/**
 *
 */
std::vector<unsigned char> Sha1::finish()
{
    //snapshot the bit count now before padding
    getBitCount();

    //Append terminal char
    update(0x80);

    //pad until we have a 56 of 64 bytes, allowing for 8 bytes at the end
    while ((nrBytes & 63) != 56)
        update(0);

    //##### Append length in bits
    appendBitCount();

    //copy out answer
    std::vector<unsigned char> res;
    for (int i=0 ; i<5 ; i++)
        {
        res.push_back((unsigned char)((hashBuf[i] >> 24) & 0xff));
        res.push_back((unsigned char)((hashBuf[i] >> 16) & 0xff));
        res.push_back((unsigned char)((hashBuf[i] >>  8) & 0xff));
        res.push_back((unsigned char)((hashBuf[i]      ) & 0xff));
        }

    // Re-initialize the context (also zeroizes contents)
    reset();

    return res;
}



/**
 * Minimalist SHA-1 hash
 * Note that we expect output to be 20 bytes wide.
 */

void sha1hash(unsigned char *data, unsigned char *outbuf, int len)
{
    const uint32_t K[] =    
        {
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
        };

    uint32_t H[] =    
        {
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0
        };

    int           t;                 /* Loop counter                */
    uint32_t      temp;              /* Temporary word value        */
    uint32_t      W[80];             /* Word sequence               */
    uint32_t      A, B, C, D, E;     /* Word buffers                */

    /*
     *  Initialize the first 16 words in the array W
     */
    
    for (t = 0; t < 16; t++)
        {
        W[t] = context->Message_Block[t * 4] << 24;
        W[t] |= context->Message_Block[t * 4 + 1] << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
        }

    for (t = 16; t < 80; t++)
        {
        W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
        }

    uint32_t A = H[0];
    uint32_t B = H[1];
    uint32_t C = H[2];
    uint32_t D = H[3];
    uint32_t E = H[4];

    for (t = 0; t < 20; t++)
        {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);

        B = A;
        A = temp;
        }

    for (t = 20; t < 40; t++)
        {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
        }

    for (t = 40; t < 60; t++)
        {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
        }

    for (t = 60; t < 80; t++)
        {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
        }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;

    context->Message_Block_Index = 0;
    
    
    
    //Output the hash
    unsigned char *out = outbuf;
    
    for (i= 0 ; i < 5 ; i++)
        {
        unsigned int h = H[i];
        *out++ = (h >> 24) & 0xff;
        *out++ = (h >> 16) & 0xff;
        *out++ = (h >>  8) & 0xff;
        *out++ = (h      ) & 0xff;
        }
}




#endif













/*###################################################################################
##  S E R V E R
###################################################################################*/



typedef struct WsServer WsServer;

typedef struct ClientInfo ClientInfo;

#define CLIENT_BUFLEN 256
struct ClientInfo
{
    WsServer *server;
    int socket;
    char resourceName[CLIENT_BUFLEN];
    char buf[CLIENT_BUFLEN];
};

typedef void (WsHandlerFunc)(ClientInfo *);

ClientInfo *infoCreate()
{
    ClientInfo *obj = (ClientInfo *)malloc(sizeof(ClientInfo));
    if (!obj)
        {
        return NULL;
        }
    memset(obj, 0, sizeof(ClientInfo));
    return obj;
}

void infoDelete(ClientInfo *obj)
{
    if (obj)
        {
        free(obj);
        }
}




struct WsServer
{
    int sock;
    int port;
    pthread_t thread;
    int cont;
    WsHandlerFunc *handlerFunc;
};




static char *trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}




#define WS_TEXT   0
#define WS_BINARY 1



int wsSend(ClientInfo *info, unsigned char *dat, int len)
{
    
}

static long getInt(unsigned char *b, int size)
{
    long v = 0;
    while (size--)
        v = (v << 8) + *b++;
    return v;
}

int wsRecv(ClientInfo *info, unsigned char *dat, int len)
{
    int sock = info->socket;
    unsigned char *buf = (unsigned char *)info->buf;
    int count = read(sock, buf, 9); 
    if (count < 9)
        {
        return -1;
        }
    unsigned char *b = buf;
    int type = *b++;
    long packlen = getInt(b, 8);
    b += 8;

    trace("type: %d len:%ld", type, packlen);
}



static void defaultHandler(ClientInfo *info)
{
    trace("resource : '%s'", info->resourceName);
    char buf[256];
    int ret = wsRecv(info, buf, 255);

}




WsServer *wsCreate(WsHandlerFunc *func, int port)
{
    WsServer *obj = (WsServer *)malloc(sizeof(WsServer));
    if (!obj)
        {
        return NULL;
        }
    obj->handlerFunc = (func) ? func : defaultHandler;
    obj->port = port;
    obj->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (obj->sock < 0)
        {
        error("Could not create server socket");
        return NULL;
        }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int ret = bind(obj->sock, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0)
        {
        error("Could not bind to port %d : %s", port, strerror(errno));
        close(obj->sock);
        free(obj);
        return NULL;
        }
    listen(obj->sock, 5);
    return obj;
}


void wsDelete(WsServer *obj)
{
    if (obj)
        {
        close(obj->sock);
        free(obj);
        }
}





typedef struct 
{
    char *ext;
    char *mimeType;
} MimeEntry;


static MimeEntry mimeTable[] = 
{
    { "txt",  "text/plain"       },
    { "htm",  "text/html"        },
    { "html", "text/html"        },
    { "jpg",  "image/jpeg"       },
    { "jpeg", "image/jpeg"       },
    { "js",   "text/javascript"  },
    { "css",  "text/css"         },
    { "png",  "image/png"        },
    { NULL,   NULL               }
};


static char *getMimeType(char *fname)
{
    char *pos = strrchr(fname, '/');
    if (pos)
        fname = pos;
    pos = strrchr(fname, '.');
    if (pos)
        {
        MimeEntry *mime = mimeTable;
        char *ext = pos+1;
        while (mime->ext)
            {
            if (strcmp(mime->ext, ext)==0)
                return mime->mimeType;
            mime++;
            }
        }

    return "application/octet-stream";
}


static void outf(int fd, char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buf, 255, fmt, args);
    va_end(args);
    write(fd, buf, strlen(buf));
}



/**
 * Deliver a static file
 */
static void serveFile(ClientInfo *info)
{
    char *resName = info->resourceName;
    char *buf = info->buf;
    int sock = info->socket;
    
    snprintf(buf, 255, "./%s", info->resourceName);
    FILE *f = fopen(buf, "rb");
    if (!f)
        {
        outf(sock, "404 Resource '%s' not found\r\n", resName);
        }
    else
        {
        struct stat finfo;
        int ret = fstat(fileno(f), &finfo);
        if (ret < 0)
            {
            outf(sock, "404 Cannot stat resource '%s'\r\n", resName);
            fclose(f);
            return;
            }
        int len = finfo.st_size;
        outf(sock, "HTTP/1.1 200 OK\r\n");
        outf(sock, "Content-Type: %s\r\n", getMimeType(resName));
        outf(sock, "Content-Length: %d\r\n", len);
        outf(sock, "\r\n");
        
        while (len-- && !feof(f))
            {
            int count = fread(buf, 1, CLIENT_BUFLEN, f);
            if (count > 0)
                write(sock, buf, count);
            }
            
        fclose(f);
        }
}



static char *header =
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "\r\n";
    
/**
 * This is the thread for handling a client request.  One is spawned per client
 */
static void *handleClient(void *ctx)
{
    ClientInfo *info = (ClientInfo *)ctx;
    char *buf = info->buf;
    int sock = info->socket;
    
    int wsrequest = FALSE;
    FILE *in = fdopen(sock, "r");
    
    fgets(buf, CLIENT_BUFLEN, in);
    char *str = trim(buf);
    char *savptr;
    char *tok = strtok_r(str, " ", &savptr);
    if (strcmp(tok, "GET")!=0)
        {
        outf(sock, "405 Method '%s' not supported by this server", tok);
        }
    else
        {
        tok = strtok_r(NULL, " ", &savptr);
        strncpy(info->resourceName, tok, CLIENT_BUFLEN);
        while (fgets(buf, 255, in))
            {
            char *str = trim(buf);
            int len = strlen(str);
            if (len == 0)
                break;
            trace("%s", str);
            tok = strtok_r(str, ":", &savptr);
            if (strcmp(tok, "Upgrade")==0)
                wsrequest = TRUE;
            }
        trace("ready to process: %d", wsrequest);
        if (wsrequest)
            {
            write(sock, header, strlen(header));
            
            WsHandlerFunc *func = info->server->handlerFunc;
            if (func)
                {
                (*func)(info);
                }
            }
        else
            {
            WsServer *srv = info->server;
            serveFile(info);
            }
        }

    fclose(in);
    
    close(info->socket);
    
    infoDelete(info);
}



/**
 * This is the listening thread
 */
static void *listenForClients(void *ctx)
{
    WsServer *obj = (WsServer *)ctx;
    obj->cont = TRUE;
    while (obj->cont)
        {
        struct sockaddr_in addr;
        int addrlen = sizeof(addr);
        int clisock = accept(obj->sock, (struct sockaddr *) &addr, &addrlen); 
        if (clisock < 0)
            {
            error("Could not accept connection: %s", strerror(errno));
            }
        trace("have new client");
        ClientInfo *info = infoCreate();
        if (!info)
            {
            error("Could not create client info record");
            continue;
            }
        info->server = obj;
        info->socket = clisock;
        
        pthread_t thread;
        int rc = pthread_create(&thread, NULL, handleClient, (void *)info);
        if (rc)
            {
            error("ERROR; return code from pthread_create() is %d", rc);
            return FALSE;
            }
        
        }
}

int wsServe(WsServer *obj)
{
    trace("starting");
    pthread_t thread;
    int rc = pthread_create(&thread, NULL, listenForClients, (void *)obj);
    if (rc)
        {
        error("ERROR; return code from pthread_create() is %d", rc);
        return FALSE;
        }
    obj->thread = thread;
    pthread_join(obj->thread, NULL);
    return TRUE;
}







int main(int argc, char **argv)
{
    WsServer *srvr = wsCreate(NULL, 4444);
    if (srvr)
        {
        wsServe(srvr);
        }
}





