#include "common.h"

#include "history.h"

#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "argp.h"

enum Eoption_group
{
    Eoption_group_collect,
    Eoption_group_options,
    Eoption_group_retrieve,
};

enum Eoption_key
{
    // Collect
    Eoption_key_collect_live = 'l',
    Eoption_key_collect_archive = 'a',

    // Options
    Eoption_key_options_save_location = 's',

    // Retrieve
    Eoption_key_retrieve_id3  = 'm',
    Eoption_key_retrieve_id64 = 'b',
    Eoption_key_retrieve_name = 'r',
};

static error_t save_location_parser(int key, char *arg, struct argp_state *state)
{
    if (key == Eoption_key_options_save_location && !history_initialized)
    {
        char *end;
        long result = strtol(arg, &end, 10);

        if (end == arg || end[0] != '\0' || errno == ERANGE)
        {
            argp_error(state, "Bad value \"%s\" passed via -s or --save-index.\n" , arg);
        }

        history_init(result);
    }

    return 0;
}

static error_t main_argp_parser(int key, char *arg, struct argp_state *state)
{
    switch (key)
    {
    case Eoption_key_collect_live:
        break;

    case Eoption_key_collect_archive:
        break;

    case Eoption_key_retrieve_id3:
        break;

    case Eoption_key_retrieve_id64:
        break;

    case Eoption_key_retrieve_name:
        break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    // TODO: Look into argp_child for potential simplification
    struct argp_option argp_options[] =
    {
        {
            .name = "save-index",
            .key = Eoption_key_options_save_location,
            .doc = "If not provided, use default location ~/.local/share/tf2pw00.sav. Else, put [0, 99] to choose number before \".sav\".",
            .arg = "SAVE_LOCATION",
            .group = Eoption_group_options
        },
        {
            .name = "collect-data-live",
            .key = Eoption_key_collect_live,
            .doc = "Collects data from TF2 during gameplay.",
            .flags = OPTION_ARG_OPTIONAL,
            .group = Eoption_group_collect
        },
        {
            .name = "collect-data-old",
            .key = Eoption_key_collect_archive,
            .doc = "Collects data from a previous log file post-gameplay.",
            .arg = "FILE",
            .group = Eoption_group_collect
        },
        {
            .name = "retrieve-data-id3",
            .key = Eoption_key_retrieve_id3,
            .doc = "Retrieve data of account with provided STEAMID3.",
            .arg = "STEAMID3",
            .group = Eoption_group_retrieve
        },
        {
            .name = "retrieve-data-id64",
            .key = Eoption_key_retrieve_id64,
            .doc = "Retrieve data of account with provided STEAMID64.",
            .arg = "STEAMID64",
            .group = Eoption_group_retrieve
        },
        {
            .name = "retrieve-data-name",
            .key = Eoption_key_retrieve_name,
            .doc = "Retrieve data of account with provided Steam name.",
            .arg = "STEAM_NAME",
            .group = Eoption_group_retrieve
        },
        // Terminating option
        {
            .name = NULL,
            .key = 0
        }
    };

    struct argp argp =
    {
        .options = argp_options,
        .parser = save_location_parser,
        .doc = "A small program to collect and display data on the players you play with in TF2."
    };

    argp_parse(&argp, argc, argv, 0, NULL, NULL);

    // No index was passed
    if (!history_initialized)
    {
        history_init(0);
    }

    argp.parser = main_argp_parser;
    argp_parse(&argp, argc, argv, 0, NULL, NULL);

    // TODO: After parsing, before freeing, some things may need doing here depending on the parsing capabilities of argp and circumstance

    history_free();
}
