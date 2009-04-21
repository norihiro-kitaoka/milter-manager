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

#ifdef HAVE_CONFIG_H
#  include "../../config.h"
#endif /* HAVE_CONFIG_H */

#include <milter/core.h>

static gboolean initialized = FALSE;

static guint glib_log_handler_id = 0;
static guint gobject_log_handler_id = 0;
static guint gthread_log_handler_id = 0;
static guint gmodule_log_handler_id = 0;
static guint milter_core_log_handler_id = 0;

void
milter_init (void)
{
    if (initialized)
        return;

    initialized = TRUE;

    g_type_init();

    if (!g_thread_supported())
        g_thread_init(NULL);

    glib_log_handler_id = MILTER_GLIB_LOG_DELEGATE("GLib");
    gobject_log_handler_id = MILTER_GLIB_LOG_DELEGATE("GLib-GObject");
    gthread_log_handler_id = MILTER_GLIB_LOG_DELEGATE("GThread");
    gmodule_log_handler_id = MILTER_GLIB_LOG_DELEGATE("GModule");
    milter_core_log_handler_id = MILTER_GLIB_LOG_DELEGATE("milter-core");
}

void
milter_quit (void)
{
    if (!initialized)
        return;

    g_log_remove_handler("GLib", glib_log_handler_id);
    g_log_remove_handler("GLib-GObject", gobject_log_handler_id);
    g_log_remove_handler("GThread", gthread_log_handler_id);
    g_log_remove_handler("GModule", gmodule_log_handler_id);
    g_log_remove_handler("milter-core", milter_core_log_handler_id);

    initialized = FALSE;
}

/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
