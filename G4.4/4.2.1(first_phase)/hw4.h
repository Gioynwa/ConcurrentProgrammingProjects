struct args {
    char name[100];
    char content[100];
    int pointer;
    int val;
};

struct list {
    int nofpos;
    int flag_sem;
    char name_of_sem[20];
    struct list *next;
    struct list *prev;
    
};

struct variables {
    char name[100];
    int val;
};

struct labels {
    char name[100];
    off_t pos;
};

struct list* node;
volatile struct variables *global_var, *threads_info;
int search_args(char* str, struct args* array, int size);
int search(char *command);
int search_varname(char* str, struct variables* array, int size);
int search_labels(char* str, struct labels* array, int size);
struct list* init(struct list* head);
void add(struct list* head, struct list* curr, int position);
void remove_node(struct list* curr);

struct multithreading {
    
    int going_to_label;
    int first_time;
    int flag;
    int counter_args;
    int counter_local;
    int counter_labels;
    char str_label[20];
    struct variables *local_var;
    struct labels *label_array;
    struct args *arg_array;
    
};
