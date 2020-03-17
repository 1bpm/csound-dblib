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
#include <iostream>
#include <pqxx/pqxx> 
#include "connection.h"
#include "postgresql.h"



void PostgresConnection::Init(csnd::Csound* csound, LoginData* login) {
    //conn = (pqxx::connection*) csound->malloc(sizeof(pqxx::connection));
    char connectionString[256];
    
    snprintf(connectionString, 256,
            "dbname=%s user=%s password=%s hostaddr=%s",
            login->dbName, login->dbUser, login->dbPass, login->dbHost
            );
  
    conn = new pqxx::connection(connectionString);
    
    // ignore notices
    std::auto_ptr<pqxx::noticer> np(new(pqxx::nonnoticer));
    conn->set_noticer(np);

    if (!conn->is_open()) {
        throw std::runtime_error("Connection not open");
    }
}

void PostgresConnection::Close(csnd::Csound* csound) {
    if (conn->is_open()) {
        conn->disconnect();
    }
    delete conn;
}

void PostgresConnection::Exec(char* sql) {
    pqxx::nontransaction nt(*conn);
    nt.exec(sql);
}

pqxx::result PostgresConnection::Query(char* sql) {
    pqxx::nontransaction nt(*conn);
    pqxx::result result(nt.exec(sql));
    return result;
}

MYFLT PostgresConnection::Scalar(char* sql, int row=0, int col=0) {
    pqxx::result result = Query(sql);

    // checks as libpqxx not throwing if this happens
    if (row > result.size() - 1) {
        throw std::runtime_error("row number out of range");
    }
    if (col > result[row].size() -1) {
        throw std::runtime_error("column number out of range");
    }

    return result[row][col].as<MYFLT>();
}

char* PostgresConnection::ScalarString(char* sql, int row=0, int col=0) {
    pqxx::result result = Query(sql);

    // checks as libpqxx not throwing if this happens
    if (row > result.size() - 1) {
        throw std::runtime_error("row number out of range");
    }
    if (col > result[row].size() -1) {
        throw std::runtime_error("column number out of range");
    }

    return result[row][col].c_str();

}


void PostgresConnection::ToArray(pqxx::result result, csnd::Csound* csound, ARRAYDAT* array, bool asString) {
    int totalResults = result.size() * result[0].size();
    array->sizes = csound->calloc(sizeof(int32_t) * 2);
    array->sizes[0] = result.size();
    array->sizes[1] = result[0].size();
    array->dimensions = 2;
    CS_VARIABLE *var = array->arrayType->createVariable(csound, NULL);
    array->arrayMemberSize = var->memBlockSize;
    array->data = csound->calloc(var->memBlockSize * totalResults);
    STRINGDAT* strings;
    if (asString) {
        strings = (STRINGDAT*) array->data;
    }

    int index = 0;
    for (int rowNum = 0; rowNum < result.size(); ++rowNum) {
        const pqxx::result::tuple row = result[rowNum];
        for (int colNum = 0; colNum < row.size(); ++colNum) {
            const pqxx::result::field field = row[colNum];
            if (asString) {
                char* item = field.c_str();
                strings[index].size = strlen(item) + 1;
                strings[index].data = csound->strdup(item);
            } else {
                array->data[index] = field.as<MYFLT>();
            }
            index++;
        }
    }
}

void PostgresConnection::ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, true);
}

void PostgresConnection::ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, false);
}
