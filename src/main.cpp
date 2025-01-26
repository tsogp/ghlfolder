#include <curl/curl.h>
#include <iostream>
#include <memory>

int main(int argc, char **argv) {
    CURL *handler = curl_easy_init();

    curl_easy_setopt(handler, CURLOPT_URL, "http://example.com");
    curl_easy_setopt(handler, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(handler, CURLOPT_USERAGENT, "curl/7.45.0");
    curl_easy_setopt(handler, CURLOPT_MAXREDIRS, 50L);

    CURLcode ret = curl_easy_perform(handler);

    curl_easy_cleanup(handler);

    return static_cast<int>(ret);
}