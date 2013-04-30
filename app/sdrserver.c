/**
 * This provides the backend file and websocket server for the
 * linetest application
 *
 * @author Bob Jamison
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h> //for getopt()
#include <wsserver.h>






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

typedef struct LineTest LineTest;

/**
 * Place any contextual data here
 */
struct LineTest
{
    int dummy;
};


static void lineTestServer(ClientInfo *info)
{
    LineTest *ctx = (LineTest *)info->context;
}





static int doRun(int port)
{
    LineTest *ctx = (LineTest *)malloc(sizeof(LineTest));
    ctx->dummy = 0xcafebabe; //does nothing.  just an example
    WsServer *svr = wsCreate(lineTestServer, (void *)ctx, port);
    wsServe(svr);
    free(ctx);
    return TRUE;
}



/* ##########################################################################################
## M A I N
########################################################################################## */

static void usage(char *progname)
{
    char * msg = 
        "Usage: %s { options }\n"
        "    where options are:\n"
        "-p <portnumber>\n";

    fprintf(stderr, msg, progname);
}


int main(int argc, char **argv)
{
    int port = 8888;
    int c;
    while ((c = getopt (argc, argv, "p:")) != -1)
        {
        switch (c)
            {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                usage(argv[0]);
                return -1;
            }
        }
    if (doRun(port))
        return 0;
    else
        return -1;
}

