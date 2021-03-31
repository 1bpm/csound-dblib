/*
    opcodes.cpp
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
#include <atomic>
#include <plugin.h>
#include <iostream>
#include "connection.h"

const char* dbname = "::dbconnection%d";
const char* badHandle = "cannot obtain connection from handle";

#define LOCK(connection) csound->get_csound()->LockMutex(connection->mutex);
#define UNLOCK(connection) csound->get_csound()->UnlockMutex(connection->mutex);

/*
 * Obtain connection from global variables by handle
 */
ConnectionData* getConnection(csnd::Csound* csound, MYFLT handle) {
    char buffer[32];
    snprintf(buffer, 32, dbname, (int)handle);
    return (ConnectionData*) csound->query_global_variable(buffer);  
}


/*
 * Create connection in global variables returning handle
 */
MYFLT CreateHandle(csnd::Csound* csound, ConnectionData** connection) {
    char buffer[32];
    int handle = 0;
    snprintf(buffer, 32, dbname, handle);
    while ((*connection = (ConnectionData*) csound->query_global_variable(buffer)) != NULL) {
        snprintf(buffer, 32, dbname, ++handle);
    }
    csound->create_global_variable(buffer, sizeof(ConnectionData));
    *connection = (ConnectionData*) csound->query_global_variable(buffer);
    
    return FL(handle);
}

/*
 * Thread for k rate query handling
 */
class QueryThread : public csnd::Thread {
    std::atomic_bool spinlock;
    std::atomic_bool on;
    ConnectionData* connection;
    QueryData* queryData;
    int sleepTime;
    
public:
    bool pending;  // if there is a query waiting to be processed
    char* charData;
    int charSize;
    char* error;
    int status;
    MYFLT flData;
    bool done;
    int ident;
    QueryThread(csnd::Csound *csound, ConnectionData* connection, QueryData* queryData) : 
        Thread(csound), 
        done(false), 
        queryData(queryData), 
        charSize(1), 
        status(0), 
        spinlock(false), 
        pending(false), 
        on(true), 
        connection(connection),
        sleepTime(10) {};
    
    
    uintptr_t run() {
        while(on) { 
            lock();
            if (pending) {
                LOCK(connection);
                try {     
                    switch (queryData->queryType) {
                        case SCALARSTRING: {
                            std::string resultString = connection->ScalarString(queryData->sql, queryData->row, queryData->col);
                            if (charData != NULL) {
                                csound->free(charData);
                            }
                            charData = csound->strdup(resultString.c_str());
                            charSize = resultString.length() + 1;
                        }
                            break;
                        case SCALAR: {
                            flData = connection->Scalar(queryData->sql, queryData->row, queryData->col);
                        }
                            break;
                        case EXEC: {
                            connection->Exec(queryData->sql);
                            
                        }
                            break;
                        case ARRAY: {
                            connection->ArrayQuery(queryData->sql, csound, queryData->array);
                        }
                            break;
                        case ARRAYSTRING: {
                            connection->ArrayQueryString(queryData->sql, csound, queryData->array);
                        }
                            break;
                    }
                    status = 0;
                    done = true;
                    pending = false;
                } catch (const std::exception &e) {
                    status = 1;
                    done = true;
                    pending = false;
                    error = csound->strdup(e.what());
                }
                UNLOCK(connection);
            }
            unlock();
            csound->sleep(sleepTime);
        }
        return 0;
    }
    
    void do_query(char* sql, int row=0, int col=0) {
        if (queryData->sql != NULL) {
            //csound->free(queryData->sql);
        }

        queryData->sql = sql;
        queryData->row = row;
        queryData->col = col;
        pending = true; 
        
    }
    
    void lock() {
        while (spinlock == true) {
            csound->sleep(sleepTime); // have as one k cycle ?? // was 100
        }
        spinlock = true;
    }
    
    void unlock() {
        spinlock = false;
    }
    
    void stop() {
        on = false;
    }
};




struct dbconnect_full : csnd::Plugin<1, 5> {
    static constexpr char const *otypes = "i";
    static constexpr char const *itypes = "SSSSS";
    ConnectionData* connection;
    
    int init() {
        csound->plugin_deinit(this);
        outargs[0] = CreateHandle(csound, &connection);
        connection->mutex = csound->get_csound()->Create_Mutex(0);
        
        STRINGDAT &dbType = inargs.str_data(0);
        STRINGDAT &dbHost = inargs.str_data(1);
        STRINGDAT &dbName = inargs.str_data(2);
        STRINGDAT &dbUser = inargs.str_data(3);
        STRINGDAT &dbPass = inargs.str_data(4);
        LoginData* login = (LoginData*) csound->malloc(sizeof(LoginData));
        
        try {
            if (!strcmp(dbType.data, "postgresql")) {
                login->dbType = POSTGRES;
            } else if (!strcmp(dbType.data, "mysql")) {
                login->dbType = MYSQL;
            } else {
                return csound->init_error("database type not supported");
            }
            login->dbHost = dbHost.data;
            login->dbName = dbName.data;
            login->dbUser = dbUser.data;
            login->dbPass = dbPass.data;

            connection->Init(csound, login);
            
        } catch (const std::exception &e) {
            return csound->init_error(e.what());
        }
        return OK;
    }
    
    int deinit() {
        bool error;
        LOCK(connection);
        try {
            connection->Close(csound);
        } catch (const std::exception &e) {
            error = true;
        }
        UNLOCK(connection);
        csound->get_csound()->DestroyMutex(connection->mutex);
        return (error)? NOTOK: OK;
    }
};

struct dbconnect_short : csnd::Plugin<1, 2> {
    static constexpr char const *otypes = "i";
    static constexpr char const *itypes = "SS";
    ConnectionData* connection;
    
    int init() {
        csound->plugin_deinit(this);
        outargs[0] = CreateHandle(csound, &connection);
        connection->mutex = csound->get_csound()->Create_Mutex(0);
        
        STRINGDAT &dbType = inargs.str_data(0);
        STRINGDAT &dbName = inargs.str_data(1);
        
        LoginData* login = (LoginData*) csound->malloc(sizeof(LoginData));
        try {
            if (strcmp(dbType.data, "sqlite")) {
                return csound->init_error("database type not supported");
            } 
            login->dbType = SQLITE;
            login->dbName = dbName.data;
            
            connection->Init(csound, login);
        } catch (const std::exception &e) {
            return csound->init_error(e.what());
        }
        return OK;
    }
    
    int deinit() {
        bool error;
        LOCK(connection);
        try {
            connection->Close(csound);
        } catch (const std::exception &e) {
            error = true;
        }
        UNLOCK(connection);;
        csound->get_csound()->DestroyMutex(connection->mutex);
        return (error)? NOTOK: OK;
    }
};





/*
 * Base struct for threaded k-rate queries
 */
template <std::size_t N, std::size_t M> struct DBPluginBaseK : csnd::Plugin<N, M> {
    using csnd::Plugin<N, M>::inargs;
    using csnd::Plugin<N, M>::outargs;
    using csnd::Plugin<N, M>::csound;
    ConnectionData* connection;
    QueryThread query;
    bool singleRun;
    bool singleComplete;

    int setup(int queryType, int row=0, int col=0, ARRAYDAT* array=NULL) {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        csound->plugin_deinit(this);
        
        QueryData* queryData = (QueryData*) csound->malloc(sizeof(QueryData));
        queryData->queryType = queryType;
        
        queryData->row = row;
        queryData->col = col;
        
        if (array) {
            queryData->array = array;
        }

        if (inargs[2] == FL(-1)) {
            singleRun = true;
        }
        
        csnd::constr(&query, csound, connection, queryData);
        return OK;
    }
    
    int deinit() {
        query.stop();
        query.join();
        csnd::destr(&query);
        return OK;
    }
    
    MYFLT kcycle(int row=0, int col=0) {
        outargs[0] = FL(0);
        if (singleComplete && singleRun) {
            return OK;
        }
        
        if (UNLIKELY(query.done)) {
            if (query.status) {
                return csound->perf_error(query.error, this);
            }
            if (singleRun) {
                singleComplete = true;
            }
            query.lock();
            query.done = false;
            query.unlock();
            outargs[0] = FL(1);
        }
        
        if (UNLIKELY(!query.pending &&
                (inargs[2] == FL(1) || 
                singleRun)
                )) {
            query.lock();
            query.do_query(csound->strdup(inargs.str_data(1).data), row, col);
            query.unlock();
        }
        
        return OK;
    }
    
};


// threaded k rate opcodes

struct dbexec_k : DBPluginBaseK<1, 3> {
    static constexpr char const *otypes = "k";
    static constexpr char const *itypes = "iSk";
    
    int init() {
        return setup(EXEC);
    }
    
    int kperf() {
        return kcycle();
    }
};



struct dbscalar_k : DBPluginBaseK<2, 5> {
    static constexpr char const *otypes = "kk";
    static constexpr char const *itypes = "iSkOO";
    
    int init() {
        return setup(SCALAR, (int)inargs[3], (int)inargs[4]);
    }

    int kperf() {
        int response = kcycle((int)inargs[3],  (int)inargs[4]);
        outargs[1] = query.flData;
        return response;
    }
};


struct dbscalarstr_k : DBPluginBaseK<2, 5> {
    static constexpr char const *otypes = "kS";
    static constexpr char const *itypes = "iSkOO";
    
    int init() {
        return setup(SCALARSTRING, (int)inargs[3], (int)inargs[4]);
    }
    
    int kperf() {
        int response = kcycle((int)inargs[3],  (int)inargs[4]);
        STRINGDAT &result = outargs.str_data(1);
        result.size = query.charSize;  
        result.data = query.charData;
        return response;
    }
};


struct dbarray_k : DBPluginBaseK<2, 3> {
    static constexpr char const *otypes = "kk[][]";
    static constexpr char const *itypes = "iSk";
    
    int init() {
        return setup(ARRAY, NULL, NULL, (ARRAYDAT*) outargs(1));
    }

    int kperf() {
        return kcycle();
    }
    
};


struct dbarraystr_k : DBPluginBaseK<2, 3> {
    static constexpr char const *otypes = "kS[][]";
    static constexpr char const *itypes = "iSk";
    
    int init() {
        return setup(ARRAYSTRING, NULL, NULL, (ARRAYDAT*) outargs(1));
    }
    
    int kperf() {
        return kcycle();
    } 
};




struct dbexec : csnd::InPlug<2> {
    static constexpr char const *otypes = "";
    static constexpr char const *itypes = "iS";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, args[0]))) {
            return csound->init_error(badHandle);
        }
        
        STRINGDAT &sql = args.str_data(1);
        LOCK(connection);
        try {
            connection->Exec(sql.data);
            return OK;
        } catch (const std::exception &e) {
            return csound->init_error(e.what());
        } 
        UNLOCK(connection);
        
    }
};



struct dbscalar : csnd::Plugin<1, 4> {
    static constexpr char const *otypes = "i";
    static constexpr char const *itypes = "iSoo";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        STRINGDAT &sql = inargs.str_data(1);
        try {
            LOCK(connection);
            outargs[0] = connection->Scalar(sql.data, inargs[2], inargs[3]);
            UNLOCK(connection);
            return OK;
        } catch (const std::exception &e) {
            return csound->init_error(e.what());
        } 
    }
};




struct dbscalarstr : csnd::Plugin<1, 4> {
    static constexpr char const *otypes = "S";
    static constexpr char const *itypes = "iSoo";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        STRINGDAT &sql = inargs.str_data(1);
        STRINGDAT &result = outargs.str_data(0);
        try {
            LOCK(connection);
            std::string resultString = connection->ScalarString(sql.data, inargs[2], inargs[3]);
            UNLOCK(connection);
            result.size = resultString.length() + 1;
            result.data = csound->strdup(resultString.c_str());
            return OK;
        } catch (const std::exception &e) {
            return csound->init_error(e.what());
        } 
    }
};






struct dbarray : csnd::Plugin<1, 2> {
    static constexpr char const *otypes = "i[][]";
    static constexpr char const *itypes = "iS";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        STRINGDAT &sql = inargs.str_data(1);

        ARRAYDAT* array = (ARRAYDAT*) outargs(0);

        try {
            LOCK(connection);
            connection->ArrayQuery(sql.data, csound, array);
            UNLOCK(connection);
            return OK;
        } catch (const std::exception &e) {
            return csound->init_error(e.what());
        }
    }
};





struct dbarraystr : csnd::Plugin<1, 2> {
    static constexpr char const *otypes = "S[][]";
    static constexpr char const *itypes = "iS";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        STRINGDAT &sql = inargs.str_data(1);

        ARRAYDAT* array = (ARRAYDAT*) outargs(0);

        try {
            LOCK(connection);
            connection->ArrayQueryString(sql.data, csound, array);
            UNLOCK(connection);
            return OK;
        } catch (const std::exception &e) {
            return csound->init_error(e.what());
        }
    }
};






// k rate blocking opcodes


struct dbexec_kb : csnd::InPlug<2> {
    static constexpr char const *otypes = "";
    static constexpr char const *itypes = "iS";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, args[0]))) {
            return csound->init_error(badHandle);
        }
        return OK;
        
    }
    
    int kperf() {
        STRINGDAT &sql = args.str_data(1);
        try {
            LOCK(connection);
            connection->Exec(sql.data);
            UNLOCK(connection);
            return OK;
        } catch (const std::exception &e) {
            return csound->perf_error(e.what(), this);
        } 
    }
};



struct dbscalar_kb : csnd::Plugin<1, 4> {
    static constexpr char const *otypes = "k";
    static constexpr char const *itypes = "iSOO";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        return OK;
    }
    
    int kperf() {
        STRINGDAT &sql = inargs.str_data(1);
        try {
            LOCK(connection);
            outargs[0] = connection->Scalar(sql.data, inargs[2], inargs[3]);
            UNLOCK(connection);
            return OK;
        } catch (const std::exception &e) {
            return csound->perf_error(e.what(), this);
        } 
    }
};




struct dbscalarstr_kb : csnd::Plugin<1, 4> {
    static constexpr char const *otypes = "S";
    static constexpr char const *itypes = "iSOO";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        return OK;
    }
    
    int kperf() {
            
        STRINGDAT &sql = inargs.str_data(1);
        STRINGDAT &result = outargs.str_data(0);
        try {
            LOCK(connection);
            std::string resultString = connection->ScalarString(sql.data, inargs[2], inargs[3]);
            UNLOCK(connection);
            result.size = resultString.length() + 1;
            result.data = csound->strdup(resultString.c_str());
            return OK;
        } catch (const std::exception &e) {
            return csound->perf_error(e.what(), this);
        } 
    }
};






struct dbarray_kb : csnd::Plugin<1, 2> {
    static constexpr char const *otypes = "k[][]";
    static constexpr char const *itypes = "iS";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        return OK;
    }
    
    int kperf() {
        STRINGDAT &sql = inargs.str_data(1);

        ARRAYDAT* array = (ARRAYDAT*) outargs(0);

        try {
            LOCK(connection);
            connection->ArrayQuery(sql.data, csound, array);
            UNLOCK(connection);
            return OK;
        } catch (const std::exception &e) {
            return csound->perf_error(e.what(), this);
        }
    }
};





struct dbarraystr_kb : csnd::Plugin<1, 2> {
    static constexpr char const *otypes = "S[][]";
    static constexpr char const *itypes = "iS";
    ConnectionData* connection;
    
    int init() {
        if (!(connection = getConnection(csound, inargs[0]))) {
            return csound->init_error(badHandle);
        }
        return OK;
    }
    
    int kperf() {
        STRINGDAT &sql = inargs.str_data(1);

        ARRAYDAT* array = (ARRAYDAT*) outargs(0);

        try {
            LOCK(connection);
            connection->ArrayQueryString(sql.data, csound, array);
            UNLOCK(connection);
            return OK;
        } catch (const std::exception &e) {
            return csound->perf_error(e.what(), this);
        }
    }
};








#include <modload.h>

void csnd::on_load(csnd::Csound *csound) {

    csnd::plugin<dbconnect_full>(csound, "dbconnect.f", csnd::thread::i);
    csnd::plugin<dbconnect_short>(csound, "dbconnect.s", csnd::thread::i);
    
    csnd::plugin<dbexec>(csound, "dbexec", csnd::thread::i);
    csnd::plugin<dbscalar>(csound, "dbscalar", csnd::thread::i);
    csnd::plugin<dbscalarstr>(csound, "dbscalar.S", csnd::thread::i);
    csnd::plugin<dbarray>(csound, "dbarray", csnd::thread::i);
    csnd::plugin<dbarraystr>(csound, "dbarray.S", csnd::thread::i);
    
    csnd::plugin<dbexec_k>(csound, "dbexec_k", csnd::thread::ik);
    csnd::plugin<dbscalar_k>(csound, "dbscalar_k", csnd::thread::ik);
    csnd::plugin<dbscalarstr_k>(csound, "dbscalar_k.S", csnd::thread::ik);
    csnd::plugin<dbarray_k>(csound, "dbarray_k", csnd::thread::ik);
    csnd::plugin<dbarraystr_k>(csound, "dbarray_k.S", csnd::thread::ik);
    
    csnd::plugin<dbexec_kb>(csound, "dbexec_kb", csnd::thread::ik);
    csnd::plugin<dbscalar_kb>(csound, "dbscalar_kb", csnd::thread::ik);
    csnd::plugin<dbscalarstr_kb>(csound, "dbscalar_kb.S", csnd::thread::ik);
    csnd::plugin<dbarray_kb>(csound, "dbarray_kb", csnd::thread::ik);
    csnd::plugin<dbarraystr_kb>(csound, "dbarray_kb.S", csnd::thread::ik);

}
