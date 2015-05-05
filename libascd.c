/*
 * libascd.c
 * libascd, the ASC daemon library
 */

#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "libascd.h"

#define PROTOCOL_VERSION            1
#define SERIALIZATION_BUFFER_SIZE   1 << 19
#define UNIX_PATH_MAX               108
#define CLIENT_BUFFER_SIZE          1 << 19
#define TEMP_FILE_PATH              ".tmp"
#define NUM_FANN_NET_LINES          36


/*
 * Serializing and deserializing FANN structures.
 */

char *fann_to_string(struct fann *model)
{
    char *temp_file_path;
    char *str;
    FILE *fp;
    int len;

    temp_file_path = tmpnam(NULL);

    fann_save(model, temp_file_path);

    fp = fopen(temp_file_path, "r");

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    str = malloc(len + 1);
    fread(str, len, 1, fp);
    fclose(fp);

    str[len] = '\0';

    return str;
}

struct fann *string_to_fann(char *str)
{
    FILE *fp;
    int num_lines;
    struct fann *model;

    // TODO use tmpnam
    // TODO use memfd
    fp = fopen(TEMP_FILE_PATH, "w");

    num_lines = 0;
    while (num_lines < NUM_FANN_NET_LINES) {
        if (*str == '\n') {
            num_lines++;
            if (num_lines >= NUM_FANN_NET_LINES)
                break;
        }

        fwrite(str++, 1, 1, fp);
    }
    fclose(fp);

    model = fann_create_from_file(TEMP_FILE_PATH);

    return model;
}

char *fann_train_data_to_string(struct fann_train_data *tdata)
{
    char *temp_file_path;
    char *str;
    FILE *fp;
    int len;

    temp_file_path = tmpnam(NULL);

    fann_save_train(tdata, temp_file_path);

    fp = fopen(temp_file_path, "r");

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    str = malloc(len + 1);
    fread(str, len, 1, fp);
    fclose(fp);

    str[len] = '\0';

    return str;
}

struct fann_train_data *string_to_fann_train_data(char *str)
{
    FILE *fp;
    struct fann_train_data *tdata;

    // TODO use tmpnam
    // TODO use memfd
    fp = fopen(TEMP_FILE_PATH, "w");

    while (*str != '\0')
        fwrite(str++, 1, 1, fp);
    fclose(fp);

    tdata = fann_read_train_from_file(TEMP_FILE_PATH);
    return tdata;
}


/*
 * Serialization/deserialization functions.
 */

inline char *end_of_string(char *s)
{
    return s + strlen(s);
}

inline char *eol(char *str)
{
    while (*str != '\n' && *str != '\0')
        str++;

    return str;
}

/*
 * Given a pointer to any memory region "dest", this function uses
 * strcpy() to copy the "src" string to "dest" (strcpy also copies
 * the NUL byte from "src") and returns a pointer to the last byte
 * copied in "dest" (the NUL byte).
 */
inline char *stradd(char *dest, char *src)
{
    strcpy(dest, src);
    dest += strlen(src);
    return dest;
}

inline int line_length(char *str)
{
    int c;

    c = 0;
    while (str[c] != '\n')
        c++;

    return c;
}

inline void error(char *str)
{
    fprintf(stderr, "error: %s\n", str);
}

char *write_field(char *buf, struct field f)
{
    buf = stradd(buf, f.key);

    *buf = '=';
    buf++;

    buf = stradd(buf, f.val);

    *buf = '\n';
    buf++;

    return buf;
}

struct field read_field(char *buf)
{
    struct field f;
    int n;
    char *end;

    n = 0;
    end = eol(buf);         // pointer to NUL byte

    while (buf[n] != '=') n++;

    f.key = malloc(n + 1);
    memcpy(f.key, buf, n + 1);
    f.key[n] = '\0';

    buf += n + 1;           // + 1 to go past the '='
    n = 0;

    while (buf[n] != '\n' && buf[n] != '\0') n++;

    f.val = malloc(n + 1);
    memcpy(f.val, buf, n + 1);
    f.val[n] = '\0';

  success:
    return f;

  failure:
    f.key = NULL;
    f.val = NULL;
    return f;
}

char *serialize_mode(enum mode *m)
{
    struct field f;
    char *start, *end;

    start = malloc(SERIALIZATION_BUFFER_SIZE);

    f.key = "mode";

    switch (*m) {
    case BASELINE:
        f.val = "baseline";
        break;

    case SINGLESTEP:
        f.val = "singlestep";
        break;

    case BLOCKSTEP:
        f.val = "blockstep";
        break;

    case BREAKPOINT:
        f.val = "breakpoint";
        break;

    case INSTRUCTIONS:
        f.val = "instructions";
        break;
    }

    end = write_field(start, f);
    end[-1] = '\0';

    return realloc(start, end - start);
}

enum mode *deserialize_mode(char *str)
{
    struct field f;
    enum mode *p;

    p = malloc(sizeof(enum mode));

    f = read_field(str);

    if (strcmp(f.key, "mode") != 0) {
        error("invalid key for mode");
        goto failure;
    }

    if (strcmp(f.val, "baseline") == 0) {
        *p = BASELINE;
    } else if (strcmp(f.val, "singlestep") == 0) {
        *p = SINGLESTEP;
    } else if (strcmp(f.val, "blockstep") == 0) {
        *p = BLOCKSTEP;
    } else if (strcmp(f.val, "breakpoint") == 0) {
        *p = BREAKPOINT;
    } else if (strcmp(f.val, "instructions") == 0) {
        *p = INSTRUCTIONS;
    } else {
        error("invalid value for mode");
        goto failure;
    }

  success:
    return p;

  failure:
    free(p);
    return NULL;
}

char *serialize_regime(struct regime *r)
{
    struct field f;
    char *start, *end;
    char *temp;
    int len;

    start = malloc(SERIALIZATION_BUFFER_SIZE);

    temp = serialize_mode(&r->mode);
    len = strlen(temp);

    strcpy(start, temp);

    start[len] = '\n';

    free(temp);

    end = start + len + 1;

    f.key = "address";
    asprintf(&temp, "%lu", r->address);
    f.val = temp;
    end = write_field(end, f);
    free(temp);

    f.key = "period";
    asprintf(&temp, "%lu", r->period);
    f.val = temp;
    end = write_field(end, f);
    free(temp);

    f.key = "instructions";
    asprintf(&temp, "%lu", r->instructions);
    f.val = temp;
    end = write_field(end, f);
    free(temp);

    f.key = "dim";
    asprintf(&temp, "%lu", r->dim);
    f.val = temp;
    end = write_field(end, f);
    free(temp);

    end[-1] = '\0';

    return realloc(start, end - start);
}

struct regime *deserialize_regime(char *str, char **end)
{
    struct field f;
    struct regime *r;
    enum mode *temp;

    r = malloc(sizeof(struct regime));

    // TODO check for NULL from all subroutine calls
    temp = deserialize_mode(str);
    r->mode = *temp;
    free(temp);

    // move past the mode
    str = eol(str) + 1;

    f = read_field(str);
    r->address = atol(f.val);
    str = eol(str) + 1;

    f = read_field(str);
    r->period = atol(f.val);
    str = eol(str) + 1;

    f = read_field(str);
    r->instructions = atol(f.val);
    str = eol(str) + 1;

    f = read_field(str);
    r->dim = atol(f.val);
    str = eol(str) + 1;

    if (end != NULL)
        *end = str;

    return r;
}

char *serialize_program(struct program *prog)
{
    struct field f;
    char *start, *end;
    char *temp;
    int len;

    start = malloc(SERIALIZATION_BUFFER_SIZE);

    // write program name
    f.key = "name";
    f.val = prog->name;

    end = write_field(start, f);

    // write any_regime field
    f.key = "any_regime";

    if (prog->any_regime)
        f.val = "true";
    else
        f.val = "false";

    end = write_field(end, f);

    // write regime
    temp = serialize_regime(&prog->regime);
    len = strlen(temp);

    strcpy(end, temp);

    free(temp);

    end = end + len + 1;

    end[-1] = '\0';

    return realloc(start, end - start);
}

struct program *deserialize_program(char *str)
{
    struct field f;
    struct program *prog;
    struct regime *temp;

    prog = malloc(sizeof(struct program));

    f = read_field(str);

    if (strcmp(f.key, "name") != 0) {
        error("invalid program name key");
        goto failure;
    }

    strcpy(prog->name, f.val);
    free(f.key);
    free(f.val);

    str = eol(str) + 1;

    // read any_regime field
    f = read_field(str);

    if (strcmp(f.key, "any_regime") != 0) {
        error("invalid any_regime key");
        goto failure;
    }

    if (strcmp(f.val, "true") == 0) {
        prog->any_regime = true;
    } else if (strcmp(f.val, "false") == 0) {
        prog->any_regime = false;
    } else {
        error("invalid any_regime value");
        free(f.key);
        free(f.val);
        goto failure;
    }

    if (prog->any_regime)
        // ignore the rest of the fields
        goto success;

    // read regime fields
    str = eol(str) + 1;

    temp = deserialize_regime(str, NULL);

    memcpy(&prog->regime, temp, sizeof(struct regime));

    free(temp);

  success:
    return prog;

  failure:
    free(prog);
    return NULL;
}

char *serialize_learning_data(struct learning_data *d)
{
    struct field f;
    char *start, *end;
    char *temp;

    start = malloc(SERIALIZATION_BUFFER_SIZE);
    end = start;

    temp = serialize_regime(&d->regime);
    end = stradd(end, temp);
    free(temp);

    *end = '\n';
    end++;

    temp = mpz_get_str(NULL, 16, *d->excitations);
    f.key = "excitations";
    f.val = temp;
    end = write_field(end, f);
    free(temp);

    /*
     * Note: not writing a newline here beacuse write_field()
     * does it for us, and returns a pointer to where we should write
     * the next field.
     */

    temp = fann_to_string(d->model);
    end = stradd(end, temp);
    free(temp);

    *end = '\n';
    end++;

    temp = fann_train_data_to_string(d->tdata);
    end = stradd(end, temp);
    free(temp);

    *end = '\0';

    return start;
}

struct learning_data *deserialize_learning_data(char *str)
{
    struct field f;
    mpz_t ex, *exp;
    FILE *fp;
    int n;
    struct learning_data *data;
    struct regime *temp_regime;

    data = malloc(sizeof(struct learning_data));

    // read regime
    temp_regime = deserialize_regime(str, &str);
    memcpy(&data->regime, temp_regime, sizeof(struct regime));
    free(temp_regime);

    // read excitations
    f = read_field(str);
    mpz_init(ex);
    mpz_set_str(ex, f.val, 16);

    exp = malloc(sizeof(mpz_t));
    memcpy(exp, &ex, sizeof(ex));

    data->excitations = exp;

    str = eol(str) + 1;

    data->model = string_to_fann(str);

    for (n = 0; n < NUM_FANN_NET_LINES; n++)
        str = eol(str) + 1;

    data->tdata = string_to_fann_train_data(str);

    return data;
}


/*
 * Connection maintenance functions.
 */

struct ascd_conn ascd_connect(char *sock_path)
{
    struct ascd_conn conn;
    int fd;
    struct sockaddr_un client;
    struct sockaddr_un daemon;

    // initialize connection struct
    conn.status = FAILED;
    conn.mode = NORMAL;
    conn.daemon_sock_path = sock_path;
    conn.fd = -1;

    if ((conn.client_sock_path = tmpnam(NULL)) == NULL) {
        goto end;
    }

    // set up client address
    client.sun_family = AF_UNIX;
    strncpy(client.sun_path, conn.client_sock_path, UNIX_PATH_MAX);

    // set up daemon address
    daemon.sun_family = AF_UNIX;
    strncpy(daemon.sun_path, sock_path, UNIX_PATH_MAX);

    if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        goto end;
    }

    if (bind(fd, (struct sockaddr *)&client, sizeof(client)) < 0) {
        goto end;
    }

    if (connect(fd, (struct sockaddr *)&daemon, sizeof(daemon)) < 0) {
        goto end;
    }

    conn.status = CONNECTED;
    conn.fd = fd;

  end:
    return conn;
}

int ascd_disconnect(struct ascd_conn *conn)
{
    if (conn->status != CONNECTED)
        return -1;

    close(conn->fd);
    conn->status = DISCONNECTED;
    conn->fd = -1;

    unlink(conn->client_sock_path);

    return 0;
}

int ascd_quit(struct ascd_conn *conn)
{
    if (conn->status != CONNECTED)
        return -1;

    if (send(conn->fd, ASCD_MSG_QUIT, strlen(ASCD_MSG_QUIT) + 1, 0) < 0) {
        conn->status = FAILED;
        return -1;
    }

    return ascd_disconnect(conn);
}


/*
 * Useful functions.
 */

struct learning_data *ascd_query(struct ascd_conn *conn,
                                 struct program *prog)
{
    char *buf;
    char *p;
    char *serialized;
    struct learning_data *data;

    if (conn->status != CONNECTED)
        return NULL;

    buf = malloc(CLIENT_BUFFER_SIZE);
    p = buf;

    strcpy(p, ASCD_MSG_LOOKUP);
    p += strlen(ASCD_MSG_LOOKUP);

    *p = '\n';
    p += 1;

    serialized = serialize_program(prog);

    strcpy(p, serialized);
    p += strlen(serialized) + 1;

    if (send(conn->fd, buf, p - buf, 0) < 0) {
        conn->status = FAILED;
        return NULL;
    }

    if (recv(conn->fd, buf, CLIENT_BUFFER_SIZE, 0) < 0) {
        conn->status = FAILED;
        return NULL;
    }

    free(serialized);

    // TODO this should go into a message interface
    if (strcmp(buf, "NONE") == 0)
        return NULL;
    else
        return deserialize_learning_data(buf);
}
