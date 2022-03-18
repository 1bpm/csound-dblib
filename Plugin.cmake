set(PLUGIN_NAME sqldb)

# Dependencies
find_package(PostgreSQL)
find_package(Sqlite3)
find_package(MySQL)

# Check databases available
if(NOT PostgreSQL_FOUND AND NOT SQLITE3_FOUND AND NOT MYSQLCONNECTORCPP_FOUND)
  message(FATAL_ERROR "No database libraries could be found")
endif()

# Source files
set(CPPFILES src/opcodes.cpp)
set(INCLUDES ${CSOUND_INCLUDE_DIRS} "include")
set(LIBS "")

if(PostgreSQL_FOUND)
  message(STATUS "Using PostgreSQL")
  list(APPEND CPPFILES "src/postgresql.cpp")
  list(APPEND INCLUDES ${PostgreSQL_INCLUDE_DIRS})
  list(APPEND LIBS ${PostgreSQL_LIBRARIES})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_POSTGRES")
endif()

if(SQLITE3_FOUND) 
  message(STATUS "Using SQLite3")
  list(APPEND CPPFILES "src/sqlite3.cpp")
  list(APPEND INCLUDES ${SQLITE3_INCLUDE_DIRS})
  list(APPEND LIBS ${SQLITE3_LIBRARIES})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_SQLITE")
endif()

if(MYSQLCONNECTORCPP_FOUND) 
  message(STATUS "Using MySQL")
  list(APPEND CPPFILES "src/mysql.cpp")
  list(APPEND INCLUDES ${MYSQLCONNECTORCPP_INCLUDE_DIRS})
  list(APPEND LIBS ${MYSQLCONNECTORCPP_LIBRARIES})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_MYSQL")
endif()


make_plugin(${PLUGIN_NAME} "${CPPFILES}" ${LIBS})
target_include_directories(${PLUGIN_NAME} PRIVATE ${INCLUDES})

