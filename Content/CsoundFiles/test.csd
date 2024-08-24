<CsoundSynthesizer>
<CsOptions>
--sample-rate=41100
</CsOptions>
<CsInstruments>
nchnls = 2
nchnls_i = 2
0dbfs = 1

chn_k "In Control 0", 1
chn_k "In Control 1", 1
chn_k "Out Control 0", 2
chn_k "Out Control 1", 2

instr 1
 	ain0, ain1 inch 1, 2		;access input signal from processing loop

	aosc0 = oscili(0.5, 1)+0.5
	aosc1 = oscili(0.5, 1, -1, 0.5)+0.5
	
	kamp0 chnget "In Control 0"
	kamp1 chnget "In Control 1"
	
	chnset kamp0, "Out Control 0"
	chnset kamp1, "Out Control 1"
	
	out ain0*aosc0*(kamp0), ain1*aosc1*0.2*kamp1		;multiply output of iscil by incoming signal
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
