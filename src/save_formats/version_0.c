#include "version_0.h"

#include "main.h"
#include "../file_io.h"
#include "version_1.h"

#include "cider.h"

#include "string.h"

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
    for (uint_fast32_t player_records_i = 0; player_records_i < save_data->player_records_len; ++ player_records_i)
    {
        fread_one(save_data->player_records[player_records_i].sid3e);
        fread_one(save_data->player_records[player_records_i].record_messages);

        // BUFF_TODO
        save_data->player_records[player_records_i].notes = NULL;
        int notes_input;
        size_t notes_len = 0;
        while ((notes_input = fgetc(input_file_ptr)) != '\0')
        {
            prealloc(save_data->player_records[player_records_i].notes, ++notes_len);
            save_data->player_records[player_records_i].notes[notes_len - 1] = (char) notes_input;
        }

        prealloc(save_data->player_records[player_records_i].notes, notes_len + 1);
        save_data->player_records[player_records_i].notes[notes_len] = '\0';

        // If just '\0', set to NULL
        if (notes_len == 0)
        {
            free(save_data->player_records[player_records_i].notes);
            save_data->player_records[player_records_i].notes = NULL;
        }

        fread_one(save_data->player_records[player_records_i].date_records_len);
        fprintf(stderr, "len: %d\n", save_data->player_records[player_records_i].date_records_len); // REMOVE

        char *last_real_name;

        save_data->player_records[player_records_i].date_records = malloc(sizeof(*save_data->player_records[player_records_i].date_records) * save_data->player_records[player_records_i].date_records_len);
        for (uint_fast32_t date_records_i = 0; date_records_i < save_data->player_records[player_records_i].date_records_len; ++date_records_i)
        {
            fread_one(save_data->player_records[player_records_i].date_records[date_records_i].date);
            fread_one(save_data->player_records[player_records_i].date_records[date_records_i].encounter_count);
            fread_one(save_data->player_records[player_records_i].date_records[date_records_i].name_len);

            // Only read real names, else set ptr to original
            if (save_data->player_records[player_records_i].date_records[date_records_i].name_len > 0)
            {
                save_data->player_records[player_records_i].date_records[date_records_i].name = malloc(sizeof(char) * (save_data->player_records[player_records_i].date_records[date_records_i].name_len + 1));
                fread_arr(save_data->player_records[player_records_i].date_records[date_records_i].name);
                save_data->player_records[player_records_i].date_records[date_records_i].name[save_data->player_records[player_records_i].date_records[date_records_i].name_len] = '\0';

                last_real_name = save_data->player_records[player_records_i].date_records[date_records_i].name;
            }
            else
            {
                save_data->player_records[player_records_i].date_records[date_records_i].name = last_real_name;
            }

            // Only read messages if they exist aka. record_messages == 1
            if (save_data->player_records[player_records_i].record_messages)
            {
                size_t msg_len = 0;
                save_data->player_records[player_records_i].date_records[date_records_i].messages_len = 0;

                int messages_input = fgetc(input_file_ptr);

                // If no messages, store nothing for consistency with live log behavior
                if ((char) messages_input == '\0')
                {
                    save_data->player_records[player_records_i].date_records[date_records_i].messages = NULL;
                    goto STOP_READING_MESSAGES;
                }
                else
                {
                    save_data->player_records[player_records_i].date_records[date_records_i].messages = malloc(sizeof(char *) * ++save_data->player_records[player_records_i].date_records[date_records_i].messages_len);
                    save_data->player_records[player_records_i].date_records[date_records_i].messages[0] = NULL;
                }

                while (1)
                {
                    switch (messages_input)
                    {
                        break; case '\0':
                        {
                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1], msg_len + 1);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1][msg_len] = '\0';

                            goto STOP_READING_MESSAGES;
                        }
                        break; case '\n':
                        {
                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1], msg_len + 1);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1][msg_len] = '\0';

                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages, ++save_data->player_records[player_records_i].date_records[date_records_i].messages_len);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1] = NULL;
                            msg_len = 0;
                        }
                        break; case EOF:
                        {
                            fprintf(stderr, ANSI_RED "Reached end of history file before finishing parsing, file corruption likely.\n" ANSI_RESET);
                            return true;
                        }
                        break; default:
                        {
                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1], ++msg_len);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1][msg_len - 1] = (char) messages_input;
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

bool save_format_0_modernize(void *save_data)
{
    fprintf(stderr, "yeag\n"); // REMOVE
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

    // Sort player and date records
    uint_fast32_t last_sid3e = 0;
    for (uint_fast32_t player_i = 0; player_i < new_data.player_records_len; ++player_i)
    {
        uint_fast16_t last_date = 0;
        for (uint_fast32_t date_i = 0; date_i < new_data.player_records[player_i].date_records_len; ++date_i)
        {
            const uint_fast16_t current_date = new_data.player_records[player_i].date_records[date_i].date;

            if (current_date < last_date)
            {
                fprintf(stderr, "SWAPPING %ld AND %ld\n", date_i, date_i - 1); // REMOVE
                // Swap previous and current records
                struct date_record_0 temp_dr;
                memcpy(&temp_dr, new_data.player_records[player_i].date_records + date_i - 1, sizeof(struct date_record_0));
                memcpy(new_data.player_records[player_i].date_records + date_i - 1, new_data.player_records[player_i].date_records + date_i, sizeof(struct date_record_0));
                memcpy(new_data.player_records[player_i].date_records + date_i, &temp_dr, sizeof(struct date_record_0));

                // Move back to previous record (accounting for subsequent increment)
                date_i -= 2;
            }

            last_date = current_date;
        }

        const uint_fast32_t current_sid3e = new_data.player_records[player_i].sid3e;

        if (current_sid3e < last_sid3e)
        {
            // Swap previous and current records
            struct player_record_0 temp_pr;
            memcpy(&temp_pr, new_data.player_records + player_i, sizeof(struct player_record_0));
            memcpy(new_data.player_records + player_i - 1, new_data.player_records + player_i, sizeof(struct player_record_0));
            memcpy(new_data.player_records + player_i, &temp_pr, sizeof(struct player_record_0));

            // Move back to previous record (accounting for subsequent increment)
            player_i -= 2;
        }

        last_sid3e = current_sid3e;
    }

    if (save_data == memcpy(save_data, &new_data, sizeof(struct save_format_1)))
    {
        return false;
    }
    else
    {
        fputs(ANSI_RED "Failed to modernize save data v0 -> v1.\n" ANSI_RESET, stderr);
        return true;
    }
}

bool save_format_0_free(struct save_format_0 *save_data)
{
    if (!save_data->player_records_len)
    {
        TF2_PLAYED_WITH_DEBUG_LOGS("Attempted history_free(...) while player_records_len == 0, ignoring.\n");
        return false;
    }

    for (uint32_t player_i = 0; player_i < save_data->player_records_len; ++player_i)
    {
        for (uint32_t date_i = 0; date_i < save_data->player_records[player_i].date_records_len; ++date_i)
        {
            if (save_data->player_records[player_i].date_records[date_i].name_len)
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
