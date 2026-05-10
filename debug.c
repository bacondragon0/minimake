#include "includes.h"

void print_var(struct variable *var) {
    while (var != NULL) {
        if (var->name != NULL) { printf("'%s' = '%s'\n", var->name, var->value); }
        var = var->next;
    }
}

void print_rule(struct rule *rule) {
    int i = 0;
    if (!rule) { printf("\n"); return; }

    if (rule->name != NULL && rule->name[0] != '\0') {
        printf("(%s):", rule->name);

        while (rule->depends[i] != NULL) {
            printf(" [%s]", rule->depends[i]);
            i++;
        }

        i = 0;
        while (rule->commands[i] != NULL) {
            printf("\n\t'%s'", rule->commands[i]);
            i++;
        }
        printf("\n");
    }
    if (rule->next != NULL) { print_rule(rule->next); }
}