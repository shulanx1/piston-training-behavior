# README #

* Behavioral training set up for two whisker discrimination task based on pneumetic whisker stimulation

### Python environment Requirement: ###
* numpy
* scipy
* playsound
* pyserial

### Hardware on the circuit board: ###
* 24 V piston air pressure regulator: https://www.smcpneumatics.com/V114T-5LZ-M5.html
* 24 VDC power relay: digikey 255-5324-5-ND
* dual channel diod: digikey 1655-BAT54CCT-ND
* dual channel NTN transister: digikey MBT3904DW1T1G
* 12 V water valve: Nresearch 161K011 or thevalveshop 411L3112HV https://www.thevalveshop.com/menu/auto/asco/asco-scientific.html 
* 12 V water valve driver: Nresearch 161D1X250
* 5V speaker (with amplifier): digikey 1528-1746-ND
* vibration motor: 3V
* stepper motor: Makeblock Me Stepper Motor Driver https://amicus.com.sg/shop/makeblock/me-stepper-motor-driver/
* rotary encoder: https://www.usdigital.com/products/encoders/incremental/rotary/shaft/H5
* piezo based lickometer:<br />
single-rail op-amp: TLV2771CDBVR<br />
shunt regulator: ATL431BQDBZR<br />
non-inverting svhmitt trigger: SN74LVC1G17DBVR<br />
LED: 160-1183-1-ND<br />
trimmer resistor: digikey 3362P-504LF-ND<br />
* piezo based lickometer (with rail splitter:<br />
op-amp: TLV2371IDBVR<br />
rail splitter: TLE2426QDRG4Q1 <br />
cap 200nF: 1189-2375-ND  <br />
cap 1uF: 399-ESS105M050AB2EACT-ND <br\>


### pin layout V6 ###
* D2: capacitance/piezo lick sensor
* D3: water valve
* D5, D6: rotary encoder (A,B)
* D8, D9, D11, D12: piston
* D10: sliding switch (start trigger)
* A1, A2: LED/conductance lick sensor
* A3: speaker
* D11, D7: stepper motor (STP, DIR)
* D4, D12: vibrating piezo

### pin layout V7 ###
* D2: capacitance/piezo lick sensor
* D3: water valve
* D4: sliding switch (start trigger)
* D5, D6, A2: rotary encoder (A,B,rotations)
* D8, D9, A4, A5: piston
* A1: LED/conductance lick sensor
* A3: speaker
* A5, A4: stepper motor (STP, DIR)


### Files ###

* //BehavioralPCB: arduino code to upload to arduino uno (board V6)
* //Bonsai lick detection: real time lick detection with webcam when capactive based lick sensor causes electrical artifact
* //training.py: training file to serial communicate with arduino
* //behavior_training_decode: matlab file to decode the serial output of arduino
* //Arduino_board_V6: PCB design
* //CAD: laser engraving pattern on the running disk

### Install ###
* Arduino: <br />
At sketch -> include library -> add .zip library, add the dio2.zip folder<br />

* Python: <br />
on the command window, run: <br />
-- pip install playsound <br />
-- pip install pyserial <br />

### To start the training from command window ###
-- cd path-to-base-folder <br />
-- python training.py COM12 ear-tag-number phase <br />

### Training phase ###
* 1: no whisker stim, reward dispense when the lick is detected
* 2: GO and NOGO stim, with teaching signal
* 3: GO and NOGO stim, reward is given at "hit", time out at false alarm
* 4: optogenetic stimulation in randomly chosen half of the trials,  reward is given at "hit", time  out at false alarm

### Arduino test serial command ###
* 'v': test valve
* 'p': test pistons
* 'c': test cue sound
* 'f': flush the valve
* 'g': GO piston, use the sliding switch to turn on and off
* 'n': NOGO piston, use the sliding switch to turn on and off

### Arduino serial output row ###
* 1: rotary decoder
* 2: licking
* 3: valve
* 4: GO piston
* 5: NOGO piston
* 6: hit rate
* 7: false alarm rate
* 8: time

### Arduino pin map ###
* DO2: lick sensor
* DO3: valve
* DO5&DO6: rotary encoder
* D8, D9, D12, D12: pistons
* D10: sliding switch
* D13: trigger