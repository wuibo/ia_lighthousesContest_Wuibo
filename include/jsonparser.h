#define json_type_string 1
#define json_type_number 2
#define json_type_object 3
#define json_type_array 4
#define json_type_boolean 5
#define json_type_null 6


struct object {
  unsigned short type;
  unsigned int size;
  unsigned int name_size;
  char * name;
  void * content;
};

int read_json(struct object *object, char * json,unsigned int size);
struct object *get_last_object(struct object *root, unsigned int *object_count, unsigned short stack_level);
unsigned int get_number(char *json,unsigned int point,double *val,unsigned int max);
unsigned int get_string(char *json,unsigned int point,char **val,unsigned int max,unsigned int *s_sice);
int json_get_item_by_name(struct object root,char *name);
unsigned short json_get_item_type(struct object root);
int json_get_string(struct object root, char **str);
double json_get_number(struct object root);
short json_get_boolean(struct object root);
