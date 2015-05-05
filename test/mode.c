#include "test.h"


int test_mode(void)
{
    char *str;

    {
        enum mode m1, *m2;

        m1 = SINGLESTEP;

        str = serialize_mode(&m1);
        m2 = deserialize_mode(str);

        assert("singlestep equivalence", m1 == *m2);

        free(m2);
        free(str);
    }

    return 0;
}
