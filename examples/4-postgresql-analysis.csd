<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>
<CsInstruments>
/*
	EXAMPLE 4

	Pitch track an oscillator, inserting the time, frequency and amplitude values in a PostgreSQL database 
	as fast as it will accept, at k-rate.
	Read back the values at i-rate in the db_play instrument and create events to mimic the original oscillator.
	

*/

sr = 44100
kr = 4410
nchnls = 2
0dbfs = 1
seed 0


; connect: type, hostname or IP, database name, username, password
gidb dbconnect "postgresql", "localhost", "databasename", "username", "password"


; create table if it doesn't exist. truncate if it does
instr start_example
	kdone1 dbexec_k gidb, "CREATE TABLE IF NOT EXISTS analysis (time FLOAT, rms FLOAT, freq FLOAT)", -1
	kdone2 dbexec_k gidb, "TRUNCATE TABLE analysis", kdone1

	; schedule next steps
	if (kdone2 == 1) then
		schedule "db_analyse", 0, 3
		schedule "db_play", 3.5, 3.5
		turnoff
	endif
endin


instr db_analyse
	
	; the oscillator
	a1 oscil abs(oscil(1, 0.6)), linseg(440, p3*0.2, 800, p3*0.2, 220, p3*0.2, 220, p3*0.2, 350, p3*0.2, 440)

	; pitch/rms and time tracking
	kcps, krms pitchamdf a1, 220, 800, 0, 0
	ktime timeinsts 

	; listen back to the pitch track output
	a1 oscil krms, kcps

	; insert the time, rms and frequency values as fast as the database will accept
	kdone init 1
	kdone dbexec_k gidb, sprintfk("INSERT INTO analysis (time, rms, freq) VALUES (%f, %f, %f)", ktime, krms, kcps), 1
	
	; declick end
	aout = a1 * linseg(1, p3*0.99, 1, p3*0.01, 0)
	outs aout, aout
endin


instr db_play

	; get all of the values
	idata[][] dbarray gidb, "SELECT time, rms, freq FROM analysis"
	
	; loop through 
	index = 0
	while (index < lenarray(idata)) do
	
		; set the duration to be the time to the next row multiplied by an overlap
		if (index == lenarray(idata) - 1) then
			iduration = 1
		else	
			iduration = (idata[index+1][0] - idata[index][0]) * 3.2
		endif
	
		; create the event accordingly
		event_i "i", "oscillator", idata[index][0]*2, iduration, idata[index][1], idata[index][2]
		index += 1
	od
endin


instr oscillator

	; basic oscillator
	iamp = p4
	ifreq = p5
	a1 oscil iamp * 0.5, ifreq
	
	; envelope to try and take away the quantised type sound
	aout = a1 * linseg(0, p3*0.45, 1, p3*0.1, 1, p3*0.45, 0)
	outs aout, aout
endin

</CsInstruments>
<CsScore>
i "start_example" 0 7


</CsScore>
</CsoundSynthesizer>