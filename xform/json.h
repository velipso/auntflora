//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct json_value_st {
  enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL
  } kind;
  union {
    struct {
      char **keys;
      struct json_value_st **values;
      int count;
    } object;
    struct {
      struct json_value_st **values;
      int count;
    } array;
    char *string;
    double number;
  } u;
} json_value;

#define JSON_IS_SPACE(c)  ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

static void json_parseobject(json_value *v, const uint8_t *data, int *i);
static void json_parsearray(json_value *v, const uint8_t *data, int *i);
static char *json_parsestring(const uint8_t *data, int *i);
static double json_parsenumber(const uint8_t *data, int *i);

static json_value *json_parsevalue(const uint8_t *data, int *i) {
  json_value *v = malloc(sizeof(json_value));
  while (1) {
    uint8_t ch = data[*i];
    if (JSON_IS_SPACE(ch)) {
      *i = *i + 1;
    } else if (ch == '{') {
      *i = *i + 1;
      v->kind = JSON_OBJECT;
      v->u.object.keys = NULL;
      v->u.object.values = NULL;
      v->u.object.count = 0;
      json_parseobject(v, data, i);
      return v;
    } else if (ch == '[') {
      *i = *i + 1;
      v->kind = JSON_ARRAY;
      v->u.array.values = NULL;
      v->u.array.count = 0;
      json_parsearray(v, data, i);
      return v;
    } else if (ch == '"') {
      *i = *i + 1;
      v->kind = JSON_STRING;
      v->u.string = json_parsestring(data, i);
      return v;
    } else if (ch == '-' || (ch >= '0' && ch <= '9')) {
      v->kind = JSON_NUMBER;
      v->u.number = json_parsenumber(data, i);
      return v;
    } else if (ch == 't') {
      *i = *i + 4;
      v->kind = JSON_TRUE;
      return v;
    } else if (ch == 'f') {
      *i = *i + 5;
      v->kind = JSON_FALSE;
      return v;
    } else if (ch == 'n') {
      *i = *i + 4;
      v->kind = JSON_NULL;
      return v;
    } else {
      fprintf(stderr, "\nInvalid JSON at character: %d\n", *i);
      exit(1);
    }
  }
}

static void json_parseobject(json_value *v, const uint8_t *data, int *i) {
  while (JSON_IS_SPACE(data[*i]))
    *i = *i + 1;
  if (data[*i] == '}')
    return;
  while (1) {
    uint8_t ch = data[*i];
    if (JSON_IS_SPACE(ch)) {
      *i = *i + 1;
    } else if (ch == '"') {
      *i = *i + 1;
      char *key = json_parsestring(data, i);
      while (JSON_IS_SPACE(data[*i]))
        *i = *i + 1;
      if (data[*i] != ':') {
        fprintf(stderr, "\n1Invalid JSON at character: %d\n", *i);
        exit(1);
      }
      *i = *i + 1;
      json_value *value = json_parsevalue(data, i);
      v->u.object.count++;
      v->u.object.keys = realloc(v->u.object.keys, sizeof(void *) * v->u.object.count);
      v->u.object.keys[v->u.object.count - 1] = key;
      v->u.object.values = realloc(v->u.object.values, sizeof(void *) * v->u.object.count);
      v->u.object.values[v->u.object.count - 1] = value;
      while (JSON_IS_SPACE(data[*i]))
        *i = *i + 1;
      if (data[*i] == ',') {
        *i = *i + 1;
      } else if (data[*i] == '}') {
        *i = *i + 1;
        return;
      } else {
        printf("??? %c\n", data[*i]);
        fprintf(stderr, "\n2Invalid JSON at character: %d\n", *i);
        exit(1);
      }
    } else {
      fprintf(stderr, "\nInvalid JSON at character: %d\n", *i);
      exit(1);
    }
  }
}

static void json_parsearray(json_value *v, const uint8_t *data, int *i) {
  while (JSON_IS_SPACE(data[*i]))
    *i = *i + 1;
  if (data[*i] == ']')
    return;
  while (1) {
    uint8_t ch = data[*i];
    if (JSON_IS_SPACE(ch)) {
      *i = *i + 1;
    } else {
      json_value *value = json_parsevalue(data, i);
      v->u.array.count++;
      v->u.array.values = realloc(v->u.array.values, sizeof(void *) * v->u.array.count);
      v->u.array.values[v->u.array.count - 1] = value;
      while (JSON_IS_SPACE(data[*i]))
        *i = *i + 1;
      if (data[*i] == ',') {
        *i = *i + 1;
      } else if (data[*i] == ']') {
        *i = *i + 1;
        return;
      } else {
        fprintf(stderr, "\nInvalid JSON at character: %d\n", *i);
        exit(1);
      }
    }
  }
}

static char *json_parsestring(const uint8_t *data, int *i) {
  int size = 0;
  int j = 0;
  while (data[*i + j] != '"') {
    if (data[*i + j] == '\\') {
      j++;
    }
    size++;
    j++;
  }
  char *str = malloc(size + 1);
  j = 0;
  while (data[*i] != '"') {
    if (data[*i] == '\\') {
      *i = *i + 1;
      switch (data[*i]) {
        case '"':
        case '\\':
        case '/':
          str[j] = data[*i];
          break;
        case 'b':
          str[j] = '\b';
          break;
        case 'f':
          str[j] = '\f';
          break;
        case 'n':
          str[j] = '\n';
          break;
        case 'r':
          str[j] = '\r';
          break;
        case 't':
          str[j] = '\t';
          break;
      }
    } else {
      str[j] = data[*i];
    }
    j++;
    *i = *i + 1;
  }
  str[j] = 0;
  *i = *i + 1;
  return str;
}

static double json_parsenumber(const uint8_t *data, int *i) {
  const char *datach = (const char *)data;
  char *end;
  double v = strtod(&datach[*i], &end);
  *i = end - datach;
  return v;
}

void json_free(json_value *jv) {
  switch (jv->kind) {
    case JSON_OBJECT:
      for (int i = 0; i < jv->u.object.count; i++) {
        free(jv->u.object.keys[i]);
        json_free(jv->u.object.values[i]);
      }
      free(jv->u.object.keys);
      free(jv->u.object.values);
      break;
    case JSON_ARRAY:
      for (int i = 0; i < jv->u.array.count; i++) {
        json_free(jv->u.array.values[i]);
      }
      free(jv->u.array.values);
      break;
    case JSON_STRING:
      free(jv->u.string);
      break;
    case JSON_NUMBER:
    case JSON_TRUE:
    case JSON_FALSE:
    case JSON_NULL:
      break;
  }
  free(jv);
}

json_value *json_parsefile(const char *file) {
  FILE *fp = fopen(file, "r");
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  uint8_t *data = malloc(size);
  fread(data, size, 1, fp);
  fclose(fp);
  int i = 0;
  json_value *jv = json_parsevalue(data, &i);
  free(data);
  return jv;
}

json_value *json_objectkey(json_value *v, const char *key) {
  if (v->kind != JSON_OBJECT) {
    fprintf(stderr, "\nExpecting JSON object when getting key \"%s\"\n", key);
    exit(1);
  }
  for (int i = 0; i < v->u.object.count; i++) {
    if (strcmp(v->u.object.keys[i], key) == 0) {
      return v->u.object.values[i];
    }
  }
  fprintf(stderr, "\nObject missing key \"%s\"\n", key);
  exit(1);
}

char *json_string(json_value *v) {
  if (v->kind != JSON_STRING) {
    fprintf(stderr, "\nExpecting string\n");
    exit(1);
  }
  return v->u.string;
}

double json_number(json_value *v) {
  if (v->kind != JSON_NUMBER) {
    fprintf(stderr, "\nExpecting number\n");
    exit(1);
  }
  return v->u.number;
}

bool json_boolean(json_value *v) {
  if (v->kind == JSON_TRUE)
    return true;
  else if (v->kind == JSON_FALSE || v->kind == JSON_NULL)
    return false;
  fprintf(stderr, "\nExpecting boolean\n");
  exit(1);
}

#define JSON_PRINT_TRUNCATE  1

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void json_print(json_value *jv, int tab, int flags) {
  switch (jv->kind) {
    case JSON_OBJECT:
      printf("{\n%*s", (tab + 1) * 2, "");
      for (int i = 0; i < jv->u.object.count; i++) {
        printf(ANSI_COLOR_CYAN "\"%s\"" ANSI_COLOR_RESET ": ", jv->u.object.keys[i]);
        json_print(jv->u.object.values[i], tab + 1, flags);
        if (i < jv->u.object.count - 1)
          printf(",\n%*s", (tab + 1) * 2, "");
      }
      printf("\n%*s}", tab * 2, "");
      break;
    case JSON_ARRAY:
      if (jv->u.array.count <= 0) {
        printf("[]");
      } else {
        printf("[");
        for (int i = 0; i < jv->u.array.count; i++) {
          if (flags & JSON_PRINT_TRUNCATE) {
            if (jv->u.array.count > 15) {
              if (i == 5)
                printf("..., ");
              if (i >= 5 && i < jv->u.array.count - 5)
                continue;
            }
          }
          json_print(jv->u.array.values[i], tab, flags);
          if (i < jv->u.array.count - 1)
            printf(", ");
        }
        printf("]");
      }
      break;
    case JSON_STRING:
      printf(ANSI_COLOR_MAGENTA "\"%s\"" ANSI_COLOR_RESET, jv->u.string);
      break;
    case JSON_NUMBER:
      printf(ANSI_COLOR_GREEN "%g" ANSI_COLOR_RESET, jv->u.number);
      break;
    case JSON_TRUE:
      printf(ANSI_COLOR_BLUE "true" ANSI_COLOR_RESET);
      break;
    case JSON_FALSE:
      printf(ANSI_COLOR_BLUE "false" ANSI_COLOR_RESET);
      break;
    case JSON_NULL:
      printf(ANSI_COLOR_BLUE "null" ANSI_COLOR_RESET);
      break;
  }
  if (tab == 0)
    printf("\n");
}
