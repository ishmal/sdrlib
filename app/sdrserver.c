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
#include <ctype.h>

#include <wsserver.h>
#include <sdrlib.h>


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

/* ##########################################################################################
## S E R V E R
########################################################################################## */

typedef struct SdrServer SdrServer;

/**
 * Place any contextual data here
 */
struct SdrServer
{
    SdrLib *sdr;
    WsServer *wsServer;
};


static void codecOutput(unsigned char *dat, int len, void *context)
{
    SdrServer *svr = (SdrServer *)context; 
    WsHandler *ws = wsGetClientWs(svr->wsServer);
    if (ws)
        {
        wsSendBinary(ws, dat, len);
        } 
}


SdrServer *svrCreate()
{
    SdrServer *svr = (SdrServer *)malloc(sizeof(SdrServer));
    if (!svr)
        {
        return NULL;
        }
    svr->sdr = sdrCreate(svr, NULL, codecOutput);  //TODO: important to supply these values
    if (!svr->sdr)
        {
        free(svr);
        return NULL;
        }
    return svr;
}


void svrDelete(SdrServer *svr)
{
    if (svr)
        {
        sdrDelete(svr->sdr);
        free(svr);
        }
}


static int equ(char *a, char *b)
{
    return strcmp(a, b)==0;
}

static int getDouble(char *str, double *f)
{
    char *end;
    double val = strtod(str, &end);
    if (str == end)
        {
        error("Invalid floating point format: '%s'", str);
        return FALSE;
        }
    else
        {
        *f = val;
        return TRUE;
        }
 }

static int getFloat(char *str, float *f)
{
    double val;
    int ret = getDouble(str, &val);
    if (ret)
        *f = val;
    return ret;
 }


int parseAndExecute(SdrLib *sdr, char *buf)
{
    int len = strlen(buf);
    if (len > 0 && buf[len-1]=='\n')
        {
        len--;
        buf[len] = '\0';
        }
    int i = 0;
    for (i=0 ; i < len ; i++)
        buf[i] = tolower(buf[i]);
    trace("Buf:'%s'", buf);
    char *delim = " ,\t";
    char *ctx;
    char *cmd = strtok_r(buf, delim, &ctx);
    if (!cmd)
        return TRUE;
    char *p0 = strtok_r(NULL, delim, &ctx);
    
    if (equ(cmd, "exit")||equ(cmd, "quit")||equ(cmd, "q"))
        {
        return -1;
        }
    else if (equ(cmd, "start"))
        {
        sdrStart(sdr);
        }
    else if (equ(cmd, "stop"))
        {
        sdrStop(sdr);
        }
    else if (equ(cmd, "freq")||equ(cmd, "f"))
        {
        if (!p0)
            {
            double freq = sdrGetCenterFrequency(sdr);
            trace("freq: %f", freq);
            }
        else
            {
            double freq;
            if (getDouble(p0, &freq))
                {
                sdrSetCenterFrequency(sdr, freq);
                }
            }
        }
    else if (equ(cmd, "gain") || equ(cmd, "g"))
        {
        if (!p0)
            {
            float gain = sdrGetRfGain(sdr);
            trace("gain: %f", gain);
            }
        else
            {
            float gain;
            if (getFloat(p0, &gain))
                {
                if (gain < 0.0 || gain > 1.0)
                    {
                    error("Gain must be in the range 0.0 .. 1.0");
                    }
                else
                    sdrSetRfGain(sdr, gain);
                }
            }
        }
    else if (equ(cmd, "vfo") || equ(cmd, "v"))
        {
        if (!p0)
            {
            float f = sdrGetVfo(sdr);
            trace("vfo: %f", f);
            }
        else
            {
            float f;
            if (getFloat(p0, &f))
                {
                sdrSetVfo(sdr, f);
                }
            }
        }
    else if (equ(cmd, "lo"))
        {
        if (!p0)
            {
            float f = sdrGetPbLo(sdr);
            trace("pblo: %f", f);
            }
        else
            {
            float f;
            if (getFloat(p0, &f))
                {
                sdrSetPbLo(sdr, f);
                }
            }
        }
    else if (equ(cmd, "hi"))
        {
        if (!p0)
            {
            float f = sdrGetPbHi(sdr);
            trace("pbhi: %f", f);
            }
        else
            {
            float f;
            if (getFloat(p0, &f))
                {
                sdrSetPbHi(sdr, f);
                }
            }
        }
    else
        {
        error("Unimplemented command:'%s'", cmd);
        }

    return TRUE;
}








static void onOpen(WsHandler *ws, char *msg)
{
    SdrServer *svr = (SdrServer *) ws->context;
    SdrLib *sdr = svr->sdr;
}

static void onClose(WsHandler *ws, char *msg)
{
    SdrServer *svr = (SdrServer *) ws->context;
    SdrLib *sdr = svr->sdr;
}

static void onMessage(WsHandler *ws, unsigned char *data, int len)
{
    SdrServer *svr = (SdrServer *) ws->context;
    SdrLib *sdr = svr->sdr;

    int ret = parseAndExecute(sdr, (char *)data);

}

static void onError(WsHandler *ws, char *msg)
{
    SdrServer *svr = (SdrServer *) ws->context;
    SdrLib *sdr = svr->sdr;
}





static int doRun(char *dir, int port)
{
    SdrServer *ctx = svrCreate();
    if (!ctx)
        {
        return FALSE;
        }
    WsServer *svr = wsCreate(onOpen, onClose, onMessage, onError, (void *)ctx, dir, port);
    int ret = TRUE;
    if (svr)
        {
        ctx->wsServer = svr;
        wsServe(svr);
        ret = FALSE;
        }
    svrDelete(ctx);
    return ret;
}



/* ##########################################################################################
## M A I N
########################################################################################## */

static void usage(char *progname)
{
    char * msg = 
        "Usage: %s { options }\n"
        "    where options are:\n"
        "-d <root_directory>\n"
        "-p <port_number>\n";

    fprintf(stderr, msg, progname);
}


int main(int argc, char **argv)
{
    char *dir = ".";
    int port = 8888;
    int c;
    while ((c = getopt (argc, argv, "d:p:")) != -1)
        {
        switch (c)
            {
            case 'd':
                dir = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                usage(argv[0]);
                return -1;
            }
        }
    if (doRun(dir, port))
        return 0;
    else
        return -1;
}

