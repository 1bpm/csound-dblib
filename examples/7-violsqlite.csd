<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>
<CsInstruments>
/*
    EXAMPLE 7

    A rudimentary pitch tracking concatenative resynthesizer as such

    This creates a sqlite database in memory, and then uses the opcode getpitches to scan through a 
    sound file in non-realtime, storing detected pitches and the relevant offset time of the pitch in 
    the database. The example file is a descending violin glissando.
    
    When done, the instrument "playmatches" is scheduled twice which plays an oscillator varying in pitch,
    which is pitch tracked and the nearest frequency is found in the database, prompting the "segment"
    instrument to be scheduled with the relevant offset.


	1. for 20s in which nearest matches are played only if the detected pitch has changed outside of a threshold 
	2. as above but with matches played continuously 

    The result is that the violin segments picked should moreorless match what the oscillator is doing.

*/

sr = 44100
kr = 4410
nchnls = 2
0dbfs = 1


; create an in-memory sqlite database and create a table
gidb  dbconnect "sqlite", ":memory:"
dbexec gidb, "CREATE TABLE pitches (time REAL, cps REAL)"

; file of ascending piano pitches
gSfile = "sounds/violin.wav"
gifn ftgen 0, 0, 0, 1, gSfile, 0, 0, 0

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
	;ain butterbp ain, 500, 250
        koct, kamp pitch ain, 0.01, 6, 12, 6, 12, 60
	
        ; only take action if pitch has changed
        kchanged changed2 koct
        if (kchanged == 1) then
		
            ; only store if cps is reasonably different from the last value
            kcps = cpsoct(koct)

            if (1==1) then

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
    schedkwhen 1, 0, 0, "playmatches", 0, 20, 0
    schedkwhen 1, 0, 0, "playmatches", 20, 20, 1
    turnoff

endin



; pitch follow a descending oscillator and attempt to find a similar cps in the database, then schedule segment accordingly
instr playmatches
    kdone init 0    ; for when the select query is done
    klast init 0    ; last pitch played, in order to avoid repeats


    ; oscillator
    k1 linseg 400, p3, 200
    ktime linseg 0.001, p3, 2
    k2 oscil k1, ktime
    ain oscil 1, abs(k2)+550, 1

    ; track it
    koct, kamp pitch ain, 0.01, 5, 10, 6, 12
	
    ; only take action when the tracked pitch has changed
    kchanged changed2 koct
    if (kchanged == 1) then

        ; (very roughly) get a near frequency match from the database
        kcps = cpsoct(koct)
        
        ; fairly nasty looking query for obtaining the nearest value
        SquerySource = {{
            SELECT time FROM (
                SELECT time, cps FROM (
                    SELECT time, cps FROM pitches WHERE cps >= %f ORDER BY cps ASC LIMIT 1
                ) 
                UNION SELECT time, cps FROM (
                    SELECT time, cps FROM pitches WHERE cps < %f ORDER BY cps DESC LIMIT 1
                )
            ) x ORDER BY ABS(cps - %f) ASC LIMIT 1
        }}

        Squery sprintfk SquerySource, kcps, kcps, kcps
        kdone, kpos dbscalar_k gidb, Squery, kchanged

        ;schedule the notes
        if (p4 == 1) then
            schedkwhen kdone, 0, 0, "segment", 0, 0.2, kpos        
        else
            if (kpos < klast*0.8 || kpos > klast*1.2) then
                schedkwhen kdone, 0, 0, "segment", 0, 0.2, kpos
                klast = kpos
            endif
        endif
    endif

   
    ; uncomment to hear the oscillator as well as pitch matched output
    ;outs ain*0.01, ain*0.01
endin



; play part of the sound file given a skip time with a basic envelope
instr segment
    il = ftlen(gifn)
    isec = il/sr         
    ist = sr*p4
    icps = 1/isec  
    aphs phasor icps      
    andx = aphs * il
    aout tablei andx+ist, gifn
    kamp linseg 0, p3*0.3, 1, p3*0.4, 1, p3*0.3, 0
    outs aout*0.1*kamp, aout*0.1*kamp
endin


</CsInstruments>
<CsScore>
f1 0 16384 10 1 0 0.3 0 0.2 0 0.14 0 .111 ; square wave

i"start_example" 0 1

</CsScore>
</CsoundSynthesizer>