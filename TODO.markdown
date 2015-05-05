*   The "mode" field might have multiple values set!
*   read_field() malloc()s memory that isn't always freed
    when (de)serializing
*   When some client uses `libascd` to connect to a daemon, the
    connection should fail if the protocol versions do not match
*   Bounds checking in serialization functions
*   Reduce data redundancy in structs
    -   The sampling regime is stored in both `program` and
        `learning_data`
*   Not all the functions in the API are implemented
*   All more checking to serialization/deserialization functions
*   Serializing the FANN model forces us to go to disk to
    save a temporary file; can we do it without disk I/O
    and without trying to parse it ourselves?
*   Saving the FANN model to disk may be memory-intensive;
    we should buffer the potentially large data and send it
    directly over the socket in pieces to reduce the
    client's memory overhead
