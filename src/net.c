#include "tinypkg.h"
#include <curl/curl.h>
#include <stdio.h>

void download(const char *url, const char *out) {
    CURL *c = curl_easy_init();
    FILE *f = fopen(out, "wb");

    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, f);
    curl_easy_perform(c);

    fclose(f);
    curl_easy_cleanup(c);
}
