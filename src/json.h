#ifndef _JSON_H_
#define _JSON_H_
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



/**
 *
 */
JsonVal *jsonCreate(int type);

/**
 *
 */
void jsonDelete(JsonVal *val);

/**
 * Output a JSON string with all of the proper escapes
 */       
static int jsonToStr(char *dest, int destlen, char *src);






/* ##########################################
## P R I M I T I V E
########################################## */

/**
 *
 */
JsonVal *jsonNil();


/**
 *
 */
JsonVal *jsonBool(int value);


/**
 *
 */
JsonVal *jsonInt(int value);


/**
 *
 */
JsonVal *jsonFloat(float value);


/**
 *
 */
JsonVal *jsonStr(char *value);


/* ##########################################
## A R R A Y
########################################## */

/**
 *
 */
JsonVal *jsonArr();

/**
 *
 */
int jsonArrAdd(JsonVal *arr, JsonVal *item);

/* ##########################################
## O B J E C T
########################################## */

/**
 *
 */
JsonVal *jsonObj();

/**
 *
 */
int jsonObjPut(JsonVal *obj, char *name, JsonVal *value);

/**
 *
 */
JsonVal *jsonObjGet(JsonVal *obj, char *name);


/* ###############################################################################
## P A R S E R
############################################################################### */


/**
 *
 */
JsonVal *jsonParse(char *buf, int len);








































#endif /* _JSON_H_ */


