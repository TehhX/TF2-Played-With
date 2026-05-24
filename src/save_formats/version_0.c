#include "version_0.h"

#include "../common.h"
#include "main.h"
#include "../file_io.h"
#include "version_1.h"

#include "cider.h"

#include "string.h"

#ifdef TF2_PLAYED_WITH_DEBUG
    #include "inttypes.h"
#endif

bool save_format_0_load(struct save_format_0 *save_data, FILE *input_file_ptr)
{
    fread_one(save_data->user_sid3e);
    fread_one(save_data->default_record_messages);
    fread_one(save_data->player_records_len);
    fread_one(save_data->tf2_filepath_len);

    save_data->tf2_filepath = malloc(save_data->tf2_filepath_len + 2);
    fread_arr(save_data->tf2_filepath);
    save_data->tf2_filepath[save_data->tf2_filepath_len] = CIDER_PATH_DELIM_C;
    save_data->tf2_filepath[save_data->tf2_filepath_len + 1] = '\0';

    TF2_PLAYED_WITH_DEBUG_LOGF("Set tf2_filepath to \"%s\".\n", save_data->tf2_filepath);

    save_data->tf2_live_log_fullname = cider_construct_fullname(strncpy(malloc(save_data->tf2_filepath_len + 2), save_data->tf2_filepath, save_data->tf2_filepath_len + 2), TF2PW_LOG_SEMINAME);
    TF2_PLAYED_WITH_DEBUG_LOGF("Set tf2_live_log_fullname to \"%s\".\n", save_data->tf2_live_log_fullname);

    save_data->player_records = malloc(save_data->player_records_len * sizeof(*save_data->player_records));
    for (uint_fast32_t player_i = 0; player_i < save_data->player_records_len; ++ player_i)
    {
        fread_one(save_data->player_records[player_i].sid3e);
        fread_one(save_data->player_records[player_i].record_messages);

        // IMMED_TODO START: Reset back to buffering solution, seems to have had an error which impedes my current work
        save_data->player_records[player_records_i].notes = NULL;
        if (0 == file_io_buffered_input(input_file_ptr, &save_data->player_records[player_records_i].notes, "", 1))
        {
            free(save_data->player_records[player_i].notes);
            save_data->player_records[player_i].notes = NULL;
        }
        // IMMED_TODO END

        fread_one(save_data->player_records[player_i].date_records_len);

        char *last_real_name;
        save_data->player_records[player_i].date_records = malloc(sizeof(*save_data->player_records[player_i].date_records) * save_data->player_records[player_i].date_records_len);
        for (uint_fast32_t date_records_i = 0; date_records_i < save_data->player_records[player_i].date_records_len; ++date_records_i)
        {
            fread_one(save_data->player_records[player_i].date_records[date_records_i].date);
            fread_one(save_data->player_records[player_i].date_records[date_records_i].encounter_count);
            fread_one(save_data->player_records[player_i].date_records[date_records_i].name_len);

            // Only read real names, else set ptr to original
            if (save_data->player_records[player_i].date_records[date_records_i].name_len > 0)
            {
                save_data->player_records[player_i].date_records[date_records_i].name = malloc(sizeof(char) * (save_data->player_records[player_i].date_records[date_records_i].name_len + 1));
                fread_arr(save_data->player_records[player_i].date_records[date_records_i].name);
                save_data->player_records[player_i].date_records[date_records_i].name[save_data->player_records[player_i].date_records[date_records_i].name_len] = '\0';

                last_real_name = save_data->player_records[player_i].date_records[date_records_i].name;
            }
            else
            {
                save_data->player_records[player_i].date_records[date_records_i].name = last_real_name;
            }

            // Only read messages if they exist aka. record_messages == true
            if (save_data->player_records[player_i].record_messages)
            {
                size_t msg_len = 0;
                save_data->player_records[player_i].date_records[date_records_i].messages_len = 0;

                int messages_input = fgetc(input_file_ptr);

                // If no messages, store nothing for consistency with live log behavior
                if ((char) messages_input == '\0')
                {
                    save_data->player_records[player_i].date_records[date_records_i].messages = NULL;
                    goto STOP_READING_MESSAGES;
                }
                else
                {
                    save_data->player_records[player_i].date_records[date_records_i].messages = malloc(sizeof(char *) * ++save_data->player_records[player_i].date_records[date_records_i].messages_len);
                    save_data->player_records[player_i].date_records[date_records_i].messages[0] = NULL;
                }

                while (1)
                {
                    switch (messages_input)
                    {
                        break; case '\0':
                        {
                            prealloc(save_data->player_records[player_i].date_records[date_records_i].messages[save_data->player_records[player_i].date_records[date_records_i].messages_len - 1], msg_len + 1);
                            save_data->player_records[player_i].date_records[date_records_i].messages[save_data->player_records[player_i].date_records[date_records_i].messages_len - 1][msg_len] = '\0';

                            goto STOP_READING_MESSAGES;
                        }
                        break; case '\n':
                        {
                            prealloc(save_data->player_records[player_i].date_records[date_records_i].messages[save_data->player_records[player_i].date_records[date_records_i].messages_len - 1], msg_len + 1);
                            save_data->player_records[player_i].date_records[date_records_i].messages[save_data->player_records[player_i].date_records[date_records_i].messages_len - 1][msg_len] = '\0';

                            prealloc(save_data->player_records[player_i].date_records[date_records_i].messages, ++save_data->player_records[player_i].date_records[date_records_i].messages_len);
                            save_data->player_records[player_i].date_records[date_records_i].messages[save_data->player_records[player_i].date_records[date_records_i].messages_len - 1] = NULL;
                            msg_len = 0;
                        }
                        break; case EOF:
                        {
                            fprintf(stderr, ANSI_RED "Reached end of history file before finishing parsing, file corruption likely.\n" ANSI_RESET);
                            return true;
                        }
                        break; default:
                        {
                            prealloc(save_data->player_records[player_i].date_records[date_records_i].messages[save_data->player_records[player_i].date_records[date_records_i].messages_len - 1], ++msg_len);
                            save_data->player_records[player_i].date_records[date_records_i].messages[save_data->player_records[player_i].date_records[date_records_i].messages_len - 1][msg_len - 1] = (char) messages_input;
                        }
                    }

                    messages_input = fgetc(input_file_ptr);
                }
                STOP_READING_MESSAGES:;
            }
        }
    }

    return false;
}

static int date_record_0_compare(const struct date_record_0 *date_record_a, const struct date_record_0 *date_record_b)
{
    return (date_record_a->date - date_record_b->date);
}

static int player_record_0_compare(const struct player_record_0 *player_record_a, const struct player_record_0 *player_record_b)
{
    return (player_record_a->sid3e - player_record_b->sid3e);
}

bool save_format_0_modernize(void *const save_data)
{
    const struct save_format_0 *const old_data = save_data;

    struct save_format_1 new_data =
    {
        .current_date = old_data->current_date,
        .default_record_messages = old_data->default_record_messages,
        .player_records = old_data->player_records,
        .player_records_len = old_data->player_records_len,
        .tf2_filepath = old_data->tf2_filepath,
        .tf2_filepath_len = old_data->tf2_filepath_len,
        .tf2_live_log_fullname = old_data->tf2_live_log_fullname,
        .user_sid3e = old_data->user_sid3e
    };

    if (save_data != memcpy(save_data, &new_data, sizeof(struct save_format_1)))
    {
        fputs(ANSI_RED "Failed to modernize save data v0 -> v1.\n" ANSI_RESET, stderr);
        return true;
    }

    qsort(new_data.player_records, new_data.player_records_len, sizeof(struct player_record_0), (__compar_fn_t) player_record_0_compare);
    for (uint_fast32_t player_i = 0; player_i < new_data.player_records_len; ++player_i)
    {
        qsort(new_data.player_records[player_i].date_records, new_data.player_records[player_i].date_records_len, sizeof(struct date_record_0), (__compar_fn_t) date_record_0_compare);

        // IMMED_TODO: Uses a ton of extra space to set names straight, not smart at all. However, it works, and that'll do for now
        for (uint_fast32_t date_i = 0; date_i < new_data.player_records[player_i].date_records_len; ++date_i)
        {
            if (new_data.player_records[player_i].date_records[date_i].name_len == 0)
            {
                new_data.player_records[player_i].date_records[date_i].name_len = (uint_fast8_t) strlen(new_data.player_records[player_i].date_records[date_i].name);
                new_data.player_records[player_i].date_records[date_i].name = strcpy(malloc(new_data.player_records[player_i].date_records[date_i].name_len + 1), new_data.player_records[player_i].date_records[date_i].name);
            }
        }

        char *last_real_name = new_data.player_records[player_i].date_records[0].name;
        for (uint_fast32_t date_i = 1; date_i < new_data.player_records[player_i].date_records_len; ++date_i)
        {
            if (!strcmp(last_real_name, new_data.player_records[player_i].date_records[date_i].name))
            {
                free(new_data.player_records[player_i].date_records[date_i].name);
                new_data.player_records[player_i].date_records[date_i].name_len = 0;
                new_data.player_records[player_i].date_records[date_i].name = last_real_name;
            }
            else
            {
                last_real_name = new_data.player_records[player_i].date_records[date_i].name;
            }
        }
    }

    return false;
}

bool save_format_0_free(struct save_format_0 *save_data)
{
    if (!save_data->player_records_len)
    {
        TF2_PLAYED_WITH_DEBUG_LOGS("Attempted save_format_0_free(...) while player_records_len == 0, ignoring.\n");
        return false;
    }

    for (uint32_t player_i = 0; player_i < save_data->player_records_len; ++player_i)
    {
        for (uint32_t date_i = 0; date_i < save_data->player_records[player_i].date_records_len; ++date_i)
        {
            if (save_data->player_records[player_i].date_records[date_i].name_len > 0)
            {
                free(save_data->player_records[player_i].date_records[date_i].name);
            }

            if (save_data->player_records[player_i].record_messages && save_data->player_records[player_i].date_records[date_i].messages)
            {
                for (size_t msg_i = 0; msg_i < save_data->player_records[player_i].date_records[date_i].messages_len; ++msg_i)
                {
                    free(save_data->player_records[player_i].date_records[date_i].messages[msg_i]);
                }

                free(save_data->player_records[player_i].date_records[date_i].messages);
            }
        }

        free(save_data->player_records[player_i].date_records);
        free(save_data->player_records[player_i].notes);
    }

    free(save_data->player_records);
    free(save_data->tf2_filepath);
    free(save_data->tf2_live_log_fullname);

    return false;
}
