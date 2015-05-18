#! /usr/bin/env python2.7
#Yunfan Li
#liyunf@onid.oregonstate.edu
#OPERATING SYSTEMS I (CS_344_001_W2015)
#Assignment #1
#No references

import sys, os, getopt

term = className = ''

opts, args = getopt.getopt(sys.argv[1:], "t:c:", ["term=", "class="])

for opt in opts:
    if opt[0] == "-t" or opt[0] == "--term":
        term = opt[1];
    elif opt[0] == "-c" or opt[0] == "--class":
        className = opt[1];

os.system('ln -s /usr/local/classes/eecs/'+term+'/'+className+'/README README')
os.system('ln -s /usr/local/classes/eecs/'+term+'/'+className+'/src src_class')