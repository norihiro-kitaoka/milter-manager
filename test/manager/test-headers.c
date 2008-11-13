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

#include <string.h>

#include <gcutter.h>

#define shutdown inet_shutdown
#include <milter-manager-test-utils.h>
#include <milter/manager/milter-manager-headers.h>
#undef shutdown

void test_add_header (void);
void test_insert_header (void);
void test_change_header (void);

static MilterManagerHeaders *headers;
static GList *expected_list;

void
setup (void)
{
    headers = milter_manager_headers_new();
    expected_list = NULL;
}

void
teardown (void)
{
    if (headers)
        g_object_unref(headers);
    if (expected_list) {
        g_list_foreach(expected_list, (GFunc)milter_manager_header_free, NULL);
        g_list_free(expected_list);
    }
}

void
test_add_header (void)
{
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("First header",
                                                            "First header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Second header",
                                                            "Second header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Third header",
                                                            "Third header value"));

    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "First header",
                                                      "First header value"));
    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Second header",
                                                      "Second header value"));
    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Third header",
                                                      "Third header value"));
    gcut_assert_equal_list(
            expected_list,
            milter_manager_headers_get_list(headers),
            milter_manager_header_equal,
            (GCutInspectFunc)milter_manager_header_inspect,
            NULL);
}

void
test_insert_header (void)
{
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("First header",
                                                            "First header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Second header",
                                                            "Second header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Third header",
                                                            "Third header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Forth header",
                                                            "Forth header value"));

    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "First header",
                                                      "First header value"));
    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Third header",
                                                      "Third header value"));
    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Forth header",
                                                      "Forth header value"));
    cut_assert_true(milter_manager_headers_insert_header(headers,
                                                         1,
                                                         "Second header",
                                                         "Second header value"));
    gcut_assert_equal_list(
            expected_list,
            milter_manager_headers_get_list(headers),
            milter_manager_header_equal,
            (GCutInspectFunc)milter_manager_header_inspect,
            NULL);
}

void
test_change_header (void)
{
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Test header",
                                                            "Test header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Test header",
                                                            "Test header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Unique header",
                                                            "Unique header value"));
    expected_list = g_list_append(expected_list, 
                                  milter_manager_header_new("Test header",
                                                            "Replaced header value"));

    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Test header",
                                                      "Test header value"));
    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Test header",
                                                      "Test header value"));
    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Unique header",
                                                      "Unique header value"));
    cut_assert_true(milter_manager_headers_add_header(headers,
                                                      "Test header",
                                                      "Test header value"));
    cut_assert_true(milter_manager_headers_change_header(headers,
                                                         "Test header",
                                                         3,
                                                         "Replaced header value"));
    gcut_assert_equal_list(
            expected_list,
            milter_manager_headers_get_list(headers),
            milter_manager_header_equal,
            (GCutInspectFunc)milter_manager_header_inspect,
            NULL);
}


/*
vi:ts=4:nowrap:ai:expandtab:sw=4
*/
