/*
    connection.h
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

#ifndef CSSQLDB_CONNECTION_H
#define CSSQLDB_CONNECTION_H
#include <plugin.h>
#include "logindata.h"

#define EXEC 0
#define SCALAR 1
#define SCALARSTRING 2
#define ARRAY 3
#define ARRAYSTRING 4

#define POSTGRES 0
#define SQLITE 1
#define MYSQL 2

#ifdef BUILD_SQLITE
#include "sqlite.h"
#endif

#ifdef BUILD_POSTGRES
#include "postgresql.h"
#endif

#ifdef BUILD_MYSQL
#include "mysql.h"
#endif


// tried to do this with templates and inheritance, failed with various approaches 



struct ConnectionData {
    void* mutex;
#ifdef BUILD_SQLITE
    SqliteConnection* sqlite;
#endif
#ifdef BUILD_POSTGRES
    PostgresConnection* postgres;
#endif
#ifdef BUILD_MYSQL
    MySQLConnection* mysql;
#endif
    int type;
    bool open;
    void Init(csnd::Csound* csound, LoginData* login);
    void Close(csnd::Csound* csound);
    void Exec(char* sql);
    MYFLT Scalar(char* sql, int row, int col);
    char* ScalarString(char* sql, csnd::Csound* csound, int row=0, int col=0);
    void ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array);
    void ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array);
};

struct QueryData {
    ConnectionData* connection;
    char* sql;
    int queryType;
    int row;
    int col;
    ARRAYDAT* array;
};



ConnectionData* getConnection(csnd::Csound* csound, MYFLT handle);

#endif /* CONNECTION_H */

