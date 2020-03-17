<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>
<CsInstruments>
/*
    EXAMPLE 9

    A rudimentary pitch tracking concatenative resynthesizer

    This creates a sqlite database in memory, and then uses the opcode getpitches to scan through a 
    sound file in non-realtime, storing detected pitches and the relevant offset time of the pitch in 
    the database. The example file is a series of ascending piano notes.
    
    When done, the instrument "playmatches" is scheduled. This plays a descending oscillator
    which is pitch tracked and then a similar pitch is found from the database, prompting the "segment" 
    instrument to be scheduled with the relevant offset.
    The result is that the piano notes/sound file segments picked should moreorless match what the 
    oscillator is doing.

*/

sr = 44100
kr = 4410
nchnls = 2
0dbfs = 1


; create an in-memory sqlite database and create a table
gidb  dbconnect "sqlite", ":memory:"
dbexec gidb, "CREATE TABLE pitches (time REAL, cps REAL)"

; file of ascending piano pitches
gSfile = "sounds/piano.wav"


; detect pitches and insert time/cps to database given a filename, in non-realtime
opcode getpitches, 0, S
    Sfile xin
    ktimek timeinstk
    ktime timeinsts
    klast init -1
    kcount init 0

    ; run in the first single k-rate cycle
    if (ktimek == 0) then

        ; get file length in k-cycles
        ilen filelen Sfile
        kcycles = ilen * kr
loop:        
        ; read file and track pitch
        ain diskin2 Sfile, 1
        koct, kamp pitch ain, 0.01, 7, 11, 6, 12, 10, 8

        ; only take action if pitch has changed
        kchanged changed2 koct
        if (kchanged == 1) then

            ; only store if cps is reasonably different from the last value
            kcps = cpsoct(koct)
            if (kcps > klast*1.1 || kcps < klast*0.9) then

                ; insert to database: the dbexec_kb opcode is k-rate but blocking/synchronous. 
                ; simpler to use for the non-realtime operation as regular _k opcodes are threaded/asynchronous
                ktime = kcount / kr
                Squery sprintfk "INSERT INTO pitches (time, cps) VALUES (%f, %f)", ktime, kcps
                dbexec_kb gidb, Squery
                klast = kcps
            endif
        endif
        loop_lt kcount, 1, kcycles, loop
        
    endif
endop



; begin the example: find pitches and then schedule the next step
instr start_example
    getpitches gSfile
    schedkwhen 1, 0, 0, "playmatches", 0, 20
    turnoff

endin



; pitch follow a descending oscillator and attempt to find a similar cps in the database, then schedule segment accordingly
instr playmatches
    kdone init 0    ; for when the select query is done
    klast init 0    ; last pitch played, in order to avoid repeats

    ; descending oscillator
    k1 linseg 2000, p3, 200
    ain oscil 1, k1, 1

    ; track it
    koct, kamp pitch ain, 0.01, 7, 11, 6, 12, 10, 8

    ; only take action when the tracked pitch has changed
    kchanged changed2 koct
    if (kchanged == 1) then

        ; (very roughly) get a near frequency match from the database
        kcps = cpsoct(koct)
        Squery sprintfk "SELECT time FROM pitches WHERE cps >= %f LIMIT 1", kcps
        kdone, kpos dbscalar_k gidb, Squery, kchanged

        ; don't repeat notes (try the schedkwhen outside of the if block for continuous play)
        if (kpos != klast) then
            schedkwhen kdone, 0, 0, "segment", 0, 0.2, kpos
            klast = kpos
        endif
    endif
    
    ; uncomment to hear the oscillator as well as pitch matched output
    ;outs ain*0.01, ain*0.01
endin



; play part of the sound file given a skip time with a basic fade in/out
instr segment
    iskip = p4
    kamp linseg 0, p3*0.1, 1, p3*0.8, 1, p3*0.1, 0
    a1 diskin2 gSfile, 1, iskip
    aout = a1*kamp * 0.1
    outs aout, aout
endin


</CsInstruments>
<CsScore>
f1 0 16384 10 1 0 0.3 0 0.2 0 0.14 0 .111 ; square wave

i"start_example" 0 1

</CsScore>
</CsoundSynthesizer>