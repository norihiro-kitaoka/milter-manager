/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2008-2009  Kouhei Sutou <kou@clear-code.com>
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
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __MILTER_CLIENT_H__
#define __MILTER_CLIENT_H__

#include <milter/core.h>
#include <milter/client/milter-client-context.h>
#include <milter/client/milter-client-enum-types.h>

G_BEGIN_DECLS

/**
 * SECTION: client
 * @title: Client library
 * @short_description: The client library to implement a milter.
 *
 * The milter-client library provides full milter protocol
 * processing features on client-side. "client-side" means
 * milter-side not MTA-side.
 *
 * %MilterClient and %MilterClientContext is the main
 * classes in the milter-client library. %MilterClient
 * accepts n-connections from MTA and each connection is
 * processed by %MilterClientContext.
 *
 * <rd>
 * == Usage overview
 *
 * You need to set connection spec and connect
 * #MilterClient::connection-established signal before
 * entering mail loop.
 *
 * Connection spec is a entry point of %MilterClient for
 * MTA.  It has 'PROTOCOL:INFORMATION' format. For IPv4
 * socket, 'inet:PORT', 'inet:PORT&commat;HOST' or
 * 'inet:PORT&commat;[ADDRESS]' are valid formats. For IPv6
 * socket, 'inet6:PORT', 'inet6:PORT&commat;HOST' or
 * 'inet6:PORT&commat;[ADDRESS]' are valid formats. For UNIX
 * domain socket, 'unix:PATH' is a valid format.
 *
 * %MilterClient emits #MilterClient::connection-established
 * signal. %MilterClientContext setup should be done in the
 * signal. In many cases, you just connect callbacks to
 * the passed %MilterClientContext. See %MilterClientContext
 * for available signals.
 *
 * milter_client_main() is mail loop function. You can enter
 * into mail loop after you finish to prepare your
 * %MilterClient.
 *
 * Here is an example codes that uses the milter-client
 * library. See also tool/milter-test-client.c. It is an
 * example milter implementation using the milter-client
 * library.
 *
 * |[
 * #include <stdlib.h>
 * #include <milter/client.h>
 *
 * static void
 * cb_connection_established (MilterClient *client,
 *                            MilterClientContext *context,
 *                            gpointer user_data)
 * {
 *    connect_to_your_interesting_signals(client, context, user_data);
 * }
 *
 * int
 * main (int argc, char **argv)
 * {
 *     gboolean success;
 *     const gchar spec[] = "inet:10025@localhost";
 *     MilterClient *client;
 *     GError *error = NULL;
 *
 *     milter_init();
 *     milter_client_init();
 *
 *     client = milter_client_new();
 *     if (!milter_client_set_connection_spec(client, spec, &error)) {
 *         g_print("%s\n", error->message);
 *         g_error_free(error);
 *         return EXIT_FAILURE;
 *     }
 *     g_signal_connect(client, "connection-established",
 *                      G_CALLBACK(cb_connection_established), NULL);
 *     milter_client_main(client);
 *
 *     milter_client_quit();
 *     milter_quit();
 *
 *     return success ? EXIT_SUCCESS : EXIT_FAILURE;
 * }
 * ]|
 *
 * == Processing model
 *
 * The libmilter provided by Sendmail uses a thread for each
 * connection model. But the milter-client library doesn't
 * use it. The milter-client library uses two threads. One
 * thread is for %MilterClient and other thread is for
 * %MilterClientContext<!-- -->s. %MilterClient just
 * accepting connections from MTA and dispatches it in the
 * first thread. Each connection is associated with
 * %MilterClientContext and they are processed in the second
 * thread. The milter-client library's model is the same
 * model of memcached. (memcached uses libevent as its event
 * loop backend but the milter-client library uses GLib's
 * %GMainLoop for it.)
 *
 * The libmilter's model will have more cost to accept a
 * connection rather than the milter-client library's
 * model. Because the libmilter's model creates a thread but
 * the milter-client library just allocate
 * %MilterClientContext. But in many case, this difference
 * is not bottleneck. :-|</rd>
 */

/**
 * milter_client_init:
 *
 * Call this function before using any other the milter-client
 * library functions.
 */
void milter_client_init (void);

/**
 * milter_client_quit:
 *
 * Call this function after the milter-client library use.
 */
void milter_client_quit (void);

/**
 * MILTER_CLIENT_ERROR:
 *
 * Used to get the #GError quark for #MilterClient errors.
 */
#define MILTER_CLIENT_ERROR           (milter_client_error_quark())

#define MILTER_TYPE_CLIENT            (milter_client_get_type())
#define MILTER_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), MILTER_TYPE_CLIENT, MilterClient))
#define MILTER_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), MILTER_TYPE_CLIENT, MilterClientClass))
#define MILTER_IS_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), MILTER_TYPE_CLIENT))
#define MILTER_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MILTER_TYPE_CLIENT))
#define MILTER_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), MILTER_TYPE_CLIENT, MilterClientClass))

/**
 * MilterClientError:
 * @MILTER_CLIENT_ERROR_RUNNING: Indicates main loop has
 * been running.
 * @MILTER_CLIENT_ERROR_UNIX_SOCKET: Indicates UNIX domain
 * socket related error.
 * @MILTER_CLIENT_ERROR_IO_ERROR: Indicates IO related
 * error.
 *
 * These identify the variable errors that can occur while
 * calling %MilterClient functions.
 */
typedef enum
{
    MILTER_CLIENT_ERROR_RUNNING,
    MILTER_CLIENT_ERROR_UNIX_SOCKET,
    MILTER_CLIENT_ERROR_IO_ERROR
} MilterClientError;

typedef struct _MilterClient         MilterClient;
typedef struct _MilterClientClass    MilterClientClass;

/**
 * MilterClient:
 *
 * %MilterClient is a front object. It accepts connections
 * from MTA and dispatches it.
 **/
struct _MilterClient
{
    GObject object;
};

struct _MilterClientClass
{
    GObjectClass parent_class;

    void   (*connection_established)      (MilterClient *client,
                                           MilterClientContext *context);
    void   (*listen_started)              (MilterClient *client,
                                           struct sockaddr *address,
                                           socklen_t        address_size);
    gchar *(*get_default_connection_spec) (MilterClient *client);
    guint  (*get_unix_socket_mode)        (MilterClient *client);
    const gchar *(*get_unix_socket_group) (MilterClient *client);
    gboolean (*is_remove_unix_socket_on_close)
                                          (MilterClient *client);
};

GQuark               milter_client_error_quark       (void);

GType                milter_client_get_type          (void) G_GNUC_CONST;


/**
 * milter_client_new:
 *
 * Creates a new client.
 *
 * Returns: a new %MilterClient object.
 */
MilterClient        *milter_client_new               (void);

/**
 * milter_client_get_default_connection_spec:
 * @client: a %MilterClient.
 *
 * Gets the default connection specification. The default
 * connection specification should be freed when no longer
 * needed.
 *
 * Returns: the copy of default connection specification.
 */
gchar               *milter_client_get_default_connection_spec
                                                     (MilterClient  *client);

/**
 * milter_client_set_connection_spec:
 * @client: a %MilterClient.
 * @spec: a connection specification like inet:10025.
 * @error: return location for an error, or %NULL.
 *
 * Sets a connection specification. If @spec is invalid
 * format and @error is not %NULL, error detail is stored
 * into @error.
 *
 * Returns: %TRUE on success.
 */
gboolean             milter_client_set_connection_spec
                                                     (MilterClient  *client,
                                                      const gchar   *spec,
                                                      GError       **error);

/**
 * milter_client_get_listen_channel:
 * @client: a %MilterClient.
 *
 * Gets the channel for listening connection.
 *
 * Returns: the listen channel.
 */
GIOChannel          *milter_client_get_listen_channel(MilterClient  *client);

/**
 * milter_client_set_listen_channel:
 * @client: a %MilterClient.
 * @channel: a channel for listening connection.
 *
 * Sets a channel for listening connection.
 */
void                 milter_client_set_listen_channel(MilterClient  *client,
                                                      GIOChannel    *channel);

/**
 * milter_client_set_listen_backlog:
 * @client: a %MilterClient.
 * @backlog: a backlog for listen(2).
 *
 * Sets a backlog for listen(2).
 */
void                 milter_client_set_listen_backlog(MilterClient  *client,
                                                      gint           backlog);

/**
 * milter_client_is_remove_unix_socket_on_create:
 * @client: a %MilterClient.
 *
 * Gets whether removing UNIX domain socket before new UNIX
 * domain socket is created.
 *
 * Returns: %TRUE if removing UNIX domain socket on create,
 * %FALSE otherwise.
 */
gboolean             milter_client_is_remove_unix_socket_on_create
                                                     (MilterClient  *client);

/**
 * milter_client_set_remove_unix_socket_on_create:
 * @client: a %MilterClient.
 * @remove: %TRUE if removing UNIX domain socket before new
 *          UNIX domain socket is create.
 *
 * Sets whether removing UNIX domain socket before new UNIX
 * domain socket is created.
 */
void                 milter_client_set_remove_unix_socket_on_create
                                                     (MilterClient  *client,
                                                      gboolean       remove);

/**
 * milter_client_set_timeout:
 * @client: a %MilterClient.
 * @timeout: a timeout by seconds.
 *
 * Sets the timeout value for established
 * %MilterClientContext.
 */
void                 milter_client_set_timeout       (MilterClient  *client,
                                                      guint          timeout);

/**
 * milter_client_get_unix_socket_mode:
 * @client: a %MilterClient.
 *
 * Gets the UNIX domain socket mode.
 *
 * Returns: the UNIX domain socket mode.
 */
guint                milter_client_get_unix_socket_mode
                                                     (MilterClient  *client);

/**
 * milter_client_get_default_unix_socket_mode:
 * @client: a %MilterClient.
 *
 * Gets the default UNIX domain socket mode.
 *
 * Returns: the default UNIX domain socket mode.
 */
guint                milter_client_get_default_unix_socket_mode
                                                     (MilterClient  *client);

/**
 * milter_client_set_default_unix_socket_mode:
 * @client: a %MilterClient.
 * @mode: the UNIX domain socket mode.
 *
 * Sets the default UNIX domain socket mode.
 */
void                 milter_client_set_default_unix_socket_mode
                                                     (MilterClient  *client,
                                                      guint          mode);

/**
 * milter_client_get_unix_socket_group:
 * @client: a %MilterClient.
 *
 * Gets the group of UNIX domain socket.
 *
 * Returns: the group name for UNIX domain socket.
 */
const gchar         *milter_client_get_unix_socket_group
                                                     (MilterClient  *client);

/**
 * milter_client_get_default_unix_socket_group:
 * @client: a %MilterClient.
 *
 * Gets the default group of UNIX domain socket.
 *
 * Returns: the default group name for UNIX domain socket.
 */
const gchar         *milter_client_get_default_unix_socket_group
                                                     (MilterClient  *client);

/**
 * milter_client_set_default_unix_socket_group:
 * @client: a %MilterClient.
 * @group: the group name.
 *
 * Sets the default group of UNIX domain socket.
 */
void                 milter_client_set_default_unix_socket_group
                                                     (MilterClient  *client,
                                                      const gchar   *group);

/**
 * milter_client_is_remove_unix_socket_on_close:
 * @client: a %MilterClient.
 *
 * Gets whether removing UNIX domain socket after the socket
 * is closed.
 *
 * Returns: %TRUE if removing UNIX domain socket on close,
 * %FALSE otherwise.
 */
gboolean             milter_client_is_remove_unix_socket_on_close
                                                     (MilterClient  *client);

/**
 * milter_client_get_default_remove_unix_socket_on_close:
 * @client: a %MilterClient.
 *
 * Gets default value whether removing UNIX domain socket
 * after the socket is closed.
 *
 * Returns: %TRUE if removing UNIX domain socket on close,
 * %FALSE otherwise.
 */
gboolean             milter_client_get_default_remove_unix_socket_on_close
                                                     (MilterClient  *client);

/**
 * milter_client_set_default_remove_unix_socket_on_close:
 * @client: a %MilterClient.
 * @remove: %TRUE if removing UNIX domain socket after the
 *          socket is closed.
 *
 * Gets default value whether removing UNIX domain socket
 * after the socket is closed.
 */
void                 milter_client_set_default_remove_unix_socket_on_close
                                                     (MilterClient  *client,
                                                      gboolean       remove);

/**
 * milter_client_main:
 * @client: a %MilterClient.
 *
 * Starts main loop.
 *
 * Returns: %TRUE if main loop is quitted successfully,
 * %FALSE otherwise.
 */
gboolean             milter_client_main              (MilterClient  *client);

/**
 * milter_client_shutdown:
 * @client: a %MilterClient.
 *
 * Shuts main loop down.
 */
void                 milter_client_shutdown          (MilterClient  *client);


G_END_DECLS

#endif /* __MILTER_CLIENT_H__ */

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
