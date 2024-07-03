<CsoundSynthesizer>
<CsOptions>
--sample-rate=41100
</CsOptions>
<CsInstruments>
nchnls = 2
nchnls_i = 2
0dbfs=1

// THIS FILE IS THE GOOD ONE

chn_k "amp", 1
chn_k "k_1", 3 // Can read and write on the same control channel
chn_k "k_2", 3

instr 1
 	ain1, ain2 inch 1, 2		;access input signal from processing loop
 	//ain1 inch 1
 	//ain oscili 0.2, 440 
	aosc1 = oscili(0.5, 1)+0.5
	aosc2 = oscili(0.5, 1, -1, 0.5)+0.5
	
	kamp chnget "amp"
	
	out ain1*aosc1*(1-kamp), ain2*aosc2*0.2*kamp		;multiply output of iscil by incoming signal
endin

instr 2
;pset p4 440 << take a look at this, possible bug with pset?
aout vco2 0.05, 220
outs aout, aout
endin

schedule(1, 0, -1);
//schedule(2, 0, 3)

</CsInstruments>
<CsScore>
//i2 0 3
//f0 z
</CsScore>

</CsoundSynthesizer>




























































<bsbPanel>
 <label>Widgets</label>
 <objectName/>
 <x>0</x>
 <y>0</y>
 <width>0</width>
 <height>0</height>
 <visible>true</visible>
 <uuid/>
 <bgcolor mode="background">
  <r>240</r>
  <g>240</g>
  <b>240</b>
 </bgcolor>
</bsbPanel>
<bsbPresets>
</bsbPresets>
