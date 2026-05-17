#include "version_1.h"

#include "main.h"
#include "../file_io.h"

#include "cider.h"

#include "string.h"
#include "inttypes.h"

bool save_format_1_save(const struct save_format_1 *save_data, FILE *output_file_ptr)
{
    fwrite_one((uint8_t){ 1 });
    fwrite_one(save_data->user_sid3e);
    fwrite_one(save_data->default_record_messages);
    fwrite_one(save_data->tf2_filepath_len);
    fwrite_arr(save_data->tf2_filepath);
    fwrite_one(save_data->player_records_len);

    for (uint_fast32_t player_records_i = 0; player_records_i < save_data->player_records_len; ++player_records_i)
    {
        fwrite_one(save_data->player_records[player_records_i].sid3e);
        fwrite_one(save_data->player_records[player_records_i].record_messages);

        // Only write notes if they exist, else just '\0'
        if (save_data->player_records[player_records_i].notes)
        {
            fprintf(output_file_ptr, "%s", save_data->player_records[player_records_i].notes);
        }
        fputc('\0', output_file_ptr);

        fwrite_one(save_data->player_records[player_records_i].date_records_len);
        for (uint_fast32_t date_records_i = 0; date_records_i < save_data->player_records[player_records_i].date_records_len; ++date_records_i)
        {
            fwrite_one(save_data->player_records[player_records_i].date_records[date_records_i].date);
            fwrite_one(save_data->player_records[player_records_i].date_records[date_records_i].encounter_count);
            fwrite_one(save_data->player_records[player_records_i].date_records[date_records_i].name_len);

            // Only write real names
            if (save_data->player_records[player_records_i].date_records[date_records_i].name_len)
            {
                fwrite_arr(save_data->player_records[player_records_i].date_records[date_records_i].name);
            }

            // Only write messages if flag set
            if (save_data->player_records[player_records_i].record_messages)
            {
                // Only write messages if they are there to be written
                if (save_data->player_records[player_records_i].date_records[date_records_i].messages)
                {
                    for (size_t message_i = 0; message_i < save_data->player_records[player_records_i].date_records[date_records_i].messages_len; ++message_i)
                    {
                        fputs(save_data->player_records[player_records_i].date_records[date_records_i].messages[message_i], output_file_ptr);

                        if (message_i != save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1)
                        {
                            fputc('\n', output_file_ptr);
                        }
                    }
                }

                // Should write after messages, or by itself if there are no messages but flag is set
                fputc('\0', output_file_ptr);
            }
        }
    }

    return false;
}

bool save_format_1_load(struct save_format_1 *save_data, FILE *input_file_ptr)
{
    fread_one(save_data->user_sid3e);
    fread_one(save_data->default_record_messages);
    fread_one(save_data->tf2_filepath_len);

    save_data->tf2_filepath = malloc(save_data->tf2_filepath_len + 2);
    fread_arr(save_data->tf2_filepath);
    save_data->tf2_filepath[save_data->tf2_filepath_len] = CIDER_PATH_DELIM_C;
    save_data->tf2_filepath[save_data->tf2_filepath_len + 1] = '\0';

    TF2_PLAYED_WITH_DEBUG_LOGF("Set tf2_filepath to \"%s\".\n", save_data->tf2_filepath);

    save_data->tf2_live_log_fullname = cider_construct_fullname(strncpy(malloc(save_data->tf2_filepath_len + 2), save_data->tf2_filepath, save_data->tf2_filepath_len + 2), TF2PW_LOG_SEMINAME);
    TF2_PLAYED_WITH_DEBUG_LOGF("Set tf2_live_log_fullname to \"%s\".\n", save_data->tf2_live_log_fullname);

    fread_one(save_data->player_records_len);

    uint_fast32_t last_sid3e = 0;
    save_data->player_records = malloc(save_data->player_records_len * sizeof(*save_data->player_records));
    for (uint_fast32_t player_records_i = 0; player_records_i < save_data->player_records_len; ++ player_records_i)
    {
        fread_one(save_data->player_records[player_records_i].sid3e);
        if (save_data->player_records[player_records_i].sid3e < last_sid3e)
        {
            fprintf(stderr, ANSI_RED "Invalid history file (SID3E's not in ascending order).\n" ANSI_RESET);
            return true;
        }
        else
        {
            last_sid3e = save_data->player_records[player_records_i].sid3e;
        }

        fread_one(save_data->player_records[player_records_i].record_messages);

        const int notes_first = fgetc(input_file_ptr);
        if (notes_first == '\0')
        {
            ungetc(notes_first, input_file_ptr);
        }
        else
        {
            fprintf(stderr, "thing here\n"); // REMOVE
            save_data->player_records[player_records_i].notes = NULL;
            file_io_buffered_input(input_file_ptr, &save_data->player_records[player_records_i].notes);
        }

        fread_one(save_data->player_records[player_records_i].date_records_len);

        char *last_real_name;

        uint_fast16_t last_date = 0;
        save_data->player_records[player_records_i].date_records = malloc(sizeof(*save_data->player_records[player_records_i].date_records) * save_data->player_records[player_records_i].date_records_len);
        for (uint_fast32_t date_records_i = 0; date_records_i < save_data->player_records[player_records_i].date_records_len; ++date_records_i)
        {
            fread_one(save_data->player_records[player_records_i].date_records[date_records_i].date);
            if (save_data->player_records[player_records_i].date_records[date_records_i].date < last_date)
            {
                fprintf(stderr, ANSI_RED "Invalid history file (Dates not in ascending order, %" PRIu16 " < %" PRIuFAST16 ").\n" ANSI_RESET, save_data->player_records[player_records_i].date_records[date_records_i].date, last_date);
                return true;
            }
            else
            {
                last_date = save_data->player_records[player_records_i].date_records[date_records_i].date;
            }

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

bool save_format_1_free(struct save_format_1 *save_data)
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
