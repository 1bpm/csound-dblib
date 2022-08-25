/*
    postgresql.h
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

#ifndef CSSQLDB_POSTGRESQL_H
#define CSSQLDB_POSTGRESQL_H

#include <plugin.h>
#include "libpq-fe.h"
#include "connection.h"

struct PostgresConnection {
    PGconn* conn;
    void Init(csnd::Csound* csound, LoginData* login);
    void Close(csnd::Csound* csound);
    void Exec(char* sql);
    PGresult* Query(char *sql);
    MYFLT Scalar(char* sql, int row, int col);
    char* ScalarString(char* sql, csnd::Csound* csound, int row, int col);
    void ToArray(PGresult* result, csnd::Csound* csound, ARRAYDAT* array, bool asString);
    void ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array);
    void ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array);
};


#endif /* POSTGRESQL_H */

