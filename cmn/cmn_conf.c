#include "cmn_base.h"
#include "cmn_log.h"
#include "cmn_conf.h"

static void
cfg_token_copy(char *dst, const char *src)
{
    int len;
    int i, j;

    len = strlen(src);
    for (i = 0; i < len; i++) {
        if (src[i] != ' ' && src[i] != '\t') {
            break;
        }
    }

    for (j = len - 1; j >= 0; j--) {
        if (src[j] != ' ' && src[j] != '\t' && src[j] != '\n') {
            break;
        }
    }

    if (i <= j) {
        len = j - i + 1;
        strncpy(dst, &src[i], len);
        dst[len] = 0x0;
    } else {
        dst[0] = 0x0;
    }
}


int
cfg_get_key(char *filename, char *section, char *key, char *defval, char *data)
{
    FILE *fp = NULL;
    char buffer[MAX_PATH_LEN+1]={0};
    int  section_found = 0, section_len=0;
    char key_buf[MAX_KEY_LEN+1];
    int  key_found = 0;
    char value_buf[MAX_VALUE_LEN+1] = {0};
    char *pdest = NULL;

    snprintf(buffer, sizeof(buffer), "%s", filename);

    if ((fp = fopen(buffer, "r")) == NULL) {
        log_warn("fopen error, [file:%s][errno:%d]...", buffer, errno);
        strcpy(data, defval);
        return CMN_ERROR;
    }

    section_len = strlen(section);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (buffer[0] == '[') {
            if (section_found) {
                break;
            } else {
                if (strncasecmp(section, &buffer[1], section_len) == 0 && buffer[section_len + 1] == ']') {
                    section_found = 1;
                    continue;
                }
            }
        } else if (!section_found || buffer[0] == '#') {
            continue;
        }

        if ((pdest = strchr(buffer, '=')) == NULL) {
            continue;
        }
        *pdest = 0x0;
        cfg_token_copy(key_buf, buffer);

        if (strcasecmp(key, key_buf) == 0) {
            key_found = 1;
            cfg_token_copy(value_buf, pdest + 1);
            strcpy(data, value_buf);
            break;
        }
    }
    fclose(fp);

    if (!key_found) {
        strcpy(data, defval);
        return CMN_NOKEY;
    }
    return CMN_OK;
}



int
cfg_get_key_with_len(char *filename, char *section, char *key, char *defval, char *data, int length)
{
    FILE *fp = NULL;
    char buffer[MAX_PATH_LEN+1] = {0};
    int  section_found = 0, section_len = 0;
    char key_buf[MAX_KEY_LEN+1] = {0};
    int  key_found = 0, value_len = 0;
    char value_buf[MAX_VALUE_LEN+1] = {0};
    char *pdest = NULL;

    snprintf(buffer, sizeof(buffer), "%s", filename);

    if ((fp = fopen(buffer, "r")) == NULL) {
        log_warn("fopen error, [file:%s][errno:%d]...", buffer, errno);
        strcpy(data, defval);
        return CMN_ERROR;
    }

    section_len = strlen(section);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (buffer[0] == '[') {
            if (section_found) {
                break;
            }
            else {
                if (strncasecmp(section, &buffer[1], section_len) == 0 && buffer[section_len + 1] == ']') {
                    section_found = 1;
                    continue;
                }
            }
        } else if (!section_found || buffer[0] == '#') {
            continue;
        }

        if ((pdest = strchr(buffer, '=')) == NULL) {
            continue;
        }
        *pdest = 0x0;
        cfg_token_copy(key_buf, buffer);

        if (strcasecmp(key, key_buf) == 0) {
            key_found = 1;
            cfg_token_copy(value_buf, pdest + 1);
            value_len = strlen(value_buf);
            if (length > value_len) {
                strcpy(data, value_buf);
            } else {
                value_len = length - 1;
                strncpy(data, value_buf, value_len);
                data[value_len] = 0x0;
            }
            break;
        }
    }
    fclose(fp);

    if (!key_found) {
        strcpy(data, defval);
        return CMN_NOKEY;
    }

    return CMN_OK;
}




