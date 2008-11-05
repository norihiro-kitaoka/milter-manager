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

#ifndef __MILTER_MANAGER_TEST_UTILS_H__
#define __MILTER_MANAGER_TEST_UTILS_H__

#include <gcutter.h>

G_BEGIN_DECLS

const gchar *milter_manager_test_get_base_dir (void);

typedef struct _MilterManagerTestHeader
{
    gchar *name;
    gchar *value;
} MilterManagerTestHeader;

MilterManagerTestHeader *milter_manager_test_header_new
                                                (const gchar *name,
                                                 const gchar *value);
void                     milter_manager_test_header_free
                                                (MilterManagerTestHeader *header);
gchar                   *milter_manager_test_header_inspect
                                                (MilterManagerTestHeader *header);


G_END_DECLS

#endif /* __MILTER_MANAGER_TEST_UTILS_H__ */

/*
vi:nowrap:ai:expandtab:sw=4
*/
