#include "history.h"
#include "collection.h"
#include "interactive.h"

#include "cider.h"

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
    Eoption_key_options_interactive = 'i',

    // Retrieve
    Eoption_key_retrieve_id3  = 'r',
    Eoption_key_retrieve_id64 = 'b',
    Eoption_key_retrieve_name = 'n',
};

#define NO_ERR ((error_t) 0)

static error_t parser_save_location(int key, char *arg, struct argp_state *state)
{
    if (key == Eoption_key_options_save_location && !history_initialized)
    {
        history_init(cider_canonicalize_file(arg));
    }

    return NO_ERR;
}

static error_t parser_interactive(int key, char *arg, struct argp_state *state)
{
    if (key == Eoption_key_options_interactive)
    {
        interactive_enter();
        history_free();

        exit(EXIT_SUCCESS);
    }

    return NO_ERR;
}

static error_t parser_main(int key, char *arg, struct argp_state *state)
{
    switch (key)
    {
    case Eoption_key_collect_live:
        /*
            * Start thread executing collection_read_live(...)
            * Accept user input as described in /README.md#functionality
        */
        break;

    case Eoption_key_collect_archive:
        char *collection_fullname = cider_canonicalize_file(arg);
        collection_read_archived(collection_fullname);
        free(collection_fullname);
        break;

    case Eoption_key_retrieve_id3:
        /*
            * Convert to SID3E if applicable
            * Loop through records
            * Once record of specified SID3E reached, return info on it
                * Name
                * TODO...
        */
        break;

    case Eoption_key_retrieve_id64:
        /*
            * Convert to SID3E
            * Use retrieve_id3
        */
        break;

    case Eoption_key_retrieve_name:
        /*
            * Loop through all date records
            * For each player record with date record with specified name, take SID3E and go to next record
            * After checking every date record, retrieve SID3 records from list of SID3E's, do with it whatever retrieve_id3 does
        */
        break;
    }

    return NO_ERR;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Need argument(s). Try \"%s --help\" for help.\n", argv[0]);
        return 1;
    }

    // TODO: Look into argp_child for potential simplification
    struct argp_option argp_options[] =
    {
        {
            .name = "save-index",
            .key = Eoption_key_options_save_location,
            // TODO: ~/.local... is not platform agnostic. Get user's data path from cider and add tf2pw.sav something
            .doc = "If file provided, save and load to that history file, else use default location \"~/.local/share/tf2pw.sav\".",
            .arg = "SAVE_LOCATION",
            .group = Eoption_group_options
        },
        {
            .name = "interactive",
            .key = Eoption_key_options_interactive,
            .doc = "Enter interactive mode. Supersedes all other arguments.",
            .flags = OPTION_ARG_OPTIONAL,
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
            .arg = "ARCHIVE_FILE",
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
        .doc = "A small program to collect and display data on the players you play with in TF2."
    };

    argp.parser = parser_save_location;
    argp_parse(&argp, argc, argv, 0, NULL, NULL);

    // No history fullname was passed in previous parse
    if (!history_initialized)
    {
        history_init(NULL);
    }

    argp.parser = parser_interactive;
    argp_parse(&argp, argc, argv, 0, NULL, NULL); // If interactive passed, will exit program here

    history_load();

    argp.parser = parser_main;
    argp_parse(&argp, argc, argv, 0, NULL, NULL);

    history_save();

    history_free();
}
