
# Connection

### dbconnect
	idb dbconnect Stype, Spath
	idb dbconnect Stype, Shost, Sdatabase, Suser, Spassword

Connect to a database of type *Stype* and keep the reference in the handle *idb*

SQLite connections require only the path to the database file, or *:memory:* can be used for an in-memory database.
MySQL and PostgreSQL connections require hostname/IP, database name, username and password. 


Recommended to be run in the global orchestra space.

	; SQLite connection
	gidb dbconnect "sqlite", "/path/to/database.db"

	; MySQL and PostgreSQL connections
	gidb dbconnect "mysql", "hostname", "database_name", "username", "password"
	gidb dbconnect "postgresql", "hostname", "database_name", "username", "password"

# Executing statements
All opcodes require

 - *idb* , which is the handle created by the dbconnect opcode.
 - *Squery*, which is the SQL statement to be executed.

## i-rate query opcodes



### dbexec
	dbexec idb, Squery
Execute a SQL statement returning no results.

	dbexec gidb, "CREATE TABLE frequencies (frequency FLOAT)"
### dbscalar
	ires dbscalar idb, Squery [, irow] [, icolumn]
	Sres dbscalar idb, Squery [, irow] [, icolumn]
Return a single numeric or string value. Optionally *irow* and *icolumn* can be specified which default to 0 and 0 , ie the first value.

	inumber dbscalar gidb, "SELECT 1, 2, 3", 0, 2
	Svalue dbscalar gidb, "SELECT 'this', 'is', 'a', 'test'"
	
### dbarray
	ires[][] dbarray idb, Squery
	Sres[][] dbarray idb, Squery
Return a two-dimensional numeric or string array.

	ires[][] dbarray gidb, "SELECT 1, 2 UNION SELECT 3, 4"
	Sres[][] dbarray gidb, "SELECT 'this', 'is' UNION SELECT 'a', 'test'"


## k-rate query opcodes
All opcodes require

 - *ktrigger* , which triggers the execution of the statement when the value is 1 or -1. If -1, any future triggers are ignored. For example *ktrigger* can be set to -1 initially and the statement will execute only once.

All opcodes emit

 - *kdone* , which is set to 1 for a single k-cycle when the statement execution has completed.

### dbexec_k
	kdone dbexec_k idb, Squery, ktrigger
Execute a SQL statement returning no results.

	kdone dbexec_k gidb, "CREATE TABLE frequencies (frequency FLOAT)", -1

### dbscalar_k
	kdone, kres dbscalar idb, Squery, ktrigger [, krow] [, kcolumn]
	kdone, Sres dbscalar idb, Squery, ktrigger [, krow] [, kcolumn]
Return a single numeric or string value. Optionally *krow* and *kcolumn* can be specified which default to 0 and 0 , ie the first value.


	kdone, knumber dbscalar_k gidb, "SELECT 1, 2, 3", ktrigger, 0, 2
	kdone, Svalue dbscalar_k gidb, "SELECT 'this', 'is', 'a', 'test'", ktrigger
	
### dbarray_k
	kdone, kres[][] dbarray_k idb, Squery, ktrigger
	kdone, Sres[][] dbarray_k idb, Squery, ktrigger
Return a two-dimensional numeric or string array. Note: any operations on the result array may fail before the query has completed. Hence ideally initialise the array first, or make sure *kdone* == 1 , otherwise the accessing the array will fail.

	kdone, kres[][] dbarray gidb, "SELECT 1, 2 UNION SELECT 3, 4", -1
	kdone, Sres[][] dbarray_k gidb, "SELECT 'this', 'is' UNION SELECT 'a', 'test'", ktrigger

## k-rate query opcodes (blocking)
These opcodes will block the execution of the k-cycle until complete so should not be used for realtime purposes. Ie offline rendering or special operations (eg in example 7) are special cases in which they could be used.


### dbexec_kb
	dbexec_kb idb, Squery
Execute a SQL statement returning no results.

	dbexec_kb gidb, "CREATE TABLE frequencies (frequency FLOAT)"

### dbscalar_kb
	kres dbscalar_kb idb, Squery [, krow] [, kcolumn]
	Sres dbscalar_kb idb, Squery [, krow] [, kcolumn]
Return a single numeric or string value. Optionally *krow* and *kcolumn* can be specified which default to 0 and 0 , ie the first value.

	knumber dbscalar_kb gidb, "SELECT 1, 2, 3", 0, 2
	Svalue dbscalar_kb gidb, "SELECT 'this', 'is', 'a', 'test'"
	
### dbarray_kb
	kres[][] dbarray_kb idb, Squery
	Sres[][] dbarray_kb idb, Squery
Return a two-dimensional numeric or string array.

	kres[][] dbarray_kb gidb, "SELECT 1, 2 UNION SELECT 3, 4"
	Sres[][] dbarray_kb gidb, "SELECT 'this', 'is' UNION SELECT 'a', 'test'"
