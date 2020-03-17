<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>
<CsInstruments>
/*
    EXAMPLE 3

    Run some queries on a PostgreSQL database using just inbuilt functions and system tables
    

*/

sr = 44100
kr = 4410
nchnls = 2
0dbfs = 1
seed 0


; connect: type, hostname or IP, database name, username, password
gidb dbconnect "postgresql", "localhost", "databasename", "username", "password"


; print a random float from the database
instr execscalar
    ires dbscalar gidb, "SELECT RANDOM()"
    print ires
endin


; print the current_timestamp from the database
instr execscalarstr
    Sres dbscalar gidb, "SELECT current_timestamp || '\n'"
    prints Sres
endin


; print some float columns from the database activity statistics table
instr execarray
    ires[][] dbarray gidb, "SELECT datid, pid, usesysid FROM pg_stat_activity"
    printarray ires
endin


; print some string columns from the database activity table
instr execarraystr
    Sres[][] dbarray gidb, "SELECT datname, usename, state, query_start FROM pg_stat_activity"
    irow = 0

    ; loop through as printarray does not support multidimensional string arrays
    while (irow < lenarray(Sres)) do
        icol = 0
        while (icol < 3) do
            Sitem sprintf "%d, %d : %s\n", irow, icol, Sres[irow][icol]
            prints Sitem
            icol += 1
        od
        irow += 1
    od
    
endin


; print the current_timestamp from the database server twice per second, and print when the query has been executed
instr execscalar_k
    ktrigger metro 2
    kdone, Sres dbscalar_k gidb, "SELECT 'now = ' || current_timestamp || '\n'", ktrigger
    if (kdone == 1) then
        printks "%s", 0, Sres
    endif
endin


; print 2x2 random floats from the database every second
instr execarray_k
    ktrigger metro 1
    kdone, kres[][] dbarray_k gidb, "SELECT RANDOM(), RANDOM() UNION SELECT RANDOM(), RANDOM()", ktrigger
    if (kdone == 1) then
        printk2 kres[0][0]
        printk2 kres[0][1]
        printk2 kres[1][0]
        printk2 kres[1][1]
    endif

endin


</CsInstruments>
<CsScore>

i "execscalar" 0 1
i "execscalarstr" 2 1
i "execarray" 4 1
i "execarraystr" 6 1
i "execscalar_k" 7 5
i "execarray_k" 12 5

</CsScore>
</CsoundSynthesizer>