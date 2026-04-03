#include "common.h"
#include "history.h"
#include "collection.h"
#include "interactive.h"
#include "steamid_manip.h"

#include "cider.h"

#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"

/*
    @brief For parsing purposes

        @param invocation First string in argv
        @param arg Proceeding argument
*/
typedef void (*operation_callback)(const char *invocation, char *arg);

/*
    @brief Contains information about option
*/
struct arg_option
{
    const char *name;
    const char *doc;
    const char *arg; // Set to NULL if no arg required
    const char *opt_long;
    const char  opt_short;
    const operation_callback operation;
};

// NEWARGS_TODO
static void operation_print_help(const char *invocation, char *arg);
static void operation_set_save_location(const char *invocation, char *arg);
static void operation_interactive(const char *invocation, char *arg);
static void operation_collect_archived(const char *invocation, char *arg);
static void operation_type(const char *invocation, char *arg);
static void operation_retrieve_records(const char *invocation, char *arg);

// NEWARGS_TODO
static struct arg_option arg_options[] =
{
    {
        .name = "--(h)elp",
        .doc = "Display this message. Supersedes all other arguments.",
        .arg = NULL,
        .opt_long = "help",
        .opt_short = 'h',
        .operation = operation_print_help
    },
    {
        .name = "--(s)et-save-location",
        // TODO: ~/.local... is not platform agnostic. Get user's data path from cider and add tf2pw.sav something
        .doc = "If provided, save and load to that history file, else use default location \"~/.local/share/tf2pw.sav\". Supersedes all below arguments.",
        .arg = "[FULLNAME]",
        .opt_long = "set-save-location",
        .opt_short = 's',
        .operation = operation_set_save_location
    },
    {
        .name = "--(i)nteractive",
        .doc = "Enter interactive mode where collect-live lives, the heart of TF2PW. Supersedes all below arguments.",
        .opt_long = "interactive",
        .opt_short = 'i',
        .operation = operation_interactive
    },
    {
        .name = "--collect-(a)rchived",
        .doc = "Collects data from a previous log file post-gameplay.",
        .arg = "[FULLNAME]",
        .opt_long = "collect-archived",
        .opt_short = 'a',
        .operation = operation_collect_archived
    },
    {
        .name = "--(t)ype",
        .doc = "Explicitly specify type of STEAMID to use with -r (retrieve), else is guessed.",
        .arg = "3|E|6|N",
        .opt_long = "type",
        .opt_short = 't',
        .operation = operation_type
    },
    {
        .name = "--(r)etrieve-records",
        .doc = "Retrieve data of account with provided Steam identifier. Optionally specify type with --type beforehand.",
        .arg = "[STEAMID3|STEAMID3E|STEAMID64|NAME]",
        .opt_long = "retrieve-records",
        .opt_short = 'r',
        .operation = operation_retrieve_records
    }
};

#define HELP_OUTP LTAB "%s %s\n" LTAB LTAB "%s\n"

// Will be VAL if VAL != 0/NULL/etc, else will be NVAL
#define NULLABLE(VAL, NVAL) ((VAL) ? (VAL) : (NVAL))

#define HELP_INFO(INDEX) arg_options[INDEX].name, NULLABLE(arg_options[INDEX].arg, ""), arg_options[INDEX].doc

static void operation_print_help(const char *invocation, char *unused)
{
    printf
    (
        "Warning: Only first occurrence of each option is considered.\n"
        "Usage: %s [OPTION]...\n"

        // NEWARGS_TODO
        HELP_OUTP // 0
        HELP_OUTP // 1
        HELP_OUTP // 2
        HELP_OUTP // 3
        HELP_OUTP // 4
        HELP_OUTP // 5
        ,
        invocation,

        // NEWARGS_TODO
        HELP_INFO(0),
        HELP_INFO(1),
        HELP_INFO(2),
        HELP_INFO(3),
        HELP_INFO(4),
        HELP_INFO(5)
    );

    exit(EXIT_SUCCESS);
}

void operation_set_save_location(const char *_, char *arg)
{
    history_init(arg);
}

void operation_interactive(const char *_, char *__)
{
    interactive_enter();

    history_free();

    exit(EXIT_SUCCESS);
}

void operation_collect_archived(const char *_, char *arg)
{
    collection_read_archived(arg);
}

// The type passed via --type. Will stay unknown if none passed
static enum Esteamid_type passed_type = Esteamid_type_unknown;

void operation_type(const char *_, char *arg)
{
    switch (*arg)
    {
        break; case '3':
        {
            passed_type = Esteamid_type_sid3;
        }
        break; case 'E': case 'e':
        {
            passed_type = Esteamid_type_sid3e;
        }
        break; case 'N': case 'n':
        {
            passed_type = Esteamid_type_name;
        }
        break; case '6':
        {
            passed_type = Esteamid_type_sid64;
        }
    }
}

void operation_retrieve_records(const char *_, char *arg)
{
    const uint32_t sid3e = sidm_parse_sid3e(arg, passed_type);

    switch (sid3e)
    {
        break; default:
        {
            history_print_record(sid3e);
        }
        break; case SIDM_ERR_NAME:
        {
            history_print_records(arg);
        }
        break; case SIDM_ERR_MISC:
        {
            fprintf(stderr, ANSI_RED "Not correct/specified form of ID: \"%s\".\n" ANSI_RESET, arg);
            TF2_PLAYED_WITH_DEBUG_ABEX();
        }
        break; case SIDM_ERR_RNGE:
        {
            fprintf(stderr, ANSI_RED "Passed ID \"%s\" too large.\n" ANSI_RESET, arg);
            TF2_PLAYED_WITH_DEBUG_ABEX();
        }
    }
}

static void parse_option(const int argc, char **argv, const int option_i)
{
    int i = 1;
    do
    {
        // Long
        if (!strncmp(argv[i], "--", 2) && !strcmp(argv[i] + 2, arg_options[option_i].opt_long))
        {
            goto OPERATE;
        }
        // Short
        else if (argv[i][0] == '-' && argv[i][1] == arg_options[option_i].opt_short)
        {
            goto OPERATE;
        }
    }
    while (++i < argc);
    return;

    OPERATE:;
    if (arg_options[option_i].arg)
    {
        if (i + 1 >= argc || !strncmp(argv[i + 1], "--", 2) || argv[i + 1][0] == '-')
        {
            fprintf(stderr, "%s requires an argument.\n", arg_options[option_i].name);
            TF2_PLAYED_WITH_DEBUG_ABEX();
        }
        else
        {
            arg_options[option_i].operation(argv[0], argv[i + 1]);
        }
    }
    else
    {
        arg_options[option_i].operation(argv[0], NULL);
    }
}

int main(const int argc, char **argv)
{
    if (argc == 1)
    {
        fprintf(stderr, "Need argument(s). Try \"%s --help\" for help.\n", argv[0]);
        return 1;
    }

    parse_option(argc, argv, 0); // Help

    parse_option(argc, argv, 1); // Set save location

    if (!history_initialized)
    {
        history_init(NULL);
    }

    history_load();

    parse_option(argc, argv, 2); // Interactive (Will exit in here if found)

    parse_option(argc, argv, 3); // Collect archived

    parse_option(argc, argv, 4); // Type

    parse_option(argc, argv, 5); // Retrieve records

    // NEWARGS_TODO

    history_save();

    history_free();
}
