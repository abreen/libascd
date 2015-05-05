#include <string.h>

#include "test.h"


int test_field(void)
{
    {
        struct field f;

        f = read_field("foo=bar");

        assert("key is 'foo'", strcmp(f.key, "foo") == 0);
        assert("value is 'bar'", strcmp(f.val, "bar") == 0);
    }

    {
        struct field f;
        char buf[256];
        char *p;
        const char *field;
        int len;
        int i;

        f.key = "myfoo";
        f.val = "mybar";

        field = "myfoo=mybar";
        len = strlen(field);

        p = write_field(buf, f);

        for (i = 0; i < len; i++)
            if (buf[i] != field[i])
                break;

        assert("all characters present", i == len);

        assert("last byte is newline", buf[len] == '\n');

        assert("returned pointer must point to next byte",
               p == (char *)buf + len + 1);
    }

    return 0;
}
