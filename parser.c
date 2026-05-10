#include "includes.h"
#include "main.h"

void append_var(struct variable *var, struct variable *new) {
    if (var->next == NULL) {
        var->next = new;
        return;
    }
    else { append_var(var->next,new); }
}

void append_rule(struct rule *rule, struct rule *new) {
    if (rule->next == NULL) {
        rule->next = new;
        return;
    }
    else { append_rule(rule->next,new); }
}

void append_command(struct rule* rule, char* cmd) {
    int i = 0;
    while (rule->commands[i] != NULL) {
        i++;
    }
    rule->commands[i] = strdup(cmd);
    rule->commands[i+1] = NULL;
}

char *get_value_var(char *line, int index) {
    int i = index;
    while (line[i] == ' ' || line[i] == '\t') { i++; }

    char *str = malloc(strlen(line) - i + 1);
    int si = 0;

    while (line[i] != '\0' && line[i] != '\n') {
        str[si++] = line[i++];
    }
    
    str[si] = '\0';
    return str;
}

struct rule* get_latest_rule(struct rule *rule) {
    while (rule->next != NULL) { rule = rule->next; }
    return rule;
}

char** get_deps_rule(char* line, int index, struct variable *vars) {
    char **list = malloc(strlen(line) * sizeof(char *));
    char* dep = malloc(sizeof(char) * strlen(line));
    list[0] = NULL;
    int i = 0;
    int di = 0;
    int len = 0;

    while (line[i + index] != '\0' && line[i + index] != '\n') {
        if (line[i + index] == ' ' || line[i + index + 1] == '\0' || line[i + index + 1] == '\n') {
            if (line[i + index] != ' ') { dep[di] = line[i + index]; di++; }
            if (di > 0) {
                dep[di] = '\0';
                char *raw = strdup(dep);
                char *expanded = expand(raw, vars);
                free(raw);
                if (expanded[0] != '\0') { list[len++] = expanded; }
                else { free(expanded); }
                free(dep);
                dep = malloc(sizeof(char) * strlen(line));
                di = 0;
            }
        }
        else {
            dep[di] = line[i + index];
            di++;
        } 
        i++;
    }

    list[len] = NULL;
    free(dep);
    return list;
}

char *find_var(char *var_name, struct variable *vars) {
    while (vars != NULL) {
        if (vars->name != NULL && strcmp(var_name, vars->name) == 0) {
            return vars->value;
        }
        vars = vars->next;
    }
    return NULL;
}

char *expand(char *input, struct variable *vars) {
    size_t buf = strlen(input) + 1;
    size_t len = 0;
    char *out = calloc(buf,sizeof(char));
    out[0] = '\0';

    size_t i = 0;
    while (input[i] != '\0') {
        if (input[i] == '$' && input[i+1] == '{') {
            size_t name_start = i + 2;
            size_t j = name_start;
            
            while (input[j] != '\0' && input[j] != '}') { j++; }
            if (input[j] != '}') {
                free(out);
                return NULL;
            }

            size_t name_len = j - name_start;
            char *name = strndup(input + name_start, name_len);
            char *value = find_var(name, vars);
            free(name);
            if (value == NULL) { value = ""; }

            size_t vlen = strlen(value);
            while (len + vlen + 1 > buf) {
                buf *= 2;
                out = realloc(out, buf * sizeof(char));
            }

            memcpy(out + len, value, vlen);
            len = len + vlen;
            i = j + 1;
        } 
        else {
            if (len + 2 > buf) {
                buf *= 2;
                out = realloc(out, buf * sizeof(char));
            }
            out[len] = input[i];
            len++;
            i++;
        }
    }
    
    out[len] = '\0';
    return out;
}

void parse_line(char* line, struct rule *rules, struct variable *vars, bool *is_cmd) {
    if (sizeof(line) < 1) { return; }

    char* token = malloc(sizeof(char) * strlen(line));

    int i = 0;
    int ti = 0;

    if (isblank(line[0]) && *is_cmd) {
        char *cmd = line;
        while (isblank(*cmd)) cmd++;
        struct rule *latest_rule = get_latest_rule(rules);
        append_command(latest_rule, cmd);
        free(token);
        return;
    }

    *is_cmd = false;

    while (line[i] != '\0') {
        if (line[i] == ':') {
            token[ti] = '\0';
            char *target = expand(token, vars);
            if (rules->name == NULL) {
                rules->name = target;
                rules->depends = get_deps_rule(line,i+1,vars);
                rules->commands = malloc(sizeof(char *) * strlen(line));
                rules->commands[0] = NULL;
                rules->next = NULL;
                //printf("Parsed rule 1\n");
            }
            else {
                struct rule *nrule = malloc(sizeof(struct rule));
                nrule->name = target;
                nrule->depends = get_deps_rule(line,i+1,vars);
                nrule->commands = malloc(sizeof(char *) * strlen(line));
                nrule->commands[0] = NULL;
                nrule->next = NULL;
                append_rule(rules,nrule);
                //printf("Parsed rule 2\n");
            }
            
            *is_cmd = true;
            free(token);
            token = malloc(sizeof(char) * strlen(line));
            ti = 0;
        }
        else if (line[i] == '=') {
            token[ti] = '\0';
            char *name = expand(token, vars);
            char *raw_value = get_value_var(line, i+1);
            char *value = expand(raw_value, vars);
            free(raw_value);

            if (vars->name == NULL) {
                vars->name = name;
                vars->value = value;
                vars->next = NULL;
            }
            else {
                struct variable *nvar = calloc(1, sizeof(struct variable));
                nvar->name = name;
                nvar->value = value;
                append_var(vars, nvar);
            }
            free(token);
            token = malloc(sizeof(char) * strlen(line));
            ti = 0;
        }
        else if (line[i] == '#') { 
            free(token);
            return;
        }
        else {
            if (!isblank(line[i])) { token[ti] = line[i]; ti++; }
        }
        i++;
    }
    free(token);
}