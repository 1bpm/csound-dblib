/*
    mysql.cpp
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
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "mysql_connection.h"
#include "connection.h"
#include "mysql.h"
namespace mysql = sql;


void MySQLConnection::Init(csnd::Csound* csound, LoginData* login) {
    driver = get_driver_instance();
    char host[256];
    snprintf(host, 256, "tcp://%s:3306", login->dbHost);
    
    conn = driver->connect(host, login->dbUser, login->dbPass);
    conn->setSchema(login->dbName);
    
    if (conn->isClosed()) {
        throw std::runtime_error("connection not established");
    }
}

void MySQLConnection::Close(csnd::Csound* csound) {
    conn->close();
    delete conn;
}

void MySQLConnection::Exec(char* sql) {
    mysql::Statement* stmt = conn->createStatement();
    stmt->execute(sql);
    delete stmt;
}

mysql::ResultSet* MySQLConnection::Query(char* sql) {
    mysql::Statement* stmt = conn->createStatement();
    mysql::ResultSet* result = stmt->executeQuery(sql);
    delete stmt;
    return result;
}

MYFLT MySQLConnection::Scalar(char* sql, int row, int col) {
    mysql::ResultSet* res = Query(sql);
    mysql::ResultSetMetaData* meta = res->getMetaData();
    int colCount = meta->getColumnCount();
    if (col > colCount - 1) {
        throw std::runtime_error("column number out of range");
    }

    res->next();
    MYFLT result = (MYFLT) res->getDouble(col + 1);
    
    delete res;
    
    return result;
}

char* MySQLConnection::ScalarString(char* sql, int row, int col) {
    mysql::ResultSet* res = Query(sql);
    mysql::ResultSetMetaData* meta = res->getMetaData();
    
    int colCount = meta->getColumnCount();
    if (col > colCount - 1) {
        throw std::runtime_error("column number out of range");
    }
    
    int rowIndex = 0;
    for (int rowIndex = 0; rowIndex <= row; rowIndex++) {
        res->next();
    }
    char* result = (char*) res->getString(col + 1).c_str();
    
    delete res;
    
    return result;  
}


void MySQLConnection::ToArray(mysql::ResultSet* result, csnd::Csound* csound, ARRAYDAT* array, bool asString) {
    mysql::ResultSetMetaData* meta = result->getMetaData();
    int colNum = meta->getColumnCount();
    int rowNum = result->rowsCount();
    int totalResults = colNum * rowNum;
    array->sizes = (int32_t*) csound->calloc(sizeof(int32_t) * 2);
    array->sizes[0] = rowNum;
    array->sizes[1] = colNum;
    array->dimensions = 2;
    CS_VARIABLE *var = array->arrayType->createVariable(csound, NULL);
    array->arrayMemberSize = var->memBlockSize;
    array->data = (MYFLT*) csound->calloc(var->memBlockSize * totalResults);
    STRINGDAT* strings;
    if (asString) {
        strings = (STRINGDAT*) array->data;
    }

    int colIndex;
    int index = 0;
    
    while (result->next()) {
        colIndex = 0;
        while (colIndex < colNum) {
            if (asString) {
                char* item = (char*) result->getString(colIndex + 1).c_str();
                strings[index].size = strlen(item) + 1;
                strings[index].data = csound->strdup(item);
            } else {
                array->data[index] = (MYFLT) result->getDouble(colIndex + 1);
            }
            colIndex++;
            index++;
        }
    }
    delete result;
}

void MySQLConnection::ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, true);
}

void MySQLConnection::ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    ToArray(Query(sql), csound, array, false);
}
