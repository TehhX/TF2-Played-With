#include "version_0.h"

#include "cider.h"

#include "string.h"

bool save_format_0_save(const struct save_format_0 *save_data, FILE *output_file_ptr)
{
    fwrite_one((uint8_t){ 0 });
    fwrite_one(save_data->user_sid3e);
    fwrite_one(save_data->default_record_messages);
    fwrite_one(save_data->player_records_len);
    fwrite_one(save_data->tf2_filepath_len);
    fwrite_arr(save_data->tf2_filepath);

    for (uint_fast32_t player_records_i = 0; player_records_i < save_data->player_records_len; ++player_records_i)
    {
        fwrite_one(save_data->player_records[player_records_i].sid3e);
        fwrite_one(save_data->player_records[player_records_i].record_messages);

        // Only write notes if they exist, else just '\0'
        if (save_data->player_records[player_records_i].notes)
        {
            fprintf(output_file_ptr, "%s", save_data->player_records[player_records_i].notes);
        }
        putc('\0', output_file_ptr);

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

struct save_format_0 *save_format_0_load(struct save_format_0 *save_data, FILE *input_file_ptr)
{
    save_data = malloc(sizeof(*save_data));

    fread_one(save_data->user_sid3e);
    fread_one(save_data->default_record_messages);
    fread_one(save_data->player_records_len);
    fread_one(save_data->tf2_filepath_len);

    save_data->tf2_filepath = malloc(save_data->tf2_filepath_len + 2);
    fread(save_data->tf2_filepath, sizeof(char), save_data->tf2_filepath_len, input_file_ptr);
    save_data->tf2_filepath[save_data->tf2_filepath_len] = CIDER_PATH_DELIM_C;
    save_data->tf2_filepath[save_data->tf2_filepath_len + 1] = '\0';

    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set tf2_filepath to \"%s\".\n" ANSI_RESET, save_data->tf2_filepath);

    save_data->tf2_live_log_fullname = cider_construct_fullname(strncpy(malloc(save_data->tf2_filepath_len + 2), save_data->tf2_filepath, save_data->tf2_filepath_len + 2), TF2PW_LOG_SEMINAME);
    TF2_PLAYED_WITH_DEBUG_LOGF(ANSI_LOG "LOG: Set tf2_live_log_fullname to \"%s\".\n" ANSI_RESET, save_data->tf2_live_log_fullname);

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
            prealloc(save_data->player_records[player_records_i].notes, sizeof(char), ++notes_len);
            save_data->player_records[player_records_i].notes[notes_len - 1] = (char) notes_input;
        }

        prealloc(save_data->player_records[player_records_i].notes, sizeof(char), notes_len + 1);
        save_data->player_records[player_records_i].notes[notes_len] = '\0';

        // If just '\0', set to NULL
        if (notes_len == 0)
        {
            free(save_data->player_records[player_records_i].notes);
            save_data->player_records[player_records_i].notes = NULL;
        }

        fread_one(save_data->player_records[player_records_i].date_records_len);

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
                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1], sizeof(char), msg_len + 1);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1][msg_len] = '\0';

                            goto STOP_READING_MESSAGES;
                        }
                        break; case '\n':
                        {
                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1], sizeof(char), msg_len + 1);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1][msg_len] = '\0';

                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages, sizeof(char *), ++save_data->player_records[player_records_i].date_records[date_records_i].messages_len);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1] = NULL;
                            msg_len = 0;
                        }
                        break; case EOF:
                        {
                            fprintf(stderr, ANSI_RED "MAJOR: Reached end of history file before finishing parsing, file corruption likely.\n" ANSI_RESET);
                            return NULL;
                        }
                        break; default:
                        {
                            prealloc(save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1], sizeof(char), ++msg_len);
                            save_data->player_records[player_records_i].date_records[date_records_i].messages[save_data->player_records[player_records_i].date_records[date_records_i].messages_len - 1][msg_len - 1] = (char) messages_input;
                        }
                    }

                    messages_input = fgetc(input_file_ptr);
                }
                STOP_READING_MESSAGES:;
            }
        }
    }

    return save_data;
}
