// mac_log.cpp
// Stored line: [YYYY-MM-DD HH:MM:SS] pid:<pid> [LEVEL] @<file> :] message
// Console warning label is WARN. The stored label is WARNING.

#include "player_project.h"
#include "mac_log.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int ciao_shutdown_seconds = 0;
int ciao_bookmark_index = 0;
bool ciao_quiet_mode = false;
bool ciao_json_mode = false;

static bool g_debug = false;
static bool g_in_append = false;
static bool g_dir_error_reported = false;
static char g_log_dir[1024];

void mac_log_init(void)
{
    g_log_dir[0] = '\0';
    g_debug = false;
    g_dir_error_reported = false;
}

void mac_log_set_directory(const char *directory)
{
    if (directory == NULL) {
        g_log_dir[0] = '\0';
        return;
    }
    snprintf(g_log_dir, sizeof g_log_dir, "%s", directory);
}

void mac_log_set_debug(bool enabled)
{
    g_debug = enabled;
}

static void mkdir_one(const char *path)
{
    if (path == NULL || path[0] == '\0') {
        return;
    }
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        /* parent may be missing; caller walks the path */
    }
}

static void mkdir_parents(const char *path)
{
    char buf[1024];
    snprintf(buf, sizeof buf, "%s", path);
    size_t n = strlen(buf);
    for (size_t i = 1; i < n; i++) {
        if (buf[i] != '/') {
            continue;
        }
        buf[i] = '\0';
        mkdir_one(buf);
        buf[i] = '/';
    }
    mkdir_one(buf);
}

static const char *file_base(const char *file)
{
    const char *base = (file != NULL) ? file : "";
    for (const char *p = base; *p != '\0'; p++) {
        if (*p == '/') {
            base = p + 1;
        }
    }
    return base;
}

static void console_line(CiaoOutputLevel level, const char *message)
{
    if (ciao_quiet_mode && level != CIAO_LEVEL_ERROR) {
        return;
    }
    if (level == CIAO_LEVEL_DEBUG && !g_debug) {
        return;
    }
    const char *label =
        (level == CIAO_LEVEL_WARNING) ? "WARN" :
        (level == CIAO_LEVEL_ERROR) ? "ERROR" :
        (level == CIAO_LEVEL_DEBUG) ? "DEBUG" : "INFO";
    FILE *out = (level == CIAO_LEVEL_ERROR) ? stderr : stdout;
    fprintf(out, "[%s] %s\n", label, (message != NULL) ? message : "(null)");
    fflush(out);
}

static void append_stored(CiaoOutputLevel level, const char *file, const char *message)
{
    if (g_in_append || g_log_dir[0] == '\0') {
        return;
    }
    g_in_append = true;

    time_t now = time(NULL);
    struct tm local_tm;
    localtime_r(&now, &local_tm);
    char civil[32];
    char stamp[16];
    snprintf(civil, sizeof civil, "%04d-%02d-%02d %02d:%02d:%02d",
             local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
             local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
    snprintf(stamp, sizeof stamp, "%04d%02d%02d",
             local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday);

    mkdir_parents(g_log_dir);

    char path[1200];
    snprintf(path, sizeof path, "%s/%s-%s%s", g_log_dir, PROJECT_ID, stamp, LOGGER_FILE_EXT);

    const char *level_name =
        (level == CIAO_LEVEL_WARNING) ? "WARNING" :
        (level == CIAO_LEVEL_ERROR) ? "ERROR" :
        (level == CIAO_LEVEL_DEBUG) ? "DEBUG" : "INFO";

    FILE *f = fopen(path, "ab");
    if (f == NULL) {
        if (!g_dir_error_reported) {
            fprintf(stderr, "[ERROR] cannot open log file\n");
            g_dir_error_reported = true;
        }
        g_in_append = false;
        return;
    }
    fprintf(f, "[%s] pid:%lu [%s] @%s :] %s\n",
            civil,
            (unsigned long)getpid(),
            level_name,
            file_base(file),
            (message != NULL) ? message : "(null)");
    fclose(f);
    g_in_append = false;
}

void ciao_log(CiaoOutputLevel level, const char *file, const char *format, va_list ap)
{
    if (level == CIAO_LEVEL_DEBUG && !g_debug) {
        return;
    }
    char message[2048];
    va_list copy;
    va_copy(copy, ap);
    vsnprintf(message, sizeof message, (format != NULL) ? format : "", copy);
    va_end(copy);
    message[sizeof message - 1] = '\0';
    console_line(level, message);
    append_stored(level, file, message);
}

void ciao_info_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_INFO, file, format, ap);
    va_end(ap);
}

void ciao_warn_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_WARNING, file, format, ap);
    va_end(ap);
}

void ciao_error_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_ERROR, file, format, ap);
    va_end(ap);
}

void ciao_debug_at(const char *file, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    ciao_log(CIAO_LEVEL_DEBUG, file, format, ap);
    va_end(ap);
}
