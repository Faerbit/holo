/*******************************************************************************
*
* Copyright 2016 Stefan Majewsky <majewsky@gmx.net>
*
* This file is part of Holo.
*
* Holo is free software: you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* Holo is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
* A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* Holo. If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
// WARNING: This file contains the unit tests for the holo binary. It is not  //
//       compiled into the installed executable.                              //
////////////////////////////////////////////////////////////////////////////////

#include "holo.h"
#include <check.h>
#include <stdlib.h>

START_TEST(test_pathClean) {
    char* s = NULL;

    //shouldn't change clean paths
    ck_assert_str_eq(s = pathClean("/"),        "/");        free(s);
    ck_assert_str_eq(s = pathClean("/foo"),     "/foo");     free(s);
    ck_assert_str_eq(s = pathClean("/foo/bar"), "/foo/bar"); free(s);

    //should remove trailing slashes
    ck_assert_str_eq(s = pathClean("/foo/"),     "/foo");     free(s);
    ck_assert_str_eq(s = pathClean("/foo/bar/"), "/foo/bar"); free(s);

    //should remove duplicate slashes
    ck_assert_str_eq(s = pathClean("////"),        "/");        free(s);
    ck_assert_str_eq(s = pathClean("//foo///bar"), "/foo/bar"); free(s);

} END_TEST

START_TEST(test_pathJoin) {
    char* s = NULL;

    //test appending of relative to absolute path
    ck_assert_str_eq(s = pathJoin("/",        "foo"),     "/foo");             free(s);
    ck_assert_str_eq(s = pathJoin("/qux/qux", "foo"),     "/qux/qux/foo");     free(s);
    ck_assert_str_eq(s = pathJoin("/",        "foo/bar"), "/foo/bar");         free(s);
    ck_assert_str_eq(s = pathJoin("/qux/qux", "foo/bar"), "/qux/qux/foo/bar"); free(s);

    //test appending of relative to relative path
    ck_assert_str_eq(s = pathJoin(".",       "foo"),     "./foo");           free(s);
    ck_assert_str_eq(s = pathJoin("qux",     "foo"),     "qux/foo");         free(s);
    ck_assert_str_eq(s = pathJoin("qux/qux", "foo"),     "qux/qux/foo");     free(s);
    ck_assert_str_eq(s = pathJoin(".",       "foo/bar"), "./foo/bar");       free(s);
    ck_assert_str_eq(s = pathJoin("qux",     "foo/bar"), "qux/foo/bar");     free(s);
    ck_assert_str_eq(s = pathJoin("qux/qux", "foo/bar"), "qux/qux/foo/bar"); free(s);

    //test appending of absolute to absolute path (always returns the second path)
    ck_assert_str_eq(s = pathJoin("/",        "/foo"),     "/foo");     free(s);
    ck_assert_str_eq(s = pathJoin("/qux/qux", "/foo"),     "/foo");     free(s);
    ck_assert_str_eq(s = pathJoin("/",        "/foo/bar"), "/foo/bar"); free(s);
    ck_assert_str_eq(s = pathJoin("/qux/qux", "/foo/bar"), "/foo/bar"); free(s);

} END_TEST

START_TEST(test_stringJoin) {
    char* s = NULL;

    ck_assert_str_eq(s = stringJoin(NULL,  NULL), "");        free(s);
    ck_assert_str_eq(s = stringJoin("",    NULL), "");        free(s);
    ck_assert_str_eq(s = stringJoin("abc", NULL), "abc");     free(s);

    ck_assert_str_eq(s = stringJoin(NULL,  ""),   "");        free(s);
    ck_assert_str_eq(s = stringJoin("",    ""),   "");        free(s);
    ck_assert_str_eq(s = stringJoin("abc", ""),   "abc");     free(s);

    ck_assert_str_eq(s = stringJoin(NULL,  "def"), "def");    free(s);
    ck_assert_str_eq(s = stringJoin("",    "def"), "def");    free(s);
    ck_assert_str_eq(s = stringJoin("abc", "def"), "abcdef"); free(s);

} END_TEST

Suite* makeTestSuite() {
    TCase* tcPaths = tcase_create("path functions");
    tcase_add_test(tcPaths, test_pathClean);
    tcase_add_test(tcPaths, test_pathJoin);
    tcase_add_test(tcPaths, test_stringJoin);

    Suite* s = suite_create("holo");
    suite_add_tcase(s, tcPaths);
    return s;
}

int main() {
    SRunner* sr = srunner_create(makeTestSuite());
    srunner_run_all(sr, CK_NORMAL);
    return srunner_ntests_failed(sr) == 0 ? 0 : 1;
}