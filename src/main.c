#include "stdio.h"
#include "argp.h"
#include "stdint.h"
#include "inttypes.h"

enum Eretrieve_type
{
    Eretrieve_type_id3 = 300,
    Eretrieve_type_id64,
    Eretrieve_type_name
};

enum Eutils_action
{
    Eutils_action_id3_to_id64 = 400
};

enum Eoption_group
{
    Eoption_group_collect,
    Eoption_group_retrieve,
    Eoption_group_options,
    Eoption_group_utils
};

static uint64_t steamid3_to_64(const char *steamid3)
{
    uint64_t return_value = 0;

    // TODO: Will segfault if steamid3 is not exactly perfect, or just wrong input. Trusting user input for now
    for (int i = 5; steamid3[i] != ']'; ++i)
    {
        return_value = return_value * 10 + steamid3[i] - '0';
    }

    return 76561197960265728 | return_value;
}

static error_t main_argp_parser(int key, char *arg, struct argp_state *state)
{
    switch (key)
    {
    case Eutils_action_id3_to_id64:
        printf("STEAMID3(%s) = STEAMID64(%" PRIu64 ")\n", arg, steamid3_to_64(arg));
        break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    struct argp_option argp_options[] =
    {
        {
            .name = "save-location",
            .key = 's',
            .doc = "The save file location to save/load to/from.",
            .arg = "SAVE LOCATION",
            .group = Eoption_group_options
        },

        {
            .name = "collect-data",
            .key = 'c',
            .doc = "Collects data from TF2 during gameplay.",
            .flags = OPTION_ARG_OPTIONAL,
            .group = Eoption_group_collect
        },

        {
            .name = "retrieve-data-id3",
            .key = Eretrieve_type_id3,
            .doc = "Retrieve data of account.",
            .arg = "STEAMID3",
            .group = Eoption_group_retrieve
        },

        {
            .name = "retrieve-data-id64",
            .key = Eretrieve_type_id64,
            .doc = "Retrieve data of account.",
            .arg = "STEAMID64",
            .group = Eoption_group_retrieve
        },

        {
            .name = "retrieve-data-name",
            .key = Eretrieve_type_name,
            .doc = "Retrieve data of account.",
            .arg = "STEAM NAME",
            .group = Eoption_group_retrieve
        },

        {
            .name = "convert-id3-id64",
            .key = Eutils_action_id3_to_id64,
            // TODO: When it no longer results in a segfault, remove warning
            .doc = "Convert a STEAMID3 to a STEAMID64. Mostly for testing purposes. WARNING: Any input errors will result in a segfault.",
            .arg = "STEAMID3",
            .group = Eoption_group_utils
        },

        {
            .name = NULL,
            .key = 0
        }
    };

    struct argp argp =
    {
        .options = argp_options,
        .parser = main_argp_parser,
        .doc = "A small program to collect data on the players you play with in TF2"
    };

    return argp_parse(&argp, argc, argv, 0, NULL, NULL);
}
