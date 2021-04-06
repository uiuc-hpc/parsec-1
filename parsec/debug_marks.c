/*
 * Copyright (c) 2013-2018 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 */
#include "parsec/parsec_config.h"

#if defined(PARSEC_DEBUG_HISTORY)
#include "parsec/debug_marks.h"
#include "parsec/utils/debug.h"
#include "parsec/parsec_internal.h"
#include "parsec/parsec_remote_dep.h"

void debug_mark_exe(int th, int vp, const struct parsec_task_s *ctx)
{
    int j, pos = 0, len = 512;
    char msg[512];

    pos += snprintf(msg+pos, len-pos, "%s(", ctx->task_class->name);
    for(j = 0; j < ctx->task_class->nb_parameters; j++) {
        pos += snprintf(msg+pos, len-pos, "locals[%d](%s)=%d%s",
                        j, ctx->task_class->locals[j]->name, ctx->locals[j].value,
                        (j == ctx->task_class->nb_parameters-1) ? "" : ", ");
    }
    pos += snprintf(msg+pos, len-pos, ")\n");

    parsec_debug_history_add("Mark: thread %2d VP %d executes:\t%s",
                            th, vp, msg);
}

void debug_mark_ctl_msg_activate_sent(int to, const void *b, const struct remote_dep_wire_activate_s *m)
{
    int j, pos = 0, len = 512;
    char msg[512];
    parsec_taskpool_t *tp;
    const parsec_task_class_t *tc;

    pos += snprintf(msg+pos, len-pos, "Mark: emission of an activate message to %d\n", to);
    pos += snprintf(msg+pos, len-pos, "\t      Using buffer %p for emision\n", b);
    tp = parsec_taskpool_lookup( m->taskpool_id );
    tc = tp->task_classes_array[m->task_class_id];
    pos += snprintf(msg+pos, len-pos, "\t      Activation passed=%s(", tc->name);
    for(j = 0; j < tc->nb_parameters; j++) {
        pos += snprintf(msg+pos, len-pos, "locals[%d](%s)=%d%s",
                        j,
                        tc->locals[j]->name, m->locals[j].value,
                        (j == tc->nb_parameters - 1) ? ")\n" : ", ");
    }
    pos += snprintf(msg+pos, len-pos, "\toutput_mask = 0x%08x\n",
                    (uint32_t)m->output_mask);

    /* Do not use set_my_mark: msg is a stack-allocated buffer */
    parsec_debug_history_add("%s", msg);
}

void debug_mark_ctl_msg_activate_recv(int from, const void *b, const struct remote_dep_wire_activate_s *m)
{
    int j, pos = 0, len = 512;
    char msg[512];
    parsec_taskpool_t *tp;
    const parsec_task_class_t *tc;

    pos += snprintf(msg+pos, len-pos, "Mark: reception of an activate message from %d\n", from);
    pos += snprintf(msg+pos, len-pos, "\t      Using buffer %p for reception\n", b);
    tp = parsec_taskpool_lookup( m->taskpool_id );
    tc = tp->task_classes_array[m->task_class_id];
    pos += snprintf(msg+pos, len-pos, "\t      Activation passed=%s(", tc->name);
    for(j = 0; j < tc->nb_parameters; j++) {
        pos += snprintf(msg+pos, len-pos, "locals[%d](%s)=%d%s",
                        j,
                        tc->locals[j]->name, m->locals[j].value,
                        (j == tc->nb_parameters - 1) ? ")\n" : ", ");
    }
    pos += snprintf(msg+pos, len-pos, "\toutput_mask = 0x%08x\n",
                    (uint32_t)m->output_mask);
    pos += snprintf(msg+pos, len-pos, "\t      deps = %p\n",
                    (uint32_t)m->deps);

    /* Do not use set_my_mark: msg is a stack-allocated buffer */
    parsec_debug_history_add("%s", msg);
}

void debug_mark_ctl_msg_get_sent(int to, const void *b, const struct remote_dep_wire_get_s *m)
{
    parsec_debug_history_add("Mark: emission of a Get control message to %d\n"
                            "\t      Using buffer %p for emission\n"
                            "\t      deps requested = %p\n"
                            "\t      which requested = 0x%08x\n"
                            "\t      remote_callback_data = %p\n",
                            to, b, m->source_deps, (uint32_t)m->output_mask, m->remote_callback_data);
}

void debug_mark_ctl_msg_get_recv(int from, const void *b, const struct remote_dep_wire_get_s *m)
{
    parsec_debug_history_add("Mark: reception of a Get control message from %d\n"
                            "\t      Using buffer %p for reception\n"
                            "\t      deps requested = %p\n"
                            "\t      which requested = 0x%08x\n"
                            "\t      remote_callback_data = %p\n",
                            from, b, m->source_deps, (uint32_t)m->output_mask, m->remote_callback_data);
}

void debug_mark_dta_put_start(int to, const struct remote_dep_cb_data_s *cb_data, uintptr_t r_cb_data)
{
    parsec_debug_history_add("Mark: Start emitting data to %d\n"
                            "\t      deps = %p\n"
                            "\t      which = 0x%08x\n"
                            "\t      remote_callback_data = %p\n",
                            to, cb_data->deps, (uint32_t)(1U << cb_data->k), r_cb_data);
}

void debug_mark_dta_put_end(int to, const struct remote_dep_cb_data_s *cb_data)
{
    parsec_debug_history_add("Mark: Done emitting data to %d\n"
                             "\t      deps = %p\n"
                             "\t      which = 0x%08x\n",
                             to, cb_data->deps, (uint32_t)(1U << cb_data->k));
}

void debug_mark_dta_put_recv(int from, const struct remote_dep_cb_data_s *cb_data)
{
    parsec_debug_history_add("Mark: Done receiving data from %d\n"
                            "\t      deps = %p\n"
                            "\t      which = 0x%08x\n",
                            from, cb_data->deps, (uint32_t)(1U << cb_data->k));
}

#endif /* defined(PARSEC_DEBUG_HISTORY) */
