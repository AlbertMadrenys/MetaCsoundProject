<CsoundSynthesizer>
<CsOptions>
</CsOptions>
<CsInstruments>
nchnls = 1
0dbfs = 1

chn_k "In Control 0", 1

instr 1
	kdist chnget "In Control 0"
	ain in
	kport port kdist, 0.05
	
	kcutfreq expseg 100, 2, 2000, 1, 2000
	alp butterlp ain, kcutfreq
	
	out alp
	;freverb
	
	/*
	ifftsize  = 2048
	ioverlap  = ifftsize / 4
	iwinsize  = ifftsize
	iwinshape = 1	
	
	fftin pvsanal ain, ifftsize, ioverlap, iwinsize, iwinshape
	fscal pvscale fftin, kport
	asynth pvsynth fscal
	
	//adop doppler ain, kport, 0
	out asynth
	*/
	
	
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
