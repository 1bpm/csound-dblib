/*
    postgresql.cpp
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

#include <plugin.h>
#include "tools.h"
#include <iostream>
#include "libpq-fe.h"
#include "connection.h"
#include "postgresql.h"


void PostgresConnection::Init(csnd::Csound* csound, LoginData* login) {
    char connectionString[256];
    
    snprintf(connectionString, 256,
            "dbname=%s user=%s password=%s hostaddr=%s",
            login->dbName, login->dbUser, login->dbPass, login->dbHost
            );
  
    conn = PQconnectdb(connectionString);

    if (PQstatus(conn) == CONNECTION_BAD) {
        throw std::runtime_error("Connection not open");
    }
}

void PostgresConnection::Close(csnd::Csound* csound) {
	PQfinish(conn);
}

void PostgresConnection::Exec(char* sql) {
	PGresult* result = PQexec(conn, sql);
	PQclear(result);
}

PGresult* PostgresConnection::Query(char* sql) {
    return PQexec(conn, sql);
}

MYFLT PostgresConnection::Scalar(char* sql, int row, int col) {
    PGresult* result = Query(sql);

	int rows = PQntuples(result);
	int cols = PQnfields(result);
    
    if (row > rows - 1) {
        PQclear(result);
        throw std::runtime_error("row number out of range");
    }
    if (col > cols - 1) {
        PQclear(result);
        throw std::runtime_error("column number out of range");
    }

    MYFLT value = (MYFLT) atof(PQgetvalue(result, row, col));
	PQclear(result);
	return value;
}

char* PostgresConnection::ScalarString(char* sql, csnd::Csound* csound, int row, int col) {
    PGresult* result = Query(sql);

	int rows = PQntuples(result);
	int cols = PQnfields(result);

    if (row > rows - 1) {
        PQclear(result);
        throw std::runtime_error("row number out of range");
    }
    if (col > cols -1) {
        PQclear(result);
        throw std::runtime_error("column number out of range");
    }

	char* value = csound->strdup(PQgetvalue(result, row, col));
	PQclear(result);
	return value;

}


void PostgresConnection::ToArray(PGresult* result, csnd::Csound* csound, ARRAYDAT* array, bool asString) {
	int rows = PQntuples(result);
	int cols = PQnfields(result);
    STRINGDAT* strings = arrayInit(csound, array, rows, cols);

    int index = 0;
    for (int rowIndex = 0; rowIndex < rows; ++rowIndex) {
        for (int colIndex = 0; colIndex < cols; ++colIndex) {
            if (asString) {
                insertArrayStringItem(csound, strings, index, (char*) PQgetvalue(result, rowIndex, colIndex));
            } else {
                array->data[index] = (MYFLT) atof(PQgetvalue(result, rowIndex, colIndex));
            }
            index++;
        }
    }
	PQclear(result);
}

void PostgresConnection::ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, true);
}

void PostgresConnection::ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, false);
}
