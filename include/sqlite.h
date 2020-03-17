/*
    sqlite.h
    Copyright (C) 2019 Richard Knight


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
 */

#ifndef XSQLITE3_H
#define XSQLITE3_H

#include <plugin.h>
#include <sqlite3.h>
#include "connection.h"

struct SqliteConnection {
    sqlite3* conn;
    void Init(csnd::Csound* csound, LoginData* login);
    void Close(csnd::Csound* csound);
    void Exec(char* sql);
    sqlite3_stmt* Query(char *sql);
    MYFLT Scalar(char* sql, int row, int col);
    char* ScalarString(char* sql, int row, int col);
    int RowCount(sqlite3_stmt* stmt);
    void ToArray(sqlite3_stmt* result, csnd::Csound* csound, ARRAYDAT* array, bool asString);
    void ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array);
    void ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array);
};

#endif /* SQLITE3_H */

