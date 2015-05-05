/*
 * libascd.h
 * libascd, the ASC daemon library
 *
 * Applications that need to communicate with an ASC daemon
 * (most notably asc, "the" ASC client) use these functions.
 * The functions here create an abstraction over the control socket
 * that is used by ascd. In summary, they allow any ASC client to:
 *     - connect to an ASC daemon, either by specifying the location
 *       of a control socket or the PID of a currently running daemon
 *     - start a new ASC daemon for use exclusively by the client
 *     - query an ASC daemon for learning data about a particular
 *       program
 *     - store new learning data obtained by the client by running
 *       the program in the ASC daemon
 */

#ifndef _LIBASCD_H_
#define _LIBASCD_H_
#endif

#include <stdbool.h>
#include <fann.h>
#include <gmp.h>

#define ASCD_MSG_QUIT   "QUIT"
#define ASCD_MSG_LOOKUP "LOOKUP"
#define ASCD_MSG_STORE  "STORE"

#define MAX_STRING_LEN      1 << 6
#define MAX_MESSAGE_LEN     1 << 12

char *fann_to_string(struct fann *model);
struct fann *string_to_fann(char *str);
char *fann_train_data_to_string(struct fann_train_data *tdata);
struct fann_train_data *string_to_fann_train_data(char *str);

inline char *eol(char *str);

/*
 * Common structures specifying programs, learning data, and
 * sampling regimes.
 */

struct field {
    char *key;
    char *val;
};

char *write_field(char *buf, struct field f);
struct field read_field(char *buf);

// transition rule modes
enum mode {
    BASELINE     = 0,
    SINGLESTEP   = 1 << 0,
    BLOCKSTEP    = 1 << 1,
    BREAKPOINT   = 1 << 2,
    INSTRUCTIONS = 1 << 3,
};

char *serialize_mode(enum mode *m);
enum mode *deserialize_mode(char *str);

struct regime {
    enum mode mode;
    long address;
    long period;
    long instructions;
    long dim;
};

char *serialize_regime(struct regime *r);
struct regime *deserialize_regime(char *str, char **end);

struct program {
    char name[MAX_STRING_LEN];   // name of the binary

    /*
     * If any_regime is set (e.g., when using this struct in a
     * call to ascd_query()), the sampling regime field below is
     * ignored and the ASC daemon will return learning data for
     * any sampling regime.
     */
    bool any_regime;
    struct regime regime;
};

char *serialize_program(struct program *p);
struct program *deserialize_program(char *str);

struct learning_data {
    struct regime regime;
    mpz_t *excitations;
    struct fann *model;
    struct fann_train_data *tdata;
};

char *serialize_learning_data(struct learning_data *d);
struct learning_data *deserialize_learning_data(char *str);

/*
 * Connection structures.
 */

enum ascd_conn_status {
    FAILED, CONNECTED, DISCONNECTED
};

enum ascd_conn_mode {
    NORMAL, EXCLUSIVE
};

/*
 * Represents a single connection to an ASC daemon. Can be reused.
 */
struct ascd_conn {
    enum ascd_conn_status status;
    enum ascd_conn_mode mode;
    char *client_sock_path;
    char *daemon_sock_path;
    int fd;
};

/*
 * Attempts to create a new connection to an ASC daemon whose control
 * socket path is given. If the connection could not be established,
 * the returned struct's status field is set to FAILED. Otherwise,
 * it is set to CONNECTED.
 */
struct ascd_conn ascd_connect(char *sock_path);
struct ascd_conn ascd_connect_pid(long pid);

// int ascd_reconnect(struct ascd_conn *conn);

int ascd_disconnect(struct ascd_conn *conn);

/*
 * Starts a new ASC daemon for use exclusively by the caller. The
 * daemon shuts down when this connection is closed. The connection mode
 * in the returned ascd_conn struct is set to EXCLUSIVE.
 */
// struct ascd_conn ascd_create(void);

/*
 * Instructs an ASC daemon to quit.
 */
int ascd_quit(struct ascd_conn *conn);

/*
 * Queries the daemon over the given connection for learning data
 * about a program. If the daemon has learning data, it is copied
 * into a buffer allocated by this function, a pointer to which is
 * returned. The caller should free() this pointer when it is
 * done with the data. If the daemon does not have learning data,
 * this function returns NULL.
 */
struct learning_data *ascd_query(struct ascd_conn *conn,
                                 struct program *p);

/*
 * Sends learning data for a program to a daemon. This function
 * returns -1 if storing the data fails, and may update the ascd_conn
 * struct (e.g., if storing fails because the connection is closed).
 */
int ascd_store(struct ascd_conn *conn, struct program *p,
               struct learning_data *data);
