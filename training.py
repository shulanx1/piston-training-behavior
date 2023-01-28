# -*- coding: utf-8 -*-
"""
Created on Fri Sep 23 12:12:00 2022

@author: dalan


tasks:
    0: just record running & licking behavior, reward given at licking
    1: GO whisker stim only, reward is given regardless
    2: GO whisker stim, reward only given under correct detection
    3: NOGO whisker stim, timeout (10s) given if FA
    
Argv[4]:
    0: training phase 0 (added), GO stim only, water delivered only when lick
    1: training phase 1, just record running & licking behavior
    2: training phase 2, GOwhisker stim, water provided when stim is given
    3: training phase 3, both whisker stim (50%), reward only with correct detection
    4: testing valve, 5 release of water with 1s each
    5: testing piston
"""


import sys
import serial
import csv
import time
from threading import Thread
import numpy as np
from numpy.random import RandomState
from scipy.stats import uniform
from pathlib import Path
from playsound import playsound

from datetime import datetime

#-----------------------------------change to GO trial portion here---------------------------------
phase0_GO_portion = 0.5
phase3_GO_portion = 0.8
opto_GO_portion = 0.7
#---------------------------------------------------------------------------------------------------

"""
def phase0(ser):
    try:
        while True:
            ser.write(bytes('2', 'utf-8'))
            tsleep = uniform.rvs(size = 1)*10+10
            time.sleep(int(tsleep))
    except KeyboardInterrupt:
        ser.flush()
        ser.close()
"""

def phase2(ser):
    try:
        while True:
            ser.write(bytes('1', 'utf-8'))
            tsleep = uniform.rvs(size = 1)*10+10
            time.sleep(int(tsleep))
    except KeyboardInterrupt:
        ser.flush()
        ser.close()
    
def phase0(ser):
    try:
        while True:
            uniform.random_state = RandomState(seed = None)
            if uniform.rvs(size = 1) <= phase0_GO_portion:
                ser.write(bytes('1', 'utf-8'))
            else:
                ser.write(bytes('7', 'utf-8'))
            tsleep = uniform.rvs(size = 1)*10+10
            time.sleep(int(tsleep))
    except KeyboardInterrupt:
        ser.flush()
        ser.close()
        
def phase3(ser):
    try:
        while True:
            uniform.random_state = RandomState(seed = None)
            if uniform.rvs(size = 1) <= phase3_GO_portion:
                ser.write(bytes('2', 'utf-8'))
            else:
                ser.write(bytes('3', 'utf-8'))
            tsleep = uniform.rvs(size = 1)*10+10
            time.sleep(int(tsleep))
    except KeyboardInterrupt:
        ser.flush()
        ser.close()

def phase3_opto(ser):
    try:
        while True:
            uniform.random_state = RandomState(seed = None)
            a = uniform.rvs(size = 2)
            if (a[0] <= opto_GO_portion) and (a[1]<=0.5):
                ser.write(bytes('2', 'utf-8'))
            elif (a[0] > opto_GO_portion) and (a[1]<=0.5):
                ser.write(bytes('3', 'utf-8'))
            elif (a[0] <= opto_GO_portion) and (a[1]>0.5):
                ser.write(bytes('8', 'utf-8'))
            elif (a[0] > opto_GO_portion) and (a[1]>0.5):
                ser.write(bytes('9', 'utf-8'))
            tsleep = uniform.rvs(size = 1)*10+10
            time.sleep(int(tsleep))
    except KeyboardInterrupt:
        ser.flush()
        ser.close()
        

    
def test_valve(ser):
    ser.write(bytes('4', 'utf-8'))
    time.sleep(8)
    
def test_piston(ser):
    ser.write(bytes('5', 'utf-8'))
    time.sleep(11)
    
def play_white_noise(audio):
    while True:
        try:
            playsound(audio)
        except KeyboardInterrupt:
            break

    
count = 0
data = []
audio = Path().cwd() / "white_gaussian_noise.wav"
timestamp = datetime.now().strftime("%Y_%m_%d_%I.%M.%p")
# background_noise_task = Thread(target = play_white_noise, args = (audio, ))
# background_noise_task.start()
if int(sys.argv[3]) == 1:
    with open(sys.argv[2]+"_phase1_"+timestamp+".csv", "w") as f:
        with serial.Serial(port=sys.argv[1], baudrate=9600,timeout=1) as ser:
            ser.flush()
            try:
                while True:
                    if ser.in_waiting > 0:
                        ser.write(bytes('0', 'utf-8'))
                        decoded_bytes= ser.readline().decode('utf-8').rstrip()
                        print(decoded_bytes)
                        try:
                            writer = csv.writer(f)
                            writer.writerow([decoded_bytes])
                            print(decoded_bytes)
                            count = count+1
                            f.flush()
                        except:
                            f.flush()
                            f.close()
                            continue
            except KeyboardInterrupt:
                print('interrupt\n')
                f.flush()
                f.close()
            finally:                
                ser.flush()
                ser.close()
                # background_noise_task.join()
elif int(sys.argv[3]) == 2:
    with open(sys.argv[2]+"_phase2_"+timestamp+".csv", "w") as f:
        with serial.Serial(port=sys.argv[1], baudrate=9600,timeout=1) as ser:
            ser.flush()
            task = Thread(target = phase2, args = (ser, ))
            task.start()
            try:
                while True:
                    if ser.in_waiting > 0:
                        decoded_bytes= ser.readline().decode('utf-8').rstrip()
                        try:
                            writer = csv.writer(f)
                            writer.writerow([decoded_bytes])
                            print(decoded_bytes)
                            count = count+1
                            f.flush()
                        except:
                            
                            f.flush()
                            f.close()
                            continue
            except KeyboardInterrupt:
                print('interrupt\n')
                f.flush()
                f.close()
            finally:      
                task.join() 
                # background_noise_task.join()
                ser.flush()
                ser.close()
elif int(sys.argv[3]) == 0:
    with open(sys.argv[2]+"_phase0_"+timestamp+".csv", "w") as f:
        with serial.Serial(port=sys.argv[1], baudrate=9600,timeout=1) as ser:
            ser.flush()
            task = Thread(target = phase0, args = (ser, ))
            task.start()
            try:
                while True:
                    if ser.in_waiting > 0:
                        decoded_bytes= ser.readline().decode('utf-8').rstrip()
                        try:
                            writer = csv.writer(f)
                            writer.writerow([decoded_bytes])
                            print(decoded_bytes)
                            count = count+1
                            f.flush()
                        except:
                            
                            f.flush()
                            f.close()
                            continue
            except KeyboardInterrupt:
                print('interrupt\n')
                f.flush()
                f.close()
            finally:      
                task.join() 
                # background_noise_task.join()
                ser.flush()
                ser.close()
elif int(sys.argv[3]) == 3:
    with open(sys.argv[2]+"_phase3_"+timestamp+".csv", "w") as f:
        with serial.Serial(port=sys.argv[1], baudrate=9600,timeout=1) as ser:
            ser.flush()
            task = Thread(target = phase3, args = (ser, ))
            task.start()
            try:
                while True:
                    if ser.in_waiting > 0:
                        decoded_bytes= ser.readline().decode('utf-8').rstrip()
                        try:
                            writer = csv.writer(f)
                            writer.writerow([decoded_bytes])
                            print(decoded_bytes)
                            count = count+1
                            f.flush()
                        except:
                            f.flush()
                            f.close()
                            continue
            except KeyboardInterrupt:
                print('interrupt\n')
                f.flush()
                f.close()
            finally:
                # background_noise_task.join()
                task.join()
                ser.flush()
                ser.close()
elif int(sys.argv[3]) == 4:
    with open(sys.argv[2]+"_phase3_opto"+timestamp+".csv", "w") as f:
        with serial.Serial(port=sys.argv[1], baudrate=9600,timeout=1) as ser:
            ser.flush()
            task = Thread(target = phase3_opto, args = (ser, ))
            task.start()
            try:
                while True:
                    if ser.in_waiting > 0:
                        decoded_bytes= ser.readline().decode('utf-8').rstrip()
                        try:
                            writer = csv.writer(f)
                            writer.writerow([decoded_bytes])
                            print(decoded_bytes)
                            count = count+1
                            f.flush()
                        except:
                            f.flush()
                            f.close()
                            continue
            except KeyboardInterrupt:
                print('interrupt\n')
                f.flush()
                f.close()
            finally:
                # background_noise_task.join()
                task.join()
                ser.flush()
                ser.close()
                """
elif int(sys.argv[3]) == 4:
    with serial.Serial(port=sys.argv[1], baudrate=9600,timeout=1) as ser:
        while True:
            if ser.in_waiting > 0:
                print("start valve testing /n")
                ser.flush()
                test_valve(ser)
                ser.flush()
                ser.close()
                print("finishing valve testing /n")
                break
            else:
                print("serial port not open /n")
                time.sleep(0.5)

elif int(sys.argv[3]) == 5:
    with serial.Serial(port=sys.argv[1], baudrate=9600,timeout=1) as ser:
        while True:
            if ser.in_waiting > 0:
                print("start piston testing /n")
                ser.flush()
                test_piston(ser)
                ser.flush()
                ser.close()
                print("finishing piston testing /n")
                break
            else:
                print("serial port not open /n")
                time.sleep(0.5)
                """
     
else:
    raise ValueError('Wrong values for the phase, choose between 0, 1 and 2')

    
    
    