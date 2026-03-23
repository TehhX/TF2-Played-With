#include "collection.h"

#include "history.h"

#include "stdio.h"
#include "string.h"

// Size of the line buffer in bytes. Should be a couple bytes larger than the largest expected line length. Keep in mind that values via pow(2, (int) exp) eg. 512 make it seem up to 8 times more professional
#define LINE_BUFB 512

// Size of the username in bytes. Steam says max is 32, 64 is to be safe
#define USER_BUFB 64

void collection_read_live(const char *collection_fullname)
{
    // TODO
}

void collection_read_archived(const char *collection_fullname)
{
    FILE *const input_file_ptr = fopen(collection_fullname, "r");
    if (!input_file_ptr)
    {
        fprintf(stderr, "MAJOR: Failed to open \"%s\" for writing. Error: ", collection_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }

    char user_name[USER_BUFB] = { '\0' }, line_buf[LINE_BUFB];

    // REMINDER: End of line will be '\n', not '\0'
    for (unsigned long long line_index = 0; fgets(line_buf, LINE_BUFB, input_file_ptr); ++line_index)
    {
        // Check for connected message in the form `<NAME> connected`
        const char connected_suffix[9] = "connected";
        for (int i = 0; i < LINE_BUFB; ++i)
        {
            if (line_buf[i] == '\n')
            {
                // Line is not long enough to house connected statement, no point in checking (may also cause a program crash but hey)
                if (i <= sizeof(connected_suffix))
                {
                    goto CONNECTED_FINISH;
                }

                if (!memcmp(line_buf + i - sizeof(connected_suffix), connected_suffix, sizeof(connected_suffix)))
                {
                    const int name_end = i - sizeof(connected_suffix) - 1;

                    // Is first occurance of username
                    if (user_name[0] == '\0')
                    {
                        memcpy(user_name, line_buf, name_end);
                        user_name[name_end] = '\0';

                        TF2_PLAYED_WITH_DEBUG_LOGF("LOG: (LI=%llu) First occurance of username found: \"%s\"\n", line_index, user_name);
                    }
                    else if (!memcmp(line_buf, user_name, name_end))
                    {
                        TF2_PLAYED_WITH_DEBUG_LOGF("LOG: (LI=%llu) Another username occurance: \"%.*s\"\n", line_index, name_end, line_buf);
                    }
                    // Is not user's name
                    else
                    {
                        TF2_PLAYED_WITH_DEBUG_LOGF("LOG: (LI=%llu) Player connected: \"%.*s\". Beginning to scan status output for their SID3E.\n", line_index, name_end, line_buf);
                    }
                }

                goto CONNECTED_FINISH;
            }
        }
        CONNECTED_FINISH:

        // Check for status output
        // TODO: Doesn't check that line is long enough. Probably not an issue but keep testing for it and remove this comment when it is sure to cause no problems
        const char status_prefix[3] = "#  ";
        if (!memcmp(line_buf, status_prefix, sizeof(status_prefix)))
        {
            TF2_PLAYED_WITH_DEBUG_LOGF("LOG: (LI=%llu) Status report: %s", line_index, line_buf);

            // TODO
        }
    }

    TF2_PLAYED_WITH_DEBUG_LOGF("LOG: Username at end: \"%s\"\n", user_name);

    if (fclose(input_file_ptr))
    {
        fprintf(stderr, "MAJOR: Failed to close \"%s\". Error: ", collection_fullname);
        perror(NULL);
        TF2_PLAYED_WITH_DEBUG_ABEX();
    }
}
