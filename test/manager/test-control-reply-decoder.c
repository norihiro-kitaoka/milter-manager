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

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <milter/manager/milter-manager-control-reply-decoder.h>

#include <gcutter.h>

void test_decode_success (void);
void test_decode_failure (void);
void test_decode_error (void);

static MilterDecoder *decoder;
static GString *buffer;

static GError *expected_error;
static GError *actual_error;

static gint n_success_received;
static gint n_failure_received;
static gint n_error_received;

static gchar *failure_message;
static gchar *error_message;

static void
cb_success (MilterManagerControlReplyDecoder *decoder, gpointer user_data)
{
    n_success_received++;
}

static void
cb_failure (MilterManagerControlReplyDecoder *decoder,
            const gchar *message, gpointer user_data)
{
    n_failure_received++;

    if (failure_message)
        g_free(failure_message);
    failure_message = g_strdup(message);
}

static void
cb_error (MilterManagerControlReplyDecoder *decoder,
          const gchar *message, gpointer user_data)
{
    n_error_received++;

    if (error_message)
        g_free(error_message);
    error_message = g_strdup(message);
}

static void
setup_signals (MilterDecoder *decoder)
{
#define CONNECT(name)                                                   \
    g_signal_connect(decoder, #name, G_CALLBACK(cb_ ## name), NULL)

    CONNECT(success);
    CONNECT(failure);
    CONNECT(error);

#undef CONNECT
}

void
setup (void)
{
    decoder = milter_manager_control_reply_decoder_new();
    setup_signals(decoder);

    expected_error = NULL;
    actual_error = NULL;

    n_success_received = 0;
    n_failure_received = 0;
    n_error_received = 0;

    buffer = g_string_new(NULL);

    failure_message = NULL;
    error_message = NULL;
}

void
teardown (void)
{
    if (decoder)
        g_object_unref(decoder);

    if (buffer)
        g_string_free(buffer, TRUE);

    if (expected_error)
        g_error_free(expected_error);
    if (actual_error)
        g_error_free(actual_error);

    if (failure_message)
        g_free(failure_message);
    if (error_message)
        g_free(error_message);
}

static GError *
decode (void)
{
    guint32 content_size;
    gchar content_string[sizeof(guint32)];
    GError *error = NULL;

    content_size = g_htonl(buffer->len);
    memcpy(content_string, &content_size, sizeof(content_size));
    g_string_prepend_len(buffer, content_string, sizeof(content_size));

    milter_decoder_decode(decoder, buffer->str, buffer->len, &error);
    g_string_truncate(buffer, 0);

    return error;
}

void
test_decode_success (void)
{
    g_string_append_c(buffer, 's');

    gcut_assert_error(decode());
    cut_assert_equal_int(1, n_success_received);
}

void
test_decode_failure (void)
{
    const gchar message[] = "Failure!";

    g_string_append_c(buffer, 'f');
    g_string_append(buffer, message);

    gcut_assert_error(decode());
    cut_assert_equal_int(1, n_failure_received);
    cut_assert_equal_string(message, failure_message);
}

void
test_decode_error (void)
{
    const gchar message[] = "Error!";

    g_string_append_c(buffer, 'e');
    g_string_append(buffer, message);

    gcut_assert_error(decode());
    cut_assert_equal_int(1, n_error_received);
    cut_assert_equal_string(message, error_message);
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
