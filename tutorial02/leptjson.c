#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGITT(ch)        ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/* refactoring the true & false & null */
static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
    size_t i;
    EXPECT(c, literal[0]);
    for(i = 0; literal[i+1] != '\0'; ++i){
        if(c->json[i] != literal[i+1]){
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    /* only do the validate here, do not consider its value */
    /* sign */
    if (*p == '-')  ++p;
    /* int */
    if (*p == '0')  ++p;
    else {
        if (!(ISDIGIT1TO9(*p)))     return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGITT(*p); p++);
    }
    /* frac */
    if (*p == '.') {
        p++;
        if (!(ISDIGITT(*p)))        return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGITT(*p); p++);
    }
    /* exp */
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!(ISDIGITT(*p)))        return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGITT(*p); p++);
    }    

    v->n = strtod(c->json, NULL);
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

/* value */
static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

/* wrong type */
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        /* the verify of if some illegal part follow the number is done here */
        /* for example: the end of the number test */
        /* if (*p != '\0')                 return LEPT_PARSE_INVALID_VALUE; */
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

/* data type */
lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
