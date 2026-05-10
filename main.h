
struct variable {
    char* name;
    char* value;
    struct variable* next;
};

struct rule {
    char* name;
    char** depends;
    char** commands;
    struct rule* next;
};

void parse_line(char* line, struct rule *rules, struct variable *vars, bool *is_cmd);
char* expand(char* line, struct variable *vars);