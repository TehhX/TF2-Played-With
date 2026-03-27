#include "history.h"
#include "collection.h"
#include "interactive.h"
#include "steamid_manip.h"

#include "cider.h"

#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "argp.h"

enum Eoption_key
{
    Eoption_key_collect_archive = 'a',
    Eoption_key_save_location   = 's',
    Eoption_key_interactive     = 'i',
    Eoption_key_id_type         = 't',
    Eoption_key_retrieve        = 'r',
};

#define NO_ERR ((error_t) 0)

// The type passed via --type. Will stay unknown if none passed
static enum Esteamid_type passed_type = Esteamid_type_unknown;

static error_t parser_type(int key, char *arg, struct argp_state *state)
{
    if (key == Eoption_key_id_type)
    {
        switch (*arg)
        {
            break; case '3': passed_type = Esteamid_type_sid3;

            break; case 'E':
                   case 'e': passed_type = Esteamid_type_sid3e;

            break; case 'N':
                   case 'n': passed_type = Esteamid_type_name;

            break; case '6': passed_type = Esteamid_type_sid64;
        }
    }

    return NO_ERR;
}

static error_t parser_save_location(int key, char *arg, struct argp_state *state)
{
    if (key == Eoption_key_save_location && !history_initialized)
    {
        history_init(cider_canonicalize_file(arg));
    }

    return NO_ERR;
}

static error_t parser_interactive(int key, char *arg, struct argp_state *state)
{
    if (key == Eoption_key_interactive)
    {
        history_load();

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
        break; case Eoption_key_collect_archive:
        {
            char *const collection_fullname = cider_canonicalize_file(arg);

            collection_read_archived(collection_fullname);

            free(collection_fullname);
        }
        break; case Eoption_key_retrieve:
        {
            // Should be a standard ID, just print associated record
            if (!(passed_type & Esteamid_type_name))
            {
                const uint32_t sid3e = sidm_parse_sid3e(arg, passed_type);

                if (sid3e == SIDM_ERR_RNGE)
                {
                    argp_error(state, "MAJOR: Passed ID \"%s\" too large.\n", arg);
                }
                else if (sid3e <= SIDM_ERR_NONE_MAX)
                {
                    history_print_record(sid3e);

                    return NO_ERR;
                }

                // Is a name, escape if statement and parse
            }

            // Is confirmed or suspected to be a name, print all player records containing even a single occurrence of it
            history_print_records(arg);
        }
    }

    return NO_ERR;
}

// Standard argp parse call via callback PARSER
#define std_argp_parse(PARSER) argp.parser = PARSER; argp_parse(&argp, argc, argv, 0, NULL, NULL)

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
            .name = "interactive",
            .key = Eoption_key_interactive,
            .doc = "Enter interactive mode. Supersedes all other arguments. Where collect-live lives, the heart of TF2PW.",
            .flags = OPTION_ARG_OPTIONAL
        },
        {
            .name = "save-index",
            .key = Eoption_key_save_location,
            // TODO: ~/.local... is not platform agnostic. Get user's data path from cider and add tf2pw.sav something
            .doc = "If file provided, save and load to that history file, else use default location \"~/.local/share/tf2pw.sav\".",
            .arg = "SAVE_LOCATION"
        },
        {
            .name = "type",
            .key = Eoption_key_id_type,
            .doc = "Explicitly specify type of STEAMID to use with -r (retrieve). Can be one of: 3 (STEAMID3), E (STEAMID3 Excerpt), 6 (STEAMID64), or N (Name). Case insensitive.",
            .arg = "[3|E|6|N]"
        },
        {
            .name = "collect-data-old",
            .key = Eoption_key_collect_archive,
            .doc = "Collects data from a previous log file post-gameplay.",
            .arg = "ARCHIVE_FILE"
        },
        {
            .name = "retrieve-records",
            .key = Eoption_key_retrieve,
            .doc = "Retrieve data of account with provided Steam identifier (STEAMID3, STEAMID3 excerpt, Name). Optionally specify type with -t (type)",
            .arg = "STEAMID"
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

    std_argp_parse(parser_save_location);

    // No history fullname was passed in previous parse
    if (!history_initialized)
    {
        history_init(NULL);
    }

    // Reminder: If interactive passed, program will exit here
    std_argp_parse(parser_interactive);

    history_load();

    // Get explicit STEAMID type to retrieve
    std_argp_parse(parser_type);

    // Parse all remaining options
    std_argp_parse(parser_main);

    history_save();

    history_free();
}
