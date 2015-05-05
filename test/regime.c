#include "test.h"


int test_regime(void)
{
    char *str;

    {
        enum mode m;
        struct regime r1, *r2;

        m = BLOCKSTEP;

        r1.mode = m;
        r1.address = 6;
        r1.period = 100;
        r1.instructions = 99;
        r1.dim = 3;

        str = serialize_regime(&r1);
        r2 = deserialize_regime(str, NULL);

        assert("mode equivalence", r1.mode == r2->mode);
        assert("address equivalence", r1.address == r2->address);
        assert("period equivalence", r1.period == r2->period);
        assert("instructions equivalence",
               r1.instructions == r2->instructions);
        assert("dim equivalence", r1.dim == r2->dim);

        free(r2);
        free(str);
    }

    return 0;
}
