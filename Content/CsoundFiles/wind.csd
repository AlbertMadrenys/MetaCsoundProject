<CsoundSynthesizer>
<CsOptions>
</CsOptions>
<CsInstruments>
nchnls = 2
0dbfs = 1

chn_k "In Control 0", 1

instr 1
	kamp chnget "In Control 0"
	
	kamp = (kamp*4)+0.5
	
	apink1 pinker
	apink2 pinker
	kcutfreq = oscili(30, 0.1) + 50 ;between 20 and 80
	
	kport port kamp, 0.05
	
	atone1 tone apink1, kcutfreq
	atone2 tone apink2, kcutfreq
	
	outs atone1*kport, atone2*kport
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
