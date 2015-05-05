#include <string.h>

#include "test.h"


int test_program(void)
{
    char *str;

    {
        enum mode m;
        struct regime r;
        struct program p1, *p2;
        bool failed;

        m = SINGLESTEP;

        r.mode = m;
        r.address = 5;
        r.period = 110;
        r.instructions = 1000;
        r.dim = 9;

        strcpy(p1.name, "hello world");
        p1.any_regime = true;
        p1.regime = r;

        str = serialize_program(&p1);
        p2 = deserialize_program(str);

        failed = p2 == NULL;

        assert("deserializing doesn't fail", !failed);

        assert("mode equivalence",
               !failed && r.mode == p2->regime.mode);

        assert("address equivalence",
               !failed && r.address == p2->regime.address);

        assert("period equivalence",
               !failed && r.period == p2->regime.period);

        assert("instructions equivalence",
               !failed && r.instructions ==
                          p2->regime.instructions);

        assert("dim equivalence",
               !failed && r.dim == p2->regime.dim);

        assert("any_regime equivalence",
               !failed && p1.any_regime == p2->any_regime);

        assert("program name equivalence",
               !failed && strcmp(p1.name, p2->name) == 0);

        free(p2);
        free(str);
    }

    return 0;
}
