#include "includes.h"
#include "parser.c"
#include "debug.c"
#include "execution.c"

int read_file(char *filepath, struct rule *rules, struct variable *vars) {
    FILE *file = fopen(filepath, "r");
    if (!file) { return 3; }
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    bool is_cmd = false;

    while ((len = getline(&line, &cap, file)) != -1) {
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            len--; 
            line[len] = '\0';
        }
        parse_line(line, rules, vars, &is_cmd);
    }

    free(line);
    fclose(file);
    return 0;
}

void free_var_list(struct variable *v) {
    while (v != NULL) {
        struct variable *next = v->next;
        free(v->name);
        free(v->value);
        free(v);
        v = next;
    }
}

void free_rule_list(struct rule *r) {
    while (r != NULL) {
        struct rule *next = r->next;
        free(r->name);
        if (r->depends != NULL) {
            for (int i = 0; r->depends[i] != NULL; i++) {
                free(r->depends[i]);
            }
            free(r->depends);
        }
        if (r->commands != NULL) {
            for (int i = 0; r->commands[i] != NULL; i++)
                free(r->commands[i]);
            free(r->commands);
        }
        free(r);
        r = next;
    }
}

void print_help() {
    printf("Usage: minimake [OPTIONS] [TARGETS]\n");
    printf("    OPTIONS can be:\n");
    printf("        \"-h\"            to display the helper\n");
    printf("        \"-p\"            to print the rules and variables\n");
    printf("        \"-f\" <filename> to change the make config file, \"Makefile\" by default\n");
    printf("    TARGETS are target name of makefile rule to execute\n");
}  

int main(int argc, char* argv[]) {

    if (argc < 1) {
        return 2;
    }

    bool print_mode = false;
    bool help_mode = false;
    char *makefile_path = "Makefile";
    char **targets = malloc(argc * sizeof(char*));
    int target_count = 0;   

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) { help_mode = true; }
        else if (strcmp(argv[i], "-p") == 0) { print_mode = true; }
        else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 >= argc) {
                free(targets);
                return 2;
            }
            makefile_path = argv[i+1];
            i++;
        }
        else {
            targets[target_count] = argv[i];
            target_count++;
        }
    }

    if (help_mode) { 
        print_help();

        free(targets);
        return 0;
    }

    struct rule *rules = calloc(1, sizeof(struct rule));
    struct variable *vars = calloc(1, sizeof(struct variable));

    if (read_file(makefile_path, rules, vars) != 0) {
        free(targets);
        free_rule_list(rules);
        free_var_list(vars);
        return 3;
    }

    if (print_mode) {
        printf("# variables\n");
        print_var(vars);
        printf("\n");
        printf("# rules\n");
        print_rule(rules);
    }
    else {
        if (target_count == 0) {
            if (rules->name == NULL) {
                printf("minimake: *** No targets.  Stop.\n");
                free(targets);
                free_rule_list(rules);
                free_var_list(vars);
                return 2;
            }
            build(rules->name, NULL, rules, vars);
        }
        else {
            for (int i = 0; i < target_count; i++) {
                build(targets[i], NULL, rules, vars);
            }
        }
    }

    free_rule_list(rules);
    free_var_list(vars);
    free(targets);

    return 0;
}