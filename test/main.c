#include "test.h"


int num_failures = 0;
int num_errors = 0;

void assert(char *str, bool cond)
{
    if (cond) {
        printf("passed: %s\n", str);
    } else {
        printf("failed: %s\n", str);
        num_failures++;
    }
}

void test_error(char *str)
{
    fprintf(stderr, "error: %s\n", str);
    num_errors++;
}

void info(char *str)
{
    printf("*** %s\n", str);
}

int main(int argc, char *argv[])
{
    info("testing field functions");
    if (test_field() < 0)
        return 1;

    info("testing mode functions");
    if (test_mode() < 0)
        return 1;

    info("testing regime functions");
    if (test_regime() < 0)
        return 1;

    info("testing program functions");
    if (test_program() < 0)
        return 1;

    info("testing learning_data functions");
    if (test_learning_data() < 0)
        return 1;

    printf("completed with %d failure(s) and %d error(s)\n",
           num_failures, num_errors);

    return num_failures;
}
