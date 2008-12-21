/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008  Kouhei Sutou <kou@cozmixng.org>
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gcutter.h>

#define shutdown inet_shutdown
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <milter/client.h>
#include <milter-test-utils.h>
#undef shutdown

void test_progress (void);
void test_quarantine (void);
void test_negotiate (void);

static MilterClientContext *context;
static MilterCommandEncoder *command_encoder;
static MilterReplyEncoder *reply_encoder;

static GIOChannel *channel;
static MilterWriter *writer;

static gchar *packet;
static gsize packet_size;

static gboolean send_progress;

static gchar *quarantine_reason;

static MilterMacrosRequests *macros_requests;
static MilterOption *option;

static MilterStatus
cb_negotiate (MilterClientContext *context, MilterOption *_option,
              MilterMacrosRequests *_macros_requests, gpointer user_data)
{
    if (option) {
        milter_option_set_version(_option, milter_option_get_version(option));
        milter_option_set_action(_option, milter_option_get_action(option));
        milter_option_set_step(_option, milter_option_get_step(option));
    }

    if (macros_requests)
        milter_macros_requests_merge(_macros_requests, macros_requests);

    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_connect (MilterClientContext *context, const gchar *host_name,
            const struct sockaddr *address, socklen_t address_size,
            gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_helo (MilterClientContext *context, const gchar *fqdn, gpointer user_data)
{
    if (send_progress) {
        milter_client_context_progress(context);
        milter_agent_set_writer(MILTER_AGENT(context), NULL);
    }

    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_envelope_from (MilterClientContext *context, const gchar *from,
                  gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_envelope_recipient (MilterClientContext *context, const gchar *to,
                       gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_data (MilterClientContext *context, gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_header (MilterClientContext *context, const gchar *name, const gchar *value,
           gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_end_of_header (MilterClientContext *context, gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_body (MilterClientContext *context, const gchar *chunk, gsize size,
         gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_end_of_message (MilterClientContext *context, gpointer user_data)
{
    if (quarantine_reason) {
        milter_client_context_quarantine(context, quarantine_reason);
        milter_agent_set_writer(MILTER_AGENT(context), NULL);
    }

    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_abort (MilterClientContext *context, gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static MilterStatus
cb_unknown (MilterClientContext *context, const gchar *command,
            gpointer user_data)
{
    return MILTER_STATUS_CONTINUE;
}

static void
setup_signals (MilterClientContext *context)
{
#define CONNECT(name)                                                   \
    g_signal_connect(context, #name, G_CALLBACK(cb_ ## name), NULL)

    CONNECT(negotiate);
    CONNECT(connect);
    CONNECT(helo);
    CONNECT(envelope_from);
    CONNECT(envelope_recipient);
    CONNECT(data);
    CONNECT(header);
    CONNECT(end_of_header);
    CONNECT(body);
    CONNECT(end_of_message);
    CONNECT(abort);
    CONNECT(unknown);

#undef CONNECT
}

void
setup (void)
{
    context = milter_client_context_new();

    channel = gcut_string_io_channel_new(NULL);
    g_io_channel_set_encoding(channel, NULL, NULL);
    writer = milter_writer_io_channel_new(channel);

    milter_agent_set_writer(MILTER_AGENT(context), writer);
    setup_signals(context);

    command_encoder = MILTER_COMMAND_ENCODER(milter_command_encoder_new());
    reply_encoder = MILTER_REPLY_ENCODER(milter_reply_encoder_new());
    packet = NULL;
    packet_size = 0;

    send_progress = FALSE;
    quarantine_reason = NULL;
    option = NULL;
    macros_requests = NULL;
}

static void
packet_free (void)
{
    if (packet)
        g_free(packet);
    packet = NULL;
    packet_size = 0;
}

void
teardown (void)
{
    if (context)
        g_object_unref(context);

    if (command_encoder)
        g_object_unref(command_encoder);
    if (reply_encoder)
        g_object_unref(reply_encoder);

    if (channel)
        g_io_channel_unref(channel);
    if (writer)
        g_object_unref(writer);

    if (quarantine_reason)
        g_free(quarantine_reason);

    if (option)
        g_object_unref(option);
    if (macros_requests)
        g_object_unref(macros_requests);

    packet_free();
}

typedef void (*HookFunction) (void);

static GError *
feed (void)
{
    GError *error = NULL;

    milter_client_context_feed(context, packet, packet_size, &error);

    return error;
}

void
test_progress (void)
{
    GString *actual_data;
    const gchar fqdn[] = "delian";

    send_progress = TRUE;
    milter_command_encoder_encode_helo(command_encoder,
                                       &packet, &packet_size,
                                       fqdn);
    gcut_assert_error(feed());

    packet_free();
    milter_reply_encoder_encode_progress(reply_encoder, &packet, &packet_size);
    actual_data = gcut_string_io_channel_get_string(channel);
    cut_assert_equal_memory(packet, packet_size,
                            actual_data->str, actual_data->len);
}

void
test_quarantine (void)
{
    GString *actual_data;

    quarantine_reason = g_strdup("virus mail!");
    milter_command_encoder_encode_end_of_message(command_encoder,
                                                 &packet, &packet_size,
                                                 NULL, 0);
    gcut_assert_error(feed());

    packet_free();
    milter_reply_encoder_encode_quarantine(reply_encoder, &packet, &packet_size,
                                           quarantine_reason);
    actual_data = gcut_string_io_channel_get_string(channel);
    cut_assert_equal_memory(packet, packet_size,
                            actual_data->str, actual_data->len);
}

void
test_negotiate (void)
{
    GString *actual_data;

    option = milter_option_new(2, MILTER_ACTION_ADD_HEADERS, MILTER_STEP_NONE);
    macros_requests = milter_macros_requests_new();
    milter_macros_requests_set_symbols(macros_requests,
                                       MILTER_COMMAND_HELO,
                                       "G", "N", "U", NULL);

    milter_command_encoder_encode_negotiate(command_encoder,
                                            &packet, &packet_size,
                                            option);
    gcut_assert_error(feed());

    packet_free();

    milter_reply_encoder_encode_negotiate(reply_encoder, &packet, &packet_size,
                                          option, macros_requests);

    actual_data = gcut_string_io_channel_get_string(channel);
    cut_assert_equal_memory(packet, packet_size,
                            actual_data->str, actual_data->len);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
