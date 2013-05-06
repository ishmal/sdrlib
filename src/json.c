


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct JsonVal JsonVal;
typedef struct JsonObj JsonObj;



typedef struct JsonNode JsonNode;
struct JsonNode
{
    JsonNode *next;
    JsonVal *value;
};

typedef struct JsonProp JsonProp;
struct JsonProp
{
    JsonProp *next;
    char *name;
    JsonVal *value;
};



#define JsonNil   0
#define JsonBool  1
#define JsonInt   2
#define JsonFloat 3
#define JsonStr   4
#define JsonArr   5
#define JsonObj   6

struct JsonVal
{
    uint8_t type;
    union
        {
        uint8_t    b;
        int        i;
        float      f;
        char      *s;
        JsonNode  *a;
        JsonProp **h;
        } value;
};



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

JsonVal *jsonNil(float value)
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

int jsonArrAdd(JsonVal *arr, JsonVal *item)
{
    if (arr->type != JsonArr)
        {
        return FALSE;
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


















