<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>
<CsInstruments>
/*
    EXAMPLE 2

    Use a metro to trigger the insertion of points from a line into a sqlite database,
    then read back the points in reverse order and play back with enveloping
    

*/

sr = 44100
kr = 4410
nchnls = 2
0dbfs = 1
seed 0


; sqlite3 takes a file path, or :memory: for a temporary in-memory database
;gidb dbconnect "sqlite", ":memory:"
gidb dbconnect "sqlite", "/tmp/example.db"


instr start_example

    ; create the table with just one float column
    kdone1 dbexec_k gidb, "CREATE TABLE IF NOT EXISTS frequencies (freq float)", -1

    ; execute when create table statement has completed - clear out if the table does exist 
    kdone2 dbexec_k gidb, "DELETE FROM frequencies", kdone1
    
    ; execute when delete has completed
    schedkwhen kdone2, 0, 0, "db_insert", 0, 5
    schedkwhen kdone2, 0, 0, "db_select", 5, 10


endin


; insert frequency values from line into database at k rate
instr db_insert

    ; generate some data and print
    kfreq line 440, p3, 1000
    ktrigger metro 5
    printk 0.2, kfreq

    ; insert snapshot with each metro (as long as the last query is still not pending)
    Squery sprintfk "INSERT INTO frequencies (freq) VALUES (%f)", kfreq
    kdone dbexec_k gidb, Squery, ktrigger
    
endin


; read frequency values from database at i rate: obtain the values in reverse order 
instr db_select

    ; get all values in descending order into an array
    ires[][] dbarray gidb, "SELECT freq FROM frequencies ORDER BY freq desc"
    itimestep = 0.3
    index = 0
    itime = 0
    ilast = 0

    ; loop through the values and play the frequency with some basic interpolation between last current values
    while (index < lenarray(ires)) do
        if (ilast != 0) then
            event_i "i", "playpitch", itime, itimestep, ilast, ires[index][0]
        endif
        ilast = ires[index][0]
        index += 1
        itime += itimestep * 0.8
    od
endin


; basic oscillator instrument with envelope
instr playpitch
    ipitch1 = p4
    ipitch2 = p5
    kamp linseg 0, p3*0.2, 1, p3*0.6, 1, p3*0.2, 0
    kpitch line ipitch1, p3, ipitch2
    aout oscil 0.8, kpitch
    outs aout*kamp, aout*kamp
endin

</CsInstruments>
<CsScore>
i "start_example" 0 15

</CsScore>
</CsoundSynthesizer>