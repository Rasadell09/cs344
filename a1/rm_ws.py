#! /usr/bin/env python2.7
#Yunfan Li
#liyunf@onid.oregonstate.edu
#CS344-001
#Assignment #1
#No references
import sys, os, getopt

flag_b = flag_i = flag_e = 0

opts, args = getopt.getopt(sys.argv[1:], 'bie')

for opt in opts:
    if opt[0] == '-b':
        flag_b = 1
    elif opt[0] == '-i':
        flag_i = 1
    elif opt[0] == '-e':
        flag_e = 1

while 1:
    line = sys.stdin.readline()
    
    if line:
        if flag_b:
            i = 0
            while ord(line[i]) == 32 or ord(line[i]) == 9:
                i += 1
            line = line[i:len(line)]
        if flag_i:
            start = 0
            end = len(line)-1
            while ord(line[start]) == 32 or ord(line[start]) == 9:
                start += 1
            while ord(line[end]) == 32 or ord(line[end]) == 9:
                end -= 1
            i = start
            while i >= start and i <=end:
                while ord(line[i]) == 32 or ord(line[i]) == 9:
                    line = line[0:i] + line[i+1:len(line)]
                    end = len(line)-1
                i += 1
        if flag_e:
            end = len(line)-1
            while ord(line[end]) == 32 or ord(line[end]) == 9:
                end -= 1
            line = line[0:end+1]    
        print line,
    else:
        break
