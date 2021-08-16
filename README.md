## Introduction
csound-sqldb provides database access opcodes which allow for the querying of data in SQL databases at i- and k-rate.
MySQL, PostgreSQL and SQLite3 are supported and are used with the same opcodes after the initial connection. The opcodes have currently been tested on Linux only.

The opcodes rely on changes made after the official 6.13.0 release of Csound so while awaiting the next release the Csound source will need to be obtained from the develop branch of https://github.com/csound/csound/ otherwise they will not build.

## Opcode overview
The opcodes are detailed in full in OpcodeDocumentation.md
Aside from the connection opcode, there are three groups available for each to allow for design options particularly when considering database latency/performance:

 - i-rate : to be used when db latency is particularly low, in global orchestra space or not running in realtime etc.
 - k-rate : these execute in a separate thread invoked by a trigger, and emit a trigger when complete so can always safely be used in audio generating instruments.
 - k-rate (blocking) : these execute at k-rate but block the instrument execution so would be used in special cases such as non-realtime analysis instruments (eg in example 7) or when generating audio offline etc.

## Requirements

 - Csound libraries
 - Cmake >= 2.8.12
 - At least one database development library from the three below. Opcodes will support whichever of these databases can be found, which will be indicated when running the initial cmake command.

### MySQL
Connector/C++ (https://dev.mysql.com/downloads/connector/cpp/)

	# apt get install libmysqlcppconn-dev

### PostgreSQL
libpq (https://www.postgresql.org/download/)

	# apt get install libpq-dev
	
### SQLite
libsqlite (https://www.sqlite.org/download.html)
	
	# apt get install libsqlite3-dev


## Building
Create a build directory at the top of the source tree, execute *cmake ..*, *make* and optionally *make install* as root. If the latter is not used/possible then the resulting libsqldb.so can be used with the *--opcode-lib* flag in Csound.
eg:

	git clone https://github.com/1bpm/csound-sqldb.git
	cd csound-sqldb
	mkdir build && cd build
	cmake ..
	make && sudo make install

## Examples
A number of examples are included in the examples directory. Generally the syntax of each opcode is agnostic to the database type used, so the different techniques in each can be used for any database type.



By Richard Knight 2019
