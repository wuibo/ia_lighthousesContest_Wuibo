#include "jsonparser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define s_start 0
#define s_name 1
#define s_nameend 2
#define s_type 3
#define s_next 4
#define s_array 5
#define s_array_next 6
#define s_object_name 7
#define s_object_nameend 8
#define s_object_type 9
#define s_object_next 10

int read_json(struct object *object, char *json, unsigned int size) {
  short state = s_start;
  struct object *json_root;
  struct object *json_temp;
  struct object *json_back;
  struct object *temp_object;
  unsigned int *object_count;
  unsigned int *object_back;
  unsigned int point;
  unsigned short stack_level = 0;
  char *temp_char;
  char *back_char;
  char *eptr;
  char *temp_int;
  double *temp_num;
  unsigned int char_size, i;
  char x_char;
  char hex_char[4];
  unsigned short decimal;
  unsigned short end = 0;

  object->type = json_type_object;
  object_count = malloc(sizeof *object_count);
  object_count[0] = 0;
  json_root = malloc(sizeof *json_root);

  for (point = 0; point < size; point++) {
    if (end == 1) {
      break;
    }
    switch (state) {
    case s_start:
      // Inicio  del json se espera withespace o { para empezar
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case '{':
        // comienza
        state = s_name;
        break;
      default:
        // error de ejecución
        return -1;
        break;
      }
      break;
    case s_name:
      // cargando nombre. cualquier caracter cuidado con los especiales
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case '}':
        end = 1;
        break;
      case '"':
        i = get_string(json, point, &temp_char, size, &char_size);
        if (i == 0) {
          return -1;
        } else {
          point = i;
          json_back = json_root;
          json_root =
              malloc((object_count[stack_level] + 1) * sizeof *json_root);
          for (i = 0; i < object_count[stack_level]; i++) {
            json_root[i] = json_back[i];
          }
          json_root[object_count[stack_level]].name = temp_char;
          json_root[object_count[stack_level]].name_size = char_size;
          object_count[stack_level]++;
          free(json_back);
          state = s_nameend;
        }
        break;
      default:
        return -1;
        break;
      }
      break;
    case s_nameend:
      // esperar : para buscar tipo (witespace o :)
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case ':':
        // detectar tipo
        state = s_type;
        break;
      default:
        // error de ejecución
        return -1;
        break;
      }
      break;
    case s_type:
      // witespace o inicio de tipo
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case 't':
        // true
        if ((json[point + 1] == 'r' && json[point + 2] == 'u' &&
             json[point + 3] == 'e') ||
            (json[point + 1] == 'R' && json[point + 2] == 'U' &&
             json[point + 3] == 'E')) {
          // correcto
          json_root[object_count[stack_level] - 1].type = json_type_boolean;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 1;
          json_root[object_count[stack_level] - 1].content = temp_int;
          json_root[object_count[stack_level] - 1].size = 1;
          point += 3;
          state = s_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'T':
        // true
        if ((json[point + 1] == 'r' && json[point + 2] == 'u' &&
             json[point + 3] == 'e') ||
            (json[point + 1] == 'R' && json[point + 2] == 'U' &&
             json[point + 3] == 'E')) {
          // correcto
          json_root[object_count[stack_level] - 1].type = json_type_boolean;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 1;
          json_root[object_count[stack_level] - 1].content = temp_int;
          json_root[object_count[stack_level] - 1].size = 1;
          point += 3;
          state = s_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'f':
        // false
        if ((json[point + 1] == 'a' && json[point + 2] == 'l' &&
             json[point + 3] == 's' && json[point + 4] == 'e') ||
            (json[point + 1] == 'A' && json[point + 2] == 'L' &&
             json[point + 3] == 'S' && json[point + 4] == 'E')) {
          // correcto
          json_root[object_count[stack_level] - 1].type = json_type_boolean;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 0;
          json_root[object_count[stack_level] - 1].content = temp_int;
          json_root[object_count[stack_level] - 1].size = 1;
          point += 4;
          state = s_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'F':
        // false
        if ((json[point + 1] == 'a' && json[point + 2] == 'l' &&
             json[point + 3] == 's' && json[point + 4] == 'e') ||
            (json[point + 1] == 'A' && json[point + 2] == 'L' &&
             json[point + 3] == 'S' && json[point + 4] == 'E')) {
          // correcto
          json_root[object_count[stack_level] - 1].type = json_type_boolean;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 0;
          json_root[object_count[stack_level] - 1].content = temp_int;
          json_root[object_count[stack_level] - 1].size = 1;
          point += 4;
          state = s_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'n':
        // null
        if ((json[point + 1] == 'u' && json[point + 2] == 'l' &&
             json[point + 3] == 'l') ||
            (json[point + 1] == 'U' && json[point + 2] == 'L' &&
             json[point + 3] == 'L')) {
          // correcto
          json_root[object_count[stack_level] - 1].type = json_type_null;
          json_root[object_count[stack_level] - 1].size = 1;
          state = s_next;
          point += 3;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'N':
        // null
        if ((json[point + 1] == 'u' && json[point + 2] == 'l' &&
             json[point + 3] == 'l') ||
            (json[point + 1] == 'U' && json[point + 2] == 'L' &&
             json[point + 3] == 'L')) {
          // correcto
          json_root[object_count[stack_level] - 1].type = json_type_null;
          json_root[object_count[stack_level] - 1].size = 1;
          state = s_next;
          point += 3;
        } else {
          // erroneo
          return -1;
        }
        break;
      case '[':
        // array
        state = s_array;
        json_root[object_count[stack_level] - 1].type = json_type_array;
        stack_level++;
        object_back = malloc((stack_level + 1) * sizeof *object_back);
        for (i = 0; i < stack_level; i++) {
          object_back[i] = object_count[i];
        }
        object_back[stack_level] = 0;
        free(object_count);
        object_count = object_back;
        break;
      case '{':
        // objeto
        state = s_object_name;
        json_root[object_count[stack_level] - 1].type = json_type_object;
        stack_level++;
        object_back = malloc((stack_level + 1) * sizeof *object_back);
        for (i = 0; i < stack_level; i++) {
          object_back[i] = object_count[i];
        }
        object_back[stack_level] = 0;
        free(object_count);
        object_count = object_back;
        break;
      case '"':
        // string
        i = get_string(json, point, &temp_char, size, &char_size);
        if (i == 0) {
          return -1;
        } else {
          point = i;
          json_root[object_count[stack_level] - 1].content = temp_char;
          json_root[object_count[stack_level] - 1].size = char_size;
          json_root[object_count[stack_level] - 1].type = json_type_string;
          state = s_next;
        }
        break;
      default:
        // numérico
        temp_num = malloc(sizeof *temp_num);
        i = get_number(json, point, temp_num, size);
        if (i == 0) {
          return -1;
        } else {
          point = i;
          json_root[object_count[stack_level] - 1].content = temp_num;
          json_root[object_count[stack_level] - 1].size = 1;
          json_root[object_count[stack_level] - 1].type = json_type_number;
          state = s_next;
        }
        break;
      }
      break;
    case s_next:
      // hasta , o fin }
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case '}':
        end = 1;
        break;
      case ',':
        // comienza
        state = s_name;
        break;
      default:
        // error de ejecución
        return -1;
        break;
      }
      break;
    case s_array:
      // se espera whitespace o valores
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case '"':
        // string
        // guardar el último objeto agrupador
        temp_object = get_last_object(json_root, object_count, stack_level);
        i = get_string(json, point, &temp_char, size, &char_size);
        if (i == 0) {
          return -1;
        }
        point = i;
        json_temp = malloc((object_count[stack_level] + 1) * sizeof *json_temp);
        if (object_count[stack_level] > 0) {
          json_back = temp_object->content;
          for (i = 0; i < object_count[stack_level]; i++) {
            json_temp[i] = json_back[i];
          }
          free(json_back);
        }
        json_temp[object_count[stack_level]].content = temp_char;
        json_temp[object_count[stack_level]].type = json_type_string;
        json_temp[object_count[stack_level]].size = char_size;
        object_count[stack_level]++;
        temp_object->content = json_temp;
        state = s_array_next;
        break;
      case 'T':
        // True
        if ((json[point + 1] == 'r' && json[point + 2] == 'u' &&
             json[point + 3] == 'e') ||
            (json[point + 1] == 'R' && json[point + 2] == 'U' &&
             json[point + 3] == 'E')) {
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 1;
          json_temp[object_count[stack_level]].content = temp_int;
          json_temp[object_count[stack_level]].type = json_type_boolean;
          json_temp[object_count[stack_level]].size = 1;
          object_count[stack_level]++;
          temp_object->content = json_temp;
          point += 3;
          state = s_array_next;
        } else {
          return -1;
        }
        break;
      case 't':
        // True
        if ((json[point + 1] == 'r' && json[point + 2] == 'u' &&
             json[point + 3] == 'e') ||
            (json[point + 1] == 'R' && json[point + 2] == 'U' &&
             json[point + 3] == 'E')) {
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 1;
          json_temp[object_count[stack_level]].content = temp_int;
          json_temp[object_count[stack_level]].type = json_type_boolean;
          json_temp[object_count[stack_level]].size = 1;
          object_count[stack_level]++;
          temp_object->content = json_temp;
          point += 3;
          state = s_array_next;
        } else {
          return -1;
        }
        break;
      case 'f':
        // False
        if ((json[point + 1] == 'a' && json[point + 2] == 'l' &&
             json[point + 3] == 's' && json[point + 4] == 'e') ||
            (json[point + 1] == 'A' && json[point + 2] == 'L' &&
             json[point + 3] == 'S' && json[point + 4] == 'E')) {
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 0;
          json_temp[object_count[stack_level]].content = temp_int;
          json_temp[object_count[stack_level]].type = json_type_boolean;
          json_temp[object_count[stack_level]].size = 1;
          object_count[stack_level]++;
          temp_object->content = json_temp;
          point += 4;
          state = s_array_next;
        } else {
          return -1;
        }
        break;
      case 'F':
        // False
        if ((json[point + 1] == 'a' && json[point + 2] == 'l' &&
             json[point + 3] == 's' && json[point + 4] == 'e') ||
            (json[point + 1] == 'A' && json[point + 2] == 'L' &&
             json[point + 3] == 'S' && json[point + 4] == 'E')) {
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 0;
          json_temp[object_count[stack_level]].content = temp_int;
          json_temp[object_count[stack_level]].type = json_type_boolean;
          json_temp[object_count[stack_level]].size = 1;
          object_count[stack_level]++;
          temp_object->content = json_temp;
          point += 4;
          state = s_array_next;
        } else {
          return -1;
        }
        break;
      case 'n':
        // Null
        if ((json[point + 1] == 'u' && json[point + 2] == 'l' &&
             json[point + 3] == 'l') ||
            (json[point + 1] == 'U' && json[point + 2] == 'L' &&
             json[point + 3] == 'L')) {
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          json_temp[object_count[stack_level]].type = json_type_null;
          json_temp[object_count[stack_level]].size = 1;
          object_count[stack_level]++;
          temp_object->content = json_temp;
          point += 3;
          state = s_array_next;
        } else {
          return -1;
        }
        break;
      case 'N':
        // Null
        if ((json[point + 1] == 'u' && json[point + 2] == 'l' &&
             json[point + 3] == 'l') ||
            (json[point + 1] == 'U' && json[point + 2] == 'L' &&
             json[point + 3] == 'L')) {
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          json_temp[object_count[stack_level]].type = json_type_null;
          json_temp[object_count[stack_level]].size = 1;
          object_count[stack_level]++;
          temp_object->content = json_temp;
          point += 3;
          state = s_array_next;
        } else {
          return -1;
        }
        break;
      case ']':
        // fin del array
        temp_object = get_last_object(json_root, object_count, stack_level);
        temp_object->size = object_count[stack_level];
        stack_level--;
        if (stack_level == 0) {
          state = s_name;
        } else {
          temp_object = get_last_object(json_root, object_count, stack_level);
          switch (temp_object->type) {
          case json_type_object:
            state = s_object_next;
            break;
          case json_type_array:
            state = s_array;
            break;
          default:
            return -1;
            break;
          }
        }
        break;
      case '[':
        // array
        state = s_array;
        temp_object = get_last_object(json_root, object_count, stack_level);
        json_temp = malloc((object_count[stack_level] + 1) * sizeof *json_temp);
        if (object_count[stack_level] > 0) {
          json_back = temp_object->content;
          for (i = 0; i < object_count[stack_level]; i++) {
            json_temp[i] = json_back[i];
          }
          free(json_back);
        }
        json_temp[object_count[stack_level]].type = json_type_array;
        temp_object->content = json_temp;
        object_count[stack_level]++;
        stack_level++;
        object_back = malloc((stack_level + 1) * sizeof *object_back);
        for (i = 0; i < stack_level; i++) {
          object_back[i] = object_count[i];
        }
        object_back[stack_level] = 0;
        free(object_count);
        object_count = object_back;
        break;
      case '{':
        // objeto
        state = s_object_name;
        temp_object = get_last_object(json_root, object_count, stack_level);
        json_temp = malloc((object_count[stack_level] + 1) * sizeof *json_temp);
        if (object_count[stack_level] > 0) {
          json_back = temp_object->content;
          for (i = 0; i < object_count[stack_level]; i++) {
            json_temp[i] = json_back[i];
          }
          free(json_back);
        }
        json_temp[object_count[stack_level]].type = json_type_object;
        temp_object->content = json_temp;
        object_count[stack_level]++;
        stack_level++;
        object_back = malloc((stack_level + 1) * sizeof *object_back);
        for (i = 0; i < stack_level; i++) {
          object_back[i] = object_count[i];
        }
        object_back[stack_level] = 0;
        free(object_count);
        object_count = object_back;
        break;
      default:
        // solo OK si es numérico o -
        if ((json[point] < '0' || json[point] > '9') && json[point] != '-') {
          return -1;
        } else {
          // guardar el último objeto agrupador
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          temp_num = malloc(sizeof *temp_num);
          i = get_number(json, point, temp_num, size);
          if (i == 0) {
            return -1;
          }
          point = i;
          json_temp[object_count[stack_level]].content = temp_num;
          json_temp[object_count[stack_level]].type = json_type_number;
          json_temp[object_count[stack_level]].size = 1;
          object_count[stack_level]++;
          temp_object->content = json_temp;
          state = s_array_next;
          break;
        }
        break;
      }
      break;
    case s_array_next:
      // hasta , o fin ]
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case ']':
        // fin del array
        temp_object = get_last_object(json_root, object_count, stack_level);
        temp_object->size = object_count[stack_level];
        stack_level--;
        if (stack_level == 0) {
          state = s_next;
        } else {
          temp_object = get_last_object(json_root, object_count, stack_level);
          switch (temp_object->type) {
          case json_type_object:
            state = s_object_next;
            break;
          case json_type_array:
            state = s_array_next;
            break;
          default:
            return -1;
            break;
          }
        }
        break;
      case ',':
        // comienza
        state = s_array;
        break;
      default:
        // error de ejecución
        return -1;
        break;
      }
      break;
    case s_object_name:
      // whitespace o "
      // cargando nombre. cualquier caracter cuidado con los especiales
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case '}':
        // fin del objeto
        if (object_count[stack_level] > 0) {
          return -1;
        }
        temp_object = get_last_object(json_root, object_count, stack_level);
        temp_object->size = object_count[stack_level];
        stack_level--;
        if (stack_level == 0) {
          state = s_next;
        } else {
          temp_object = get_last_object(json_root, object_count, stack_level);
          switch (temp_object->type) {
          case json_type_object:
            state = s_object_next;
            break;
          case json_type_array:
            state = s_array_next;
            break;
          default:
            return -1;
            break;
          }
        }
        break;
      case '"':
        i = get_string(json, point, &temp_char, size, &char_size);
        if (i == 0) {
          return -1;
        } else {
          temp_object = get_last_object(json_root, object_count, stack_level);
          point = i;
          json_back = temp_object->content;
          json_temp =
              malloc((object_count[stack_level] + 1) * sizeof *json_temp);
          if (object_count[stack_level] > 0) {
            json_back = temp_object->content;
            for (i = 0; i < object_count[stack_level]; i++) {
              json_temp[i] = json_back[i];
            }
            free(json_back);
          }
          json_temp[object_count[stack_level]].name = temp_char;
          json_temp[object_count[stack_level]].name_size = char_size;
          temp_object->content = json_temp;
          object_count[stack_level]++;
          state = s_object_nameend;
        }
        break;
      default:
        return -1;
        break;
      }
      break;
    case s_object_nameend:
      // esperar : para buscar tipo (witespace o :)
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case ':':
        // detectar tipo
        state = s_object_type;
        break;
      default:
        // error de ejecución
        return -1;
        break;
      }
      break;
    case s_object_type:
      // witespace o inicio de tipo
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case 't':
        // true
        if ((json[point + 1] == 'r' && json[point + 2] == 'u' &&
             json[point + 3] == 'e') ||
            (json[point + 1] == 'R' && json[point + 2] == 'U' &&
             json[point + 3] == 'E')) {
          // correcto
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 1;
          json_temp[object_count[stack_level] - 1].content = temp_int;
          json_temp[object_count[stack_level] - 1].type = json_type_boolean;
          json_temp[object_count[stack_level] - 1].size = 1;
          point += 3;
          state = s_object_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'T':
        // true
        if ((json[point + 1] == 'r' && json[point + 2] == 'u' &&
             json[point + 3] == 'e') ||
            (json[point + 1] == 'R' && json[point + 2] == 'U' &&
             json[point + 3] == 'E')) {
          // correcto
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 1;
          json_temp[object_count[stack_level] - 1].content = temp_int;
          json_temp[object_count[stack_level] - 1].type = json_type_boolean;
          json_temp[object_count[stack_level] - 1].size = 1;
          point += 3;
          state = s_object_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'f':
        // false
        if ((json[point + 1] == 'a' && json[point + 2] == 'l' &&
             json[point + 3] == 's' && json[point + 4] == 'e') ||
            (json[point + 1] == 'A' && json[point + 2] == 'L' &&
             json[point + 3] == 'S' && json[point + 4] == 'E')) {
          // correcto
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 0;
          json_temp[object_count[stack_level] - 1].content = temp_int;
          json_temp[object_count[stack_level] - 1].type = json_type_boolean;
          json_temp[object_count[stack_level] - 1].size = 1;
          point += 4;
          state = s_object_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'F':
        // false
        if ((json[point + 1] == 'a' && json[point + 2] == 'l' &&
             json[point + 3] == 's' && json[point + 4] == 'e') ||
            (json[point + 1] == 'A' && json[point + 2] == 'L' &&
             json[point + 3] == 'S' && json[point + 4] == 'E')) {
          // correcto
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          temp_int = malloc(sizeof *temp_int);
          temp_int[0] = 0;
          json_temp[object_count[stack_level] - 1].content = temp_int;
          json_temp[object_count[stack_level] - 1].type = json_type_boolean;
          json_temp[object_count[stack_level] - 1].size = 1;
          point += 4;
          state = s_object_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'n':
        // null
        if ((json[point + 1] == 'u' && json[point + 2] == 'l' &&
             json[point + 3] == 'l') ||
            (json[point + 1] == 'U' && json[point + 2] == 'L' &&
             json[point + 3] == 'L')) {
          // correcto
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          json_temp[object_count[stack_level] - 1].type = json_type_null;
          json_temp[object_count[stack_level] - 1].size = 1;
          point += 3;
          state = s_object_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case 'N':
        // null
        if ((json[point + 1] == 'u' && json[point + 2] == 'l' &&
             json[point + 3] == 'l') ||
            (json[point + 1] == 'U' && json[point + 2] == 'L' &&
             json[point + 3] == 'L')) {
          // correcto
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          json_temp[object_count[stack_level] - 1].type = json_type_null;
          json_temp[object_count[stack_level] - 1].size = 1;
          point += 3;
          ;
          state = s_object_next;
        } else {
          // erroneo
          return -1;
        }
        break;
      case '[':
        // array
        state = s_array;
        temp_object = get_last_object(json_root, object_count, stack_level);
        json_temp = temp_object->content;
        json_temp[object_count[stack_level] - 1].type = json_type_array;
        stack_level++;
        object_back = malloc((stack_level + 1) * sizeof *object_back);
        for (i = 0; i < stack_level; i++) {
          object_back[i] = object_count[i];
        }
        object_back[stack_level] = 0;
        free(object_count);
        object_count = object_back;
        break;
      case '{':
        // objeto
        state = s_object_name;
        temp_object = get_last_object(json_root, object_count, stack_level);
        json_temp = temp_object->content;
        json_temp[object_count[stack_level] - 1].type = json_type_object;
        stack_level++;
        object_back = malloc((stack_level + 1) * sizeof *object_back);
        for (i = 0; i < stack_level; i++) {
          object_back[i] = object_count[i];
        }
        object_back[stack_level] = 0;
        free(object_count);
        object_count = object_back;
        break;
      case '"':
        // string
        i = get_string(json, point, &temp_char, size, &char_size);
        if (i == 0) {
          return -1;
        } else {
          point = i;
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          json_temp[object_count[stack_level] - 1].type = json_type_string;
          json_temp[object_count[stack_level] - 1].size = char_size;
          json_temp[object_count[stack_level] - 1].content = temp_char;
          state = s_object_next;
        }
        break;
      default:
        // numérico
        temp_num = malloc(sizeof *temp_num);
        i = get_number(json, point, temp_num, size);
        if (i == 0) {
          return -1;
        } else {
          point = i;
          temp_object = get_last_object(json_root, object_count, stack_level);
          json_temp = temp_object->content;
          json_temp[object_count[stack_level] - 1].type = json_type_number;
          json_temp[object_count[stack_level] - 1].size = 1;
          json_temp[object_count[stack_level] - 1].content = temp_num;
          state = s_object_next;
        }
        break;
      }
      break;
    case s_object_next:
      // hasta , o fin }
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case ',':
        state = s_object_name;
        break;
      case '}':
        // fin del array
        temp_object = get_last_object(json_root, object_count, stack_level);
        temp_object->size = object_count[stack_level];
        stack_level--;
        if (stack_level == 0) {
          state = s_next;
        } else {
          temp_object = get_last_object(json_root, object_count, stack_level);
          switch (temp_object->type) {
          case json_type_object:
            state = s_object_next;
            break;
          case json_type_array:
            state = s_array_next;
            break;
          default:
            return -1;
            break;
          }
        }
        break;
      }
      break;
    default:
      return -1;
    }
  }
  object->content = json_root;
  object->size = object_count[0];
  return 0;
}

struct object *get_last_object(struct object *root, unsigned int *object_count,
                               unsigned short stack_level) {
  int i;
  struct object *act_object = root;
  for (i = 0; i < (stack_level - 1); i++) {
    act_object = act_object[object_count[i] - 1].content;
  }
  return &act_object[object_count[stack_level - 1] - 1];
}

unsigned int get_number(char *json, unsigned int point, double *val,
                        unsigned int max) {
  unsigned short end = 0;
  char *temp_char;
  char *back_char;
  unsigned short size = 0;
  char *eptr;
  unsigned short i;
  temp_char = malloc(sizeof *temp_char);
  unsigned short state = 0; // 0-no start 1-number 2-decimal 3-exp
  while (point < max) {
    switch (json[point]) {
    // whitespace o valores
    case ' ':
      if (state != 0)
        end = 1;
      break;
    case '\n':
      if (state != 0)
        end = 1;
      break;
    case '\r':
      if (state != 0)
        end = 1;
      break;
    case '\t':
      if (state != 0)
        end = 1;
      break;
    case ',':
      if (state != 0)
        end = 1;
      break;
    case ']':
      if (state != 0)
        end = 1;
      break;
    case '}':
      if (state != 0)
        end = 1;
      break;
    case '-':
      if (state != 0) {
        // salimos pero con error
        end = 1;
        size = 0;
      } else {
        size = 1;
        temp_char[0] = '-';
        state = 1;
      }
      break;
    case '.':
      if (state != 1 || (size == 1 && temp_char[0] == '-')) {
        // situaciones en las que se acepta un .
        return 0;
      } else {
        back_char = temp_char;
        temp_char = malloc((size + 1) * sizeof(char));
        for (i = 0; i < size; i++) {
          temp_char[i] = back_char[i];
        }
        temp_char[size] = json[point];
        size++;
        free(back_char);
        state = 2;
      }
      break;
    case 'e':
      if (state == 0 || state == 3 || (size == 1 && temp_char[0] == '-')) {
        return 0;
      } else {
        if (json[point + 1] != '+' && json[point + 1] != '-' &&
            (json[point - 1] < '0' || json[point - 1] > '9')) {
          // fuera de rango tras la e
          return 0;
        } else {
          back_char = temp_char;
          temp_char = malloc((size + 2) * sizeof(char));
          for (i = 0; i < size; i++) {
            temp_char[i] = back_char[i];
          }
          temp_char[size] = json[point];
          temp_char[size + 1] = json[point + 1];
          size += 2;
          point++;
          free(back_char);
          state = 3;
        }
      }
      break;
    case 'E':
      if (state == 0 || state == 3 || (size == 1 && temp_char[0] == '-')) {
        return 0;
      } else {
        if (json[point + 1] != '+' && json[point + 1] != '-' &&
            (json[point - 1] < '0' || json[point - 1] > '9')) {
          // fuera de rango tras la e
          return 0;
        } else {
          back_char = temp_char;
          temp_char = malloc((size + 2) * sizeof(char));
          for (i = 0; i < size; i++) {
            temp_char[i] = back_char[i];
          }
          temp_char[size] = json[point];
          temp_char[size + 1] = json[point + 1];
          size += 2;
          point++;
          free(back_char);
          state = 3;
        }
      }
      break;
    default:
      // debe ser numérico
      if (json[point] < '0' || json[point] > '9') {
        return 0;
      } else {
        back_char = temp_char;
        temp_char = malloc((size + 1) * sizeof(char));
        for (i = 0; i < size; i++) {
          temp_char[i] = back_char[i];
        }
        temp_char[size] = json[point];
        size++;
        free(back_char);
        if (state == 0) {
          state = 1;
        }
      }
      break;
    }
    if (end == 1) {
      break;
    } else {
      point++;
    }
  }
  if ((end == 0 && json[point] != ',') || size == 0) {
    // No se ha terminado correctamente
    return 0;
  }
  errno = 0;
  val[0] = strtod(temp_char, &eptr);
  free(temp_char);
  /* If the result is 0, test for an error */
  if (val[0] == 0) {
    /* If the value provided was out of range, display a warning message */
    if (errno == ERANGE) {
      return 0;
    }
  }
  return (point - 1);
}

unsigned int get_string(char *json, unsigned int point, char **val,
                        unsigned int max, unsigned int *s_sice) {
  char *temp_char;
  char *back_char;
  unsigned short size = 0;
  unsigned short i;
  char x_char;
  char hex_char[4];
  unsigned short state = 0; // 0-start 1-leters
  temp_char = malloc(sizeof *temp_char);

  while (point < max) {
    if (state == 0) {
      switch (json[point]) {
      case ' ':
        break;
      case '\n':
        break;
      case '\r':
        break;
      case '\t':
        break;
      case '"':
        state = 1;
        break;
      default:
        return 0;
        break;
      }
    } else {
      // cuidado con los \ es un string de JSON
      if (json[point] == '\\') {
        point++;
        switch (json[point]) {
        case '"':
          x_char = '"';
          break;
        case '/':
          x_char = '/';
          break;
        case '\\':
          x_char = '\\';
          break;
        case 'b':
          x_char = '\b';
          break;
        case 'f':
          x_char = '\f';
          break;
        case 'n':
          x_char = '\n';
          break;
        case 'r':
          x_char = '\r';
          break;
        case 't':
          x_char = '\t';
          break;
        case 'u':
          hex_char[0] = json[point + 1];
          hex_char[1] = json[point + 2];
          hex_char[2] = json[point + 3];
          hex_char[3] = json[point + 4];
          point += 4;
          x_char = strtol(hex_char, 0, 16);
          break;
        default:
          // no es un comando
          return 0;
        }
        back_char = temp_char;
        temp_char = malloc((size + 1) * sizeof *temp_char);
        for (i = 0; i < size; i++) {
          temp_char[i] = back_char[i];
        }
        free(back_char);
        temp_char[size] = x_char;
        size++;
      } else {
        if (json[point] == '"') {
          break;
        }
        back_char = temp_char;
        temp_char = malloc((size + 1) * sizeof *temp_char);
        for (i = 0; i < size; i++) {
          temp_char[i] = back_char[i];
        }
        free(back_char);
        temp_char[size] = json[point];
        size++;
      }
    }
    point++;
  }
  val[0] = temp_char;
  *s_sice = size;
  return point;
}

int json_get_item_by_name(struct object root, char *name) {
  int i;
  struct object *json_temp = root.content;
  for (i = 0; i < root.size; i++) {
    if (strncmp(json_temp[i].name, name,json_temp[i].name_size) == 0 && strlen(name) == json_temp[i].name_size) {
      return i;
    }
  }
  return -1;
}

unsigned short json_get_item_type(struct object root) { return root.type; }

int json_get_string(struct object root, char **str) {
  char *temp;
  if (root.type != json_type_string) {
    return -1;
  }
  if (root.size > 0) {
    *str = malloc(root.size * sizeof *temp);
    temp = root.content;
    strcpy(*str,temp);
  }
  return root.size;
}

double json_get_number(struct object root){
  double *temp;
  if(root.type != json_type_number || root.size != 1){
    errno = ERANGE;
    return 0;
  }
  errno = 0;
  temp = root.content;
  return *temp;  
}

short json_get_boolean(struct object root){
  char * temp;
  if(root.type != json_type_boolean || root.size != 1){
    return -1;
  }
  temp = root.content;
  return (short)*temp;
}
