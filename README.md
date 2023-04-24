# README #

* Behavioral training set up for two whisker discrimination task based on pneumetic whisker stimulation

### Python environment Requirement: ###
* numpy
* scipy
* playsound

### Hardware on the circuit board: ###
* 24 V piston air pressure regulator: https://www.smcpneumatics.com/V114T-5LZ-M5.html
* 24 VDC power relay: digikey T1258-5-ND
* dual channel diod: digikey 1655-BAT54CCT-ND
* dual channel NTN transister: digikey MBT3904DW1T1G
* 12 V water valve: Nresearch 161K011
* 12 V water valve driver: Nresearch 161D1X250
* 5V speaker (with amplifier): digikey 1528-1746-ND
* vibration motor: 3V
* rotary encoder: https://www.usdigital.com/products/encoders/incremental/rotary/shaft/H5


### Files ###

* //BehavioralPCB: arduino code to upload to arduino uno
* //Bonsai lick detection: real time lick detection with webcam when capactive based lick sensor causes electrical artifact
* //training.py: training file to serial communicate with arduino
* //behavior_training_decode: matlab file to decode the serial output of arduino


### To start the training from command window ###
* cd path-to-base-folder
* python training.py COM12 ear-tag-number phase

### Training phase ###
* 1: no whisker stim, reward dispense when the lick is detected
* 2: only GO stim, reward is given when the GO stim is presented
* 0: only GO stim, reward only when "hit"
* 4: GO and NOGO stim, with teaching signal 
* 3: GO and NOGO stim, reward is given at "hit", time out at false alarm
* 5: optogenetic stimulation in randomly chosen half of the trials,  reward is given at "hit", time  out at false alarm

### Arduino test serial command ###
* '4': test valve
* '5': test pistons
* '6': test cue sound

### Arduino serial output row ###
* 1: rotary decoder
* 2: licking
* 3: valve
* 4: GO piston
* 5: NOGO piston
* 6: GO previous performance (16 bits, 1st digit: teaching signal, last digit: licking behavior of the most recent trial)
* 7: NOGO previous performance
* 8: time