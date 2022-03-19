/*
    connection.cpp
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
#include "connection.h"


const char* badDatabase = "database type not supported";
const char* badConnection = "connection not open";

// tried to do this with templates, failed with various approaches 
//   hence the copious number of switches etc (horrible?). Due a revisit/refactor.

void ConnectionData::Init(csnd::Csound* csound, LoginData* login) {
    type = login->dbType;
    switch (type) {
#ifdef BUILD_POSTGRES
        case POSTGRES:
            postgres = (PostgresConnection*) csound->malloc(sizeof(PostgresConnection));
            postgres->Init(csound, login);
            break;
#endif
#ifdef BUILD_SQLITE
        case SQLITE:
            sqlite = (SqliteConnection*) csound->malloc(sizeof(SqliteConnection));
            sqlite->Init(csound, login);
            break;
#endif
#ifdef BUILD_MYSQL
        case MYSQL:
            mysql = (MySQLConnection*) csound->malloc(sizeof(MySQLConnection));
            mysql->Init(csound, login);
            break;
#endif
        default:
            throw std::runtime_error(badDatabase);
    }
    open = true;
}

void ConnectionData::Close(csnd::Csound* csound) {
    if (!open) {
        throw std::runtime_error(badConnection);
    }
    
    switch (type) {
#ifdef BUILD_POSTGRES        
        case POSTGRES:
            postgres->Close(csound);
            break;
#endif
#ifdef BUILD_SQLITE
        case SQLITE:
            sqlite->Close(csound);
            break;
#endif
#ifdef BUILD_MYSQL
        case MYSQL:
            mysql->Close(csound);
            break;
#endif
        default:
            throw std::runtime_error(badDatabase);
    }
    open = false;
}

void ConnectionData::Exec(char* sql) {
    if (!open) {
        throw std::runtime_error(badConnection);
    }
    
    switch (type) {
#ifdef BUILD_POSTGRES
        case POSTGRES:
            postgres->Exec(sql);
            break;
#endif
#ifdef BUILD_SQLITE
        case SQLITE:
            sqlite->Exec(sql);
            break;
#endif
#ifdef BUILD_MYSQL
        case MYSQL:
            mysql->Exec(sql);
            break;
#endif
        default:
            throw std::runtime_error(badDatabase);
    }
}

MYFLT ConnectionData::Scalar(char* sql, int row, int col) {
    if (!open) {
        throw std::runtime_error(badConnection);
    }
    
    switch (type) {
#ifdef BUILD_POSTGRES
        case POSTGRES:
            return postgres->Scalar(sql, row, col);
            break;
#endif
#ifdef BUILD_SQLITE
        case SQLITE:
            return sqlite->Scalar(sql, row, col);
            break;
#endif
#ifdef BUILD_MYSQL
        case MYSQL:
            return mysql->Scalar(sql, row, col);
            break;
#endif
        default:
            throw std::runtime_error(badDatabase);
    }

}

char* ConnectionData::ScalarString(char* sql, int row, int col) {
    if (!open) {
        throw std::runtime_error(badConnection);
    }
    
    switch (type) {
#ifdef BUILD_POSTGRES
        case POSTGRES:
            return postgres->ScalarString(sql, row, col);
            break;
#endif
#ifdef BUILD_SQLITE
        case SQLITE:
            return sqlite->ScalarString(sql, row, col);
            break;
#endif
#ifdef BUILD_MYSQL
        case MYSQL:
            return mysql->ScalarString(sql, row, col);
            break;
#endif
        default:
            throw std::runtime_error(badDatabase);
    }

}

void ConnectionData::ArrayQuery(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    if (!open) {
        throw std::runtime_error(badConnection);
    }
    
    switch (type) {
#ifdef BUILD_POSTGRES        
        case POSTGRES:
            return postgres->ArrayQuery(sql, csound, array);
            break;
#endif
#ifdef BUILD_SQLITE            
        case SQLITE:
            return sqlite->ArrayQuery(sql, csound, array);
            break;
#endif
#ifdef BUILD_MYSQL            
        case MYSQL:
            return mysql->ArrayQuery(sql, csound, array);
            break;
#endif            
        default:
            throw std::runtime_error(badDatabase);
    }

}

void ConnectionData::ArrayQueryString(char* sql, csnd::Csound* csound, ARRAYDAT* array) {
    if (!open) {
        throw std::runtime_error(badConnection);
    }
    
    switch (type) {
#ifdef BUILD_POSTGRES        
        case POSTGRES:
            return postgres->ArrayQueryString(sql, csound, array);
            break;
#endif
#ifdef BUILD_SQLITE            
        case SQLITE:
            return sqlite->ArrayQueryString(sql, csound, array);
            break;
#endif
#ifdef BUILD_MYSQL            
        case MYSQL:
            return mysql->ArrayQueryString(sql, csound, array);
            break;
#endif          
        default:
            throw std::runtime_error(badDatabase);
    }

}
