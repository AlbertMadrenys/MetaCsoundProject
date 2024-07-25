<CsoundSynthesizer>
<CsOptions>
--sample-rate=41100
</CsOptions>
<CsInstruments>
nchnls = 2
nchnls_i = 2
0dbfs=1


instr 1
 	ain1 foscil 0.5, 330, 0.5, 0.5, -1
	
	out ain1, ain1
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
