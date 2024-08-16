<CsoundSynthesizer>
<CsOptions>
--sample-rate=41100
</CsOptions>
<CsInstruments>
nchnls = 2
nchnls_i = 2
0dbfs=1

// THIS FILE IS THE **** REALLY ***** GOOD ONE

chn_k "InControl0", 1 // Can read and write on the same control channel
chn_k "InControl1", 1
chn_k "OutControl0", 2
chn_k "OutControl1", 2

instr 1
 	ain1, ain2 inch 1, 2		;access input signal from processing loop
 	//ain1 inch 1
 	//ain oscili 0.2, 440 
	aosc1 = oscili(0.5, 1)+0.5
	aosc2 = oscili(0.5, 1, -1, 0.5)+0.5
	
	kamp chnget "InControl0"
	kamp2 chnget "InControl1"
	//kamp init 0.5
	//kamp2 init 0.5
	
	chnset kamp, "OutControl0"
	chnset kamp2, "OutControl1"
	
	out ain1*aosc1*(kamp), ain2*aosc2*0.2*kamp2		;multiply output of iscil by incoming signal
endin

instr 2
pset 2, 0, 1, 220
aout vco2 0.05, p4
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
