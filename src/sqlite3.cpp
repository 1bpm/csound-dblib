/*
    sqlite3.cpp
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
#include <iostream>
#include <sqlite3.h>
#include "connection.h"
#include "sqlite.h"


void SqliteConnection::Init(csnd::Csound* csound, LoginData* login) {
    int result = sqlite3_open(login->dbName, &conn);
    std::cout << "Y";
    if (result) {
        throw std::runtime_error("connection not established");
    }
}

void SqliteConnection::Close(csnd::Csound* csound) {
    sqlite3_close(conn);
}

void SqliteConnection::Exec(char* sql) {
    sqlite3_stmt* stmt = Query(sql);
    int rc = sqlite3_step(stmt);
    rc = sqlite3_finalize(stmt);
}

sqlite3_stmt* SqliteConnection::Query(char* sql) {
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(conn));
    }
    return stmt;
}

MYFLT SqliteConnection::Scalar(char* sql, int row=0, int col=0) {
    sqlite3_stmt *stmt = Query(sql);
    int colCount = sqlite3_column_count(stmt);
    int rc = sqlite3_step(stmt);
    int rowIndex = 0;
    while (rc != SQLITE_DONE && rc != SQLITE_OK) {
        if (rowIndex == row) {

            if (col > colCount -1) {
                rc = sqlite3_finalize(stmt);
                throw std::runtime_error("column number out of range");
            }
            MYFLT result = (MYFLT) sqlite3_column_double(stmt, col);
            rc = sqlite3_finalize(stmt);
            return result;
        }
        rc = sqlite3_step(stmt);
        rowIndex++;
    }
    rc = sqlite3_finalize(stmt);
    throw std::runtime_error("no result");  
}

char* SqliteConnection::ScalarString(char* sql, int row=0, int col=0) {
    sqlite3_stmt *stmt = Query(sql);
    int colCount = sqlite3_column_count(stmt);
    int rc = sqlite3_step(stmt);
    int rowIndex = 0;
    while (rc != SQLITE_DONE && rc != SQLITE_OK) {
        if (rowIndex == row) {

            if (col > colCount -1) {
                rc = sqlite3_finalize(stmt);
                throw std::runtime_error("column number out of range");
            }
            char* result = sqlite3_column_text(stmt, col);
            rc = sqlite3_finalize(stmt);
            return result;
        }
        rc = sqlite3_step(stmt);
        rowIndex++;
    }
    rc = sqlite3_finalize(stmt);
    throw std::runtime_error("no result");   

}

int SqliteConnection::RowCount(sqlite3_stmt* stmt) {
    int rowCount = 0;
    int rc = sqlite3_step(stmt);
    while (rc != SQLITE_DONE && rc != SQLITE_OK) {
        rc = sqlite3_step(stmt);
        rowCount ++;
    }
    rc = sqlite3_reset(stmt);
    return rowCount;
}

void SqliteConnection::ToArray(sqlite3_stmt* stmt, csnd::Csound* csound, ARRAYDAT* array, bool asString) {
    int colNum = sqlite3_column_count(stmt);
    int rowNum = RowCount(stmt);
    int totalResults = colNum * rowNum;
    array->sizes = csound->calloc(sizeof(int32_t) * 2);
    array->sizes[0] = rowNum;
    array->sizes[1] = colNum;
    array->dimensions = 2;
    CS_VARIABLE *var = array->arrayType->createVariable(csound, NULL);
    array->arrayMemberSize = var->memBlockSize;
    array->data = csound->calloc(var->memBlockSize * totalResults);
    STRINGDAT* strings;
    if (asString) {
        strings = (STRINGDAT*) array->data;
    }

    int colIndex;
    int rowIndex;
    int index = 0;
    int rc = sqlite3_step(stmt);
    while (rc != SQLITE_DONE && rc != SQLITE_OK) {
        colIndex = 0;
        while (colIndex < colNum) {
            if (asString) {
                char* item = sqlite3_column_text(stmt, colIndex);
                strings[index].size = strlen(item) + 1;
                strings[index].data = csound->strdup(item);
            } else {
                array->data[index] = (MYFLT) sqlite3_column_double(stmt, colIndex);
            }
            colIndex ++;
            index++;
        }            
        rc = sqlite3_step(stmt);
        rowIndex++;
    }
    rc = sqlite3_finalize(stmt);
}

void SqliteConnection::ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, true);
}

void SqliteConnection::ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, false);
}
