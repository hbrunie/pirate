/*
 * Copyright (C) 2012, Andreas Sandberg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PERF_COMMON_H
#define PERF_COMMON_H

#ifndef PFM_INC
#include <perfmon/pfmlib_perf_event.h>
#define PFM_INC
#endif

#include <argp.h> 

typedef struct ctr {
    struct perf_event_attr attr;
    const char *event_name;
    int fd;
    struct ctr *next;
} ctr_t;

typedef struct {
    struct ctr *head;
    struct ctr *tail;
} ctr_list_t;

extern struct perf_event_attr perf_base_attr;
extern ctr_list_t perf_ctrs;

extern long perf_argp_parse_long(const char *name, const char *arg,
                                 struct argp_state *state);

size_t write_all(int fd, const void *buf, size_t size);

void *mem_huge_alloc(size_t size);
void mem_huge_free(void *addr, size_t size);

/**
 * Create a counter structure and initialize the attributes structure with base_attr.
 *
 * @param base_attr Base attributes for counter or NULL for to initialize to all 0.
 * @return Pointer to ctr_t structure or NULL on error.
 */
ctr_t *ctr_create(const struct perf_event_attr *base_attr);

/**
 * Attach a counter to a running process and cpu combination.
 *
 * @param ctr Counter to attach
 * @param pid PID of process to attach to. Special values: 0 for
 *            current process, -1 for all processes
 * @param cpu Restrict counters to a CPU, -1 to allow all CPUs.
 * @param group_fd fd of perfromance counter group leader, -1 to
 *                 create a new group.
 * @param flags Perf event flags, see kernel docs/sources
 *
 * @return -1 on error, counter fd on success
 */
int ctr_attach(ctr_t *ctr, pid_t pid, int cpu, int group_fd, int flags);


/**
 * Close all counters in a list. Counters with fd == -1 are ignored.
 */
void ctrs_close(ctr_list_t *list);

/**
 * Add a counter to a counter list
 */
ctr_t *ctrs_add(ctr_list_t *list, ctr_t *ctr);

/**
 * Count the number of counters in a list
 */
int ctrs_len(ctr_list_t *list);

/**
 * Copy a list of counter configurations.
 *
 * @note Only counter definitions are included. Fields initialized
 * when attaching (e.g., fd) are initialized to their default values.
 *
 * @param dest Destionation list. Must be empty.
 * @param src Source list.o
 */
// void ctrs_cpy_conf(ctr_list_t *dest, ctr_list_t *src);

/**
 * Attach all counters in a list to a process/cpu combination.
 *
 * @param list List of counters to attach
 * @param pid PID of process to attach to. Special values: 0 for
 *            current process, -1 for all processes
 * @param cpu Restrict counters to a CPU, -1 to allow all CPUs.
 * @param group_fd fd of perfromance counter group leader, -1 to
 *                 create a new group.
 * @param flags Perf event flags, see kernel docs/sources
 *
 * @return >= 0 on success, -1 on error
 */
int ctrs_attach(ctr_list_t *list, pid_t pid, int cpu, int flags);

/**
 * Execute a process and attach the counter list to the child process.
 * See exec(3).
 *
 * @param ctr Counter to attach
 * @param cpu Restrict counters to a CPU, -1 to allow all CPUs.
 * @param group_fd fd of perfromance counter group leader, -1 to
 *                 create a new group.
 * @param flags Perf event flags, see kernel docs/sources
 * @param file Binary to execute.
 * @param argv Argument vector
 *
 * @return >= 0 on success, -1 on error
 */
pid_t ctrs_execvp(ctr_list_t *list, int cpu, int flags,
                  const char *file, char *const argv[]);

/**
 * Execute a process and attach the counter list to the child process.
 * See exec(3). Allows the user to specify a callback to be called
 * from the child process before calling execv to start the new
 * binary.
 *
 * @param ctr Counter to attach
 * @param cpu Restrict counters to a CPU, -1 to allow all CPUs.
 * @param group_fd fd of perfromance counter group leader, -1 to
 *                 create a new group.
 * @param flags Perf event flags, see kernel docs/sources
 * @param child_callback Function to call before calling exec in the
 *                       child process.
 * @param callback_data Data to pass to the child callback function.
 * @param file Binary to execute.
 * @param argv Argument vector
 *
 * @return >= 0 on success, -1 on error
 */
pid_t ctrs_execvp_cb(ctr_list_t *list, int cpu, int flags,
                     void (*child_callback)(void *data), void *callback_data,
                     const char *file, char *const argv[]);


/**
 * Takes a perfmon event name and attaches the corresponding counter to
 * the counter list
 *
 * @param event Event name for perfmon
 * @param list List of counters to attach
 */
void setup_ctr(const char *event, ctr_list_t *ctrs_list);

/**
 * Takes a raw perf event name and attaches the corresponding counter to
 * the counter list
 *
 * @param event Raw event name starting with "raw:"
 * @param list List of counters to attach
 */
void setup_raw_ctr(const char *event, ctr_list_t *ctrs_list);


#endif

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * c-file-style: "k&r"
 * End:
 */
