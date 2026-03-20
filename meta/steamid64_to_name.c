#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "stdint.h"
#include "inttypes.h"
#include "string.h"
#include "curl/curl.h"

// REMINDER: Link to libcurl during compilation

struct string
{
    char *contents;
    size_t len;
};

static size_t curl_write_func(void *input, size_t size, size_t count, void *void_string)
{
    struct string *const output_string = void_string;

    const size_t
        old_len = output_string->len,
        new_len = old_len + count
    ;

    output_string->contents = realloc(output_string->contents, new_len);
    output_string->len = new_len;

    memcpy(output_string->contents + old_len, input, count);

    return count;
}

char *steamid64_to_name(uint_fast64_t steamid64)
{
    CURL *const curl = curl_easy_init();
    if (!curl)
    {
        fputs("Failed Initializing Curl\n", stderr);
        return NULL;
    }

    char steam_link[sizeof("steamcommunity.com/profiles/XXXXXXXXXXXXXXXXX")];
    sprintf(steam_link, "steamcommunity.com/profiles/%" PRIuFAST64, steamid64);

    curl_easy_setopt(curl, CURLOPT_URL, steam_link);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    struct string output_string = { NULL, 0 };
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output_string);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);

    CURLcode curl_result = curl_easy_perform(curl);
    if (curl_result != CURLE_OK)
    {
        fprintf(stderr, "Could Not Fetch Webpage\nError: %s\n", curl_easy_strerror(curl_result));
        return NULL;
    }

    (output_string.contents = realloc(output_string.contents, output_string.len + 1))[output_string.len] = '\0';

    curl_easy_cleanup(curl);

    puts("Successfully downloaded data from steamcommunity.com.");

    const char name_matcher[] = "<span class=\"actual_persona_name\">";

    char *steam_name_excerpt_begin = strstr(output_string.contents, name_matcher);
    if (!steam_name_excerpt_begin)
    {
        fputs("Did not find name match in data. Is the ID correct?\n", stderr);
        free(output_string.contents);
        return NULL;
    }

    char *steam_name_excerpt_end   = strstr(steam_name_excerpt_begin + sizeof(name_matcher), "</span>");

    const size_t steam_name_len = steam_name_excerpt_end - steam_name_excerpt_begin - sizeof(name_matcher) + 1;
    char *steam_name = malloc(steam_name_len + 1);
    steam_name[steam_name_len] = '\0';

    memcpy(steam_name, steam_name_excerpt_begin + sizeof(name_matcher) - 1, steam_name_len);

    free(output_string.contents);
    return steam_name;
}

int main(int argc, char **argv)
{
    --argc; ++argv;

    if (!argc)
    {
        fputs("Requires argv[0] to be STEAMID64.\n", stderr);
        return 1;
    }

    char *end;
    const uint_fast64_t steamid64 = strtoull(argv[0], &end, 10);
    if (end == argv[0] || end[0] != '\0' || errno == ERANGE)
    {
        fprintf(stderr, "Bad input: \"%s\"\n", argv[0]);
        return 1;
    }

    puts("Loading name from steamcommunity.com...");
    char *name = steamid64_to_name(steamid64);
    if (name)
    {
        printf("STEAMID64 %lu = STEAM NAME \"%s\"\n", steamid64, name);
        free(name);
    }
    else
    {
        fputs("Failed during conversion.\n", stderr);
        return 1;
    }
}
