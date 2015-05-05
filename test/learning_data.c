#include <string.h>

#include "test.h"


int test_learning_data(void)
{
    char *str;

    {
        enum mode m;
        struct regime r;
        mpz_t ex;
        struct fann *model;
        struct fann_train_data *train;
        FILE *fp;
        struct learning_data ld;
        struct learning_data *ld2;
        bool failed;

        // mode (for regime)
        m = SINGLESTEP;

        // regime
        r.mode = m;
        r.address = 11225;
        r.period = 215609;
        r.instructions = 102023;
        r.dim = 12;

        // excitations (read from disk)
        fp = fopen("files/rsa.excite", "r");

        if (fp == NULL) {
            test_error("error opening rsa.excite");
            return 1;
        }

        mpz_init(ex);
        mpz_inp_raw(ex, fp);

        fclose(fp);

        // model (read from disk)
        model = fann_create_from_file("files/rsa.net");

        // training data (read from disk)
        train = fann_read_train_from_file("files/rsa.train");

        // learning data
        ld.regime = r;
        ld.excitations = &ex;
        ld.model = model;
        ld.tdata = train;

        str = serialize_learning_data(&ld);

        ld2 = deserialize_learning_data(str);

        failed = ld2 == NULL;

        assert("deserializing learning data doesn't fail",
               !failed);

        assert("regime mode equivalence",
               !failed && ld.regime.mode == ld2->regime.mode);

        assert("regime address equivalence",
               !failed && ld.regime.address ==
                          ld2->regime.address);

        assert("regime period equivalence",
               !failed && ld.regime.period ==
                          ld2->regime.period);

        assert("regime instructions equivalence",
               !failed && ld.regime.instructions ==
                          ld2->regime.instructions);

        assert("regime dimension equivalence",
               !failed && ld.regime.dim == ld2->regime.dim);

        assert("excitations equivalence",
               !failed && mpz_cmp(*ld.excitations,
                                  *ld2->excitations) == 0);

        assert("FANN model equivalence: network_type",
               !failed && memcmp(&ld.model->network_type,
                                 &ld2->model->network_type,
                                 sizeof(ld.model->network_type)) == 0);

        assert("FANN model equivalence: num_input",
               !failed && memcmp(&ld.model->num_input,
                                 &ld2->model->num_input,
                                 sizeof(ld.model->num_input)) == 0);

        assert("FANN model equivalence: num_output",
               !failed && memcmp(&ld.model->num_output,
                                 &ld2->model->num_output,
                                 sizeof(ld.model->num_output)) == 0);

        assert("FANN training data equivalence: num_data",
               !failed && memcmp(&ld.tdata->num_data,
                                 &ld2->tdata->num_data,
                                 sizeof(ld.tdata->num_data)) == 0);

        assert("FANN training data equivalence: num_input",
               !failed && memcmp(&ld.tdata->num_input,
                                 &ld2->tdata->num_input,
                                 sizeof(ld.tdata->num_input)) == 0);

        // TODO add more exhaustive tests here

        free(ld2);
        free(str);
    }

    return 0;
}
