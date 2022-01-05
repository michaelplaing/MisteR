// memory.c

#include <jemalloc/jemalloc.h>
#include <string.h>
#include <errno.h>

#include "mister/mister.h"
#include "mister/mrzlog.h"
#include "memory_internal.h"

int mr_calloc(void **ppv, size_t count, size_t size) {
    if (!count) count = 1; // always allocate something even if size is 0
    if (!size) size = 1;
    *ppv = calloc(count, size);

    if (!*ppv) {
        mr_errno = errno;
        dzlog_error("calloc error: %d %s", errno, strerror(errno));
        return -1;
    }

    return 0;
}

int mr_malloc(void **ppv, size_t size) {
    if (!size) size = 1; // always allocate something even if size is 0
    *ppv = malloc(size);

    if (!*ppv) {
        mr_errno = errno;
        dzlog_error("malloc error: %d %s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

int mr_realloc(void **ppv, size_t size) {
    *ppv = realloc(*ppv, size);

    if (!*ppv) {
        mr_errno = errno;
        dzlog_error("realloc error: %d %s", errno, strerror(errno));
        return -1;
    }

    return 0;
}
