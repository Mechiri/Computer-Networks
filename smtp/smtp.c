#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>

typedef struct cfg {
  char *server;
  char *from_address;
  char *to_address;
  char *message;
} cfg_t;

char *body = NULL;

static char *payload_data[] = {
 "Subject: SMTP example message\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322 */
  "body",
  "\r\n",
  NULL
};


struct upload_status {
  int lines_read;
};

static size_t payload_func(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  data = payload_data[upload_ctx->lines_read];

  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;

    return len;
  }

  return 0;
}

int send_mail(cfg_t *cfg)
{
        printf("Arguments: %s %s %s %s\n", cfg->server, cfg->from_address, cfg->to_address, cfg->message);

        CURL *curl;
        CURLcode result = CURLE_OK;
        struct curl_slist *rcpt = NULL;
        struct upload_status upload_ctx;

        upload_ctx.lines_read = 0;

        curl = curl_easy_init();
        if(curl)
        {
                //smtp server
                char server[100] = "smtp://";
                strcat(server, cfg->server);
                curl_easy_setopt(curl, CURLOPT_URL, server);
                //sender and recepient
                curl_easy_setopt(curl, CURLOPT_MAIL_FROM, cfg->from_address);
                rcpt = curl_slist_append(rcpt, cfg->to_address);
                curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpt);
                //body of the message
                payload_data[2] = cfg->message;
                curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_func);
                curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

                //send smtp message
                result = curl_easy_perform(curl);

                if(result != CURLE_OK)
                  fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));

                curl_slist_free_all(rcpt);

                curl_easy_cleanup(curl);
        }
        return (int)result;
}

int main(int argc, char *argv[])
{
        int res;
        cfg_t cfg;

        if (argc < 5 || argc > 5) {
                fprintf(stderr, "Usage Example: ./smtp mail-relay.iu.edu <from> <to> <message>\n");
                exit(1);
        }

        cfg.server = argv[1];
        cfg.from_address = argv[2];
        cfg.to_address = argv[3];
        cfg.message = argv[4];

        res = send_mail(&cfg);
        return res;
}
