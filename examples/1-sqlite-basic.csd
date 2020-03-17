<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>
<CsInstruments>
/*
    EXAMPLE 1

    print some data from a sqlite3 database at i-rate
    

*/

sr = 44100
kr = 4410
nchnls = 2
0dbfs = 1
seed 0

; sqlite3 takes a file path, or :memory: for a temporary in-memory database
; gidb dbconnect "sqlite", "/tmp/example.db"
gidb dbconnect "sqlite", ":memory:"

instr start_example

    ; print a random value
    ires1 dbscalar gidb, "SELECT RANDOM()"
    print ires1

    ; print the second column (1) of the first row (0)
    ires2 dbscalar gidb, "SELECT 99, 98", 0, 1
    print ires2
endin


</CsInstruments>
<CsScore>
i "start_example" 0 1

</CsScore>
</CsoundSynthesizer>