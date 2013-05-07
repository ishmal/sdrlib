/**
 * Tiny JSON parser definitions and implementations.
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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>

#include "json.h"
#include "private.h"


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


JsonVal *jsonCreate(int type)
{
    JsonVal *v = (JsonVal *)malloc(sizeof(JsonVal));
    if (!v)
        return NULL;
    memset(v, 0, sizeof(JsonVal));
    v->type = type;
    return v;
}


void jsonDelete(JsonVal *val)
{
    if (!val)
        return;
    if (val->type == JsonStr)
        {
        if (val->value.s)
            free(val->value.s);
        }
    else if (val->type == JsonArr)
        {
        JsonNode *curr = val->value.a;
        JsonNode *next = NULL;
        while (curr)
            {
            next = curr->next;
            jsonDelete(curr->value);
            free(curr);
            curr = next;
            }
        }
    else if (val->type == JsonObj)
        {
        JsonProp **buckets = val->value.h;
        int i = 0;
        for ( ; i < 256 ; i++)
            {
            JsonProp *curr = buckets[i];
            JsonProp *next = NULL;
            while (curr)
                {
                next = curr->next;
                free(curr->name);
                jsonDelete(curr->value);
                free(curr);
                curr = next;
                }
            }
        free(buckets);
        }
    free(val);
}




/**
 * Output a JSON string with all of the proper escapes
 */       
static int formatJsonStr(char *dest, int destlen, char *src)
{
    if (destlen < 2)
        {
        return -1;
        }
    char *out = dest;
    char *end = dest + destlen - 2;
    
    *out++ = '"';
    while (*src)
        {
        char ch = *src++;
        if (ch == '\\')       { *out++ = '\\'; *out++ = '\\'; }
        else if (ch == '"')   { *out++ = '\\'; *out++ = '"';  }
        else if (ch == '\f')  { *out++ = '\\'; *out++ = 'f';  }
        else if (ch == '\b')  { *out++ = '\\'; *out++ = 'b';  }
        else if (ch == '\t')  { *out++ = '\\'; *out++ = 't';  }
        else if (ch == '\r')  { *out++ = '\\'; *out++ = 'r';  }
        else if (ch == '\n')  { *out++ = '\\'; *out++ = 'n';  }
        else if (ch >= 32 && ch < 127) *out++ = ch;
        else 
            { snprintf(out, 6, "\\u%04x", (uint32_t) ch); out += 6; }
                
        if (out >= end)
            return -1;
        }
        
    *out++ = '"';
    return out-dest;
}







/* ##########################################
## P R I M I T I V E
########################################## */

JsonVal *jsonNil()
{
    JsonVal *v = jsonCreate(JsonNil);
    if (!v) return NULL;
    return v;
}

JsonVal *jsonBool(int value)
{
    JsonVal *v = jsonCreate(JsonBool);
    if (!v) return NULL;
    v->value.b = (value) ? 1 : 0;
    return v;
}

JsonVal *jsonInt(int value)
{
    JsonVal *v = jsonCreate(JsonInt);
    if (!v) return NULL;
    v->value.i = value;
    return v;
}

JsonVal *jsonFloat(float value)
{
    JsonVal *v = jsonCreate(JsonFloat);
    if (!v) return NULL;
    v->value.f = value;
    return v;
}

JsonVal *jsonStr(char *value)
{
    JsonVal *v = jsonCreate(JsonStr);
    if (!v) return NULL;
    char *str = strdup(value);
    if (!str)
        {
        free(v);
        return NULL;
        }
    v->value.s = str;
    return v;
}

/* ##########################################
## A R R A Y
########################################## */

JsonVal *jsonArr()
{
    JsonVal *v = jsonCreate(JsonArr);
    if (!v) return NULL;
    v->value.a = NULL;
    return v;
}

static JsonNode *nodeCreate(JsonVal *val)
{
    JsonNode *node = (JsonNode *)malloc(sizeof(JsonNode));
    if (!node)
        return NULL;
    node->next = NULL;
    node->value = val;
    return node;
}

int jsonArrAdd(JsonVal *arr, JsonVal *item)
{
    if (arr->type != JsonArr)
        {
        return FALSE;
        }
    JsonNode *newnode = nodeCreate(item);
    if (!newnode)
        return FALSE;
    JsonNode *n = arr->value.a;
    if (!n)
        arr->value.a = newnode;
    else
        {
        while (n->next)
            n = n->next;
        n->next = newnode;
        }        
    return TRUE;
}


/* ##########################################
## O B J E C T
########################################## */
/**
 * Oh-so-Tiny jenkins hash
 */
static uint32_t jenkinsHash(char *key)
{
    uint32_t hash = 0;
    while (*key)
        {
        hash += *key++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}





JsonVal *jsonObj()
{
    JsonVal *v = jsonCreate(JsonObj);
    if (!v) return NULL;
    JsonProp **props = (JsonProp **)malloc(256 * sizeof(JsonProp *));
    if (!props)
        {
        free(v);
        return NULL;
        }
    v->value.h = props;
    return v;
}


static JsonProp *propCreate(char *name, JsonVal *value)
{
    JsonProp *p = (JsonProp *)malloc(sizeof(JsonProp));
    if (!p)
        return NULL;
    p->next = NULL;
    p->name = strdup(name);
    if (!p->name)
        {
        free( p );
        return NULL;
        }
    p->value = value;
    return p;
}


int jsonObjPut(JsonVal *obj, char *name, JsonVal *value)
{
    if (!obj || obj->type != JsonObj)
        {
        return FALSE;
        }
    if (!name || !value)
        {
        return FALSE;
        }
    uint32_t hash = jenkinsHash(name);
    int bucketNr = hash & 255;
    JsonProp **buckets = obj->value.h;
    JsonProp *prop = buckets[bucketNr];
    //create first entry
    if (!prop)
        {
        JsonProp *p = propCreate(name, value);
        if (!p)
            return FALSE;
        buckets[bucketNr] = p;
        return TRUE;
        }
    JsonProp *prev = NULL;
    while (prop)
        {
        //replace existing value
        if (strcmp(name, prop->name)==0)
            {
            jsonDelete(prop->value);
            prop->value = value;
            return TRUE;
            }
        prev = prop;
        prop = prop->next;
        }
    //append to end
    JsonProp *p = propCreate(name, value);
    if (!p)
        return FALSE;
    prev->next = p;
 
    return TRUE;
}


JsonVal *jsonObjGet(JsonVal *obj, char *name)
{
    if (obj->type != JsonObj)
        return NULL;
    
    uint32_t hash = jenkinsHash(name);
    int bucketNr = hash & 255;
    JsonProp **buckets = obj->value.h;
    JsonProp *prop = buckets[bucketNr];
    while (prop)
        {
        if (strcmp(prop->name, name)==0)
            return prop->value;
        prop = prop->next;
        }
    return NULL;
}



/* ###############################################################################
## P A R S E R
############################################################################### */


#define MAXSTR (2048)

typedef struct
{
    char *buf;
    int len;
    int pos;
    char strbuf[MAXSTR];
} JsonParser;

static JsonVal *parse(JsonParser *p);


void jerror(JsonParser *p, char *format, ...)
{
    fprintf(stderr, "json parser (%d): ", p->pos);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}


static int skipwhite(JsonParser *p)
{
    while (isspace(p->buf[p->pos]))
        {
        if (p->pos >= p->len)
            return FALSE;
        p->pos++;
        }
    return TRUE;
}

static int get(JsonParser *p)
{
    if (p->pos >= p->len)
        return -1;
    else
        return p->buf[p->pos++];
}


static int getString(JsonParser *p)
{
    int ch = get( p );
    if (ch != '\'' && ch != '"')
        {
        jerror(p, "String expected single or double quote");
        return FALSE;
        }
    int quoteChar = ch;
    char *s = p->strbuf;
    while (1)
        {
        if ((s - p->strbuf) > MAXSTR)
            {
            jerror(p, "String too long");
            return FALSE;
            }
        ch = get( p );
        if (ch < 0)
            {
            jerror(p, "Unterminated string");
            return FALSE;
            }
        else if (ch == '\\')
            {
            ch = get( p );
            if (ch == '\\')     *s++ = '\\';
            else if (ch == '/') *s++ = '/';
            else if (ch == '"') *s++ = '"';
            else if (ch == 'f') *s++ = '\f';
            else if (ch == 'b') *s++ = '\b';
            else if (ch == 't') *s++ = '\t';
            else if (ch == 'r') *s++ = '\r';
            else if (ch == 'n') *s++ = '\n';
            else if (ch == 'u')
                {
                char *startptr = p->buf + p->pos;
                char *endptr;
                long v = strtol(startptr, &endptr, 16);
                if ((endptr - startptr) != 4)
                    {
                    jerror(p, "Invalid hex format");
                    return FALSE;
                    }
                *s++ = v & 255;
                p->pos += 4;
                }
            }
        else if (ch == quoteChar)
            {
            break;
            }
        else
            {
            *s++ = ch;
            }
        }
    return TRUE;     
}


static JsonVal *getObject(JsonParser *p)
{
    JsonVal *js = jsonObj();
    while (1)
        {
        if (!skipwhite( p ))
            {
            jerror(p, "Unterminated object, looking for property name");
            jsonDelete(js);
            return NULL;
            }
        if (!getString( p ))
            {
            jerror(p, "Expected object property name");
            jsonDelete(js);
            return NULL;
            }
        char *name = strdup(p->strbuf);
        if (!name)
            {
            }
        if (!skipwhite( p ))
            {
            jerror(p, "Unterminated object looking for ':'");
            free(name);
            jsonDelete(js);
            return NULL;
            }
        int ch = get( p );
        if (ch != ':')
            {
            jerror(p, "Object expected ':'");
            free(name);
            jsonDelete(js);
            return NULL;
            }
        JsonVal *v = parse( p );
        if (!v)
            {
            jerror(p, "Object expected JsonValue");
            free(name);
            jsonDelete(js);
            return NULL;
            }
        if (!jsonObjPut(js, name, v))
            {
            }
        free(name);
        if (!skipwhite( p ))
            {
            jerror(p, "Unterminated object looking for '%c' or '%c'", ',', '}');
            free(name);
            jsonDelete(js);
            return NULL;
            }
        ch = get( p );
        if (ch == '}')
            {
            break;
            }
        else if (ch == ',')
            {
            }
        else
            {
            jerror(p, "Object expected '%c' or '%c'", ':', '}');
            jsonDelete(js);
            return NULL;
            }
        }
    return js;
}


static JsonVal *getArray(JsonParser *p)
{
    JsonVal *js = jsonArr();
    while (1)
        {
        if (!skipwhite( p ))
            {
            jerror(p, "Unterminated array, looking for property name");
            jsonDelete(js);
            return NULL;
            }
        JsonVal *v = parse( p );
        if (!v)
            {
            jerror(p, "Array expected JsonValue");
            jsonDelete(js);
            return NULL;
            }
        if (!jsonArrAdd(js, v))
            {
            }
        if (!skipwhite( p ))
            {
            jerror(p, "Unterminated array looking for '%c' or '%c'", ',', ']');
            jsonDelete(js);
            return NULL;
            }
        int ch = get( p );
        if (ch == ']')
            {
            break;
            }
        else if (ch == ',')
            {
            }
        else
            {
            jerror(p, "Array expected '%c' or '%c'", ':', ']');
            jsonDelete(js);
            return NULL;
            }
        }
    return js;
}



static JsonVal *parse(JsonParser *p)
{
    char *buf = p->buf;
    
    if (!skipwhite( p ))
        return NULL;
    
    int ch = get(p);

    if (ch == '{')
        {
        JsonVal *js = getObject( p );
        return js;
        }
    else if (ch == '[')
        {
        JsonVal *js = getArray( p );
        return js;
        }
    else if (ch == '"')
        {
        p->pos--; //back up to the quote again
        if (!getString( p ))
            {
            return NULL;
            }
        return jsonStr(p->strbuf);
        }
    else if (strncmp("null", buf, 4)==0)
        {
        p->pos += 4;
        return jsonNil();
        }
    else if (strncmp("true", buf, 4)==0)
        {
        p->pos += 4;
        return jsonBool(TRUE);
        }
    else if (strncmp("false", buf, 4)==0)
        {
        p->pos += 5;
        return jsonBool(FALSE);
        }
    else
        {
        //try for a float, then int
        char *startptr = p->buf + p->pos;
        char *endptr;
        float fval = strtof(startptr, &endptr);
        int flen = endptr - startptr;
        int ival = strtol(startptr, &endptr, 10);
        int ilen = endptr - startptr;
        if (flen > 0 && flen > ilen)
            return jsonFloat(fval);
        else if (ilen>0)
            return jsonInt(ival);
        else
            {
            jerror(p, "Unknown token");
            return NULL;
            }
        }
}


JsonVal *jsonParse(char *buf, int len)
{
    JsonParser *p = (JsonParser *)malloc(sizeof(JsonParser));
    if (!p)
        {
        return NULL;
        }
        
    p->buf = buf;
    p->len = len;
    p->pos = 0;
        
    JsonVal *js = parse( p );
    
    skipwhite( p );
    if (p->pos < p->len)
        {
        jerror(p, "Illegal content at end of JSON string");
        jsonDelete(js);
        js = NULL;
        }
        
    free( p );
    return js;
    
}
















