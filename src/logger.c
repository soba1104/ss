#include "ss.h"
#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

void ss_log(ss_logger *logger, int level, const char *format, ...) {
    // MEMO logger->level 参照時に排他制御をかけていないので、
    // log level 変更時に反映されないことがあるかもしれない。
    // しかし、ここで毎回 mutex を取りに行きたくないので困るまではこのままにする。
    if (level <= logger->level) {
        va_list ap;
        pthread_mutex_lock(&(logger->mutex));
        va_start(ap, format);
        logger->cbk(logger->cbk_arg, format, ap);
        va_end(ap);
        pthread_mutex_unlock(&(logger->mutex));
    }
}

void ss_logger_set_cbk(ss_logger *logger, ss_logger_cbk cbk, void *arg) {
    pthread_mutex_lock(&(logger->mutex));
    logger->cbk = cbk;
    logger->cbk_arg = arg;
    pthread_mutex_unlock(&(logger->mutex));
}

static void default_logger_cbk(void *arg, const char *format, va_list ap) {
    vdprintf((int)arg, format, ap);
}

void ss_logger_set_log_level(ss_logger *logger, int level) {
    pthread_mutex_lock(&(logger->mutex));
    logger->level = level;
    pthread_mutex_unlock(&(logger->mutex));
}

bool ss_logger_init(ss_logger *logger) {
    logger->level = SS_DEFAULT_LOG_LEVEL;
    logger->cbk = default_logger_cbk;
    logger->cbk_arg = (void*)STDERR_FILENO;
    if (pthread_mutex_init(&(logger->mutex), NULL) != 0) {
        goto err;
    }
    return true;

err:
    return false;
}
