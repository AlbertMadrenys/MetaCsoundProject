<CsoundSynthesizer>
<CsOptions>
</CsOptions>
<CsInstruments>
nchnls = 1
0dbfs = 1

instr 1
	ain in
	
	kcutfreq expseg 100, 2, 2000, 1, 2000
	alp butterlp ain, kcutfreq
	
	out alp
endin

schedule(1, 0, -1);

</CsInstruments>
<CsScore>
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
