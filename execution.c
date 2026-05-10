#include "includes.h"

struct rule *find_rule(char *rule_name, struct rule *rules) {
    while (rules != NULL) {
        if (rules->name != NULL && strcmp(rule_name, rules->name) == 0) {
            return rules;
        }
        rules = rules->next;
    }
    return NULL;
}

int build(char* target, char *origin, struct rule *rules, struct variable *vars) {
    
    struct rule *target_rule = find_rule(target,rules);
    if (!target_rule) {
        if (origin != NULL) { printf("minimake: *** No rule to make target '%s', needed by '%s'.  Stop.\n",target, origin); }
        else { printf("minimake: *** No rule to make target '%s'.  Stop.\n", target); }
        return 2;
    }

    int i = 0;
    while (target_rule->depends[i] != NULL) {
        if (build(target_rule->depends[i], target, rules, vars) == 2) { return 2; }
        i++;
    }

    int j = 0;
    while (target_rule->commands[j] != NULL) {
        char *raw = target_rule->commands[j];
        char *expanded = expand(raw, vars);
        printf("%s\n", expanded);
        fflush(stdout);

        pid_t pid = fork();
        if (pid == -1) {
            free(expanded);
            return 2;
        }

        if (pid == 0) {
            execlp("/bin/sh", "sh", "-c", expanded, NULL);
            exit(1);
        }

        int status;
        waitpid(pid, &status, 0);
        free(expanded);
        j++;
    }

    return 0;
}

