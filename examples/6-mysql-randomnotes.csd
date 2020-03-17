<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>
<CsInstruments>
/*
    EXAMPLE 3

    Insert some frequencies and durations to a MySQL database
    then query/play random pairs
    

*/

sr = 44100
kr = 4410
nchnls = 2
0dbfs = 1
seed 0


; connect: type, hostname or IP, database name, username, password
gidb dbconnect "mysql", "localhost", "databasename", "username", "password"


; initialise the example
instr start_example

    ; create tables if not existing/truncate if existing and fill with frequency and duration data
    dbexec gidb, "CREATE TABLE IF NOT EXISTS frequencies (freq FLOAT)"
    dbexec gidb, "TRUNCATE TABLE frequencies"
    dbexec gidb, "CREATE TABLE IF NOT EXISTS durations (dur FLOAT)"
    dbexec gidb, "TRUNCATE TABLE durations"
    dbexec gidb, "INSERT INTO frequencies (freq) VALUES (277.2), (349.272), (415.24559), (523.35359), (391.96079)"
    dbexec gidb, "INSERT INTO frequencies (freq) SELECT freq*2 FROM frequencies"
    dbexec gidb, "INSERT INTO durations (dur) VALUES (0.1), (0.2), (0.4), (0.8), (1), (1.2)"

    ; run the sequencer instrument for 30 seconds
    event_i "i", "sequence", 0, 30
endin


; random sequencer instrument
instr sequence

    ; query to return random frequency/duration pair
    Sql = {{
        SELECT f.freq, d.dur
        FROM frequencies f 
        JOIN (SELECT dur FROM durations ORDER BY RAND() LIMIT 1) d
        ORDER BY RAND() 
        LIMIT 1
    }}

    ; array needs to be initialised or schedkwhen complains
    kevent[][] init 1, 2    

    ; query twice per second
    ktrig metro 3
    
    ; query on metro returning the frequency/duration pair
    kdone, kevent[][] dbarray_k gidb, Sql, ktrig

    ; schedule the note
    schedkwhen kdone, 0, 0, "osc", 0, kevent[0][1], kevent[0][0]

endin


; play a note
instr osc

    ; pick a random waveform
    ifn = int(random(1, 4))

    ; set frequency and envelope
    ifreq = p4
    kamp line 0.5, p3, 0

    ; play it
    a1 oscil kamp, ifreq, ifn
    outs a1, a1
endin




</CsInstruments>
<CsScore>
f1 0 16384 10 1
f2 0 16384 10 1 0.5 0.3 0.25 0.2 0.167 0.14 0.125 .11
f3 0 16384 10 1 0 0.3 0 0.2 0 0.14 0 .111
f4 0 16384 10 1 1 1 1 0.7 0.5 0.3 0.1

; begin example
i "start_example" 0 1

</CsScore>
</CsoundSynthesizer>