#Yunfan Li
#liyunf@onid.oregonstate.edu
#CS344-001
#Homework #4
#No References
import re
import os
import pwd
import sys
import stat
import getopt
import getpass

PROC_DIR = '/proc'

help_s = 0
sum_s = 0
thread_s = 0
zombie_s = 0
uname_s = 0
uid_s = 0

_uname = getpass.getuser()
_uid = os.geteuid()

p_cnt = 0
z_cnt = 0

try:
    opts, args = getopt.getopt(sys.argv[1:], 'hTZSU:u:')
except getopt.GetoptError as err:
    help_s = 1
    print sys.argv[0]+': invalid option -- \''+err.opt+'\''
    print '*** Unrecognized command line option \''+sys.argv[0]+' -'+err.opt+'\' ***'
    print sys.argv[0]+': command line options: hTZSUu'
    print '  -h:  show help and exit'
    print '  -S:  show summary counts only'
    print '  -T:  show thread count'
    print '  -Z:  show only zombie processes'
    print '  -U <user_name>:  show <user_name>\'s processes'
    print '  -u <uid>: show <uid>\'s processes'
    print ''
    print '$Id: user_procs.py,v 1.8 2015/02/27 04:40:08 Voldy Exp liyunf $'
    sys.exit(2);    

for opt in opts:
    if opt[0] == '-h':
        help_s = 1;
    elif opt[0] == '-S':
        sum_s = 1;
    elif opt[0] == '-T':
        thread_s = 1;
    elif opt[0] == '-Z':
        zombie_s = 1;
    elif opt[0] == '-U':
        uname_s = 1;
        _uname = opt[1];
    elif opt[0] == '-u':
        uid_s = 1;
        _uid = opt[1];

if help_s == 1:
    print sys.argv[0]+': command line options: hTZSUu'
    print '  -h:  show help and exit'
    print '  -S:  show summary counts only'
    print '  -T:  show thread count'
    print '  -Z:  show only zombie processes'
    print '  -U <user_name>:  show <user_name>\'s processes'
    print '  -u <uid>: show <uid>\'s processes'
    print ''
    print '$Id: user_procs.py,v 1.8 2015/02/27 04:40:08 Voldy Exp liyunf $'
    sys.exit(0);

if uid_s == 1:
    _uname  = pwd.getpwuid(int(_uid)).pw_name
    
if uname_s == 1:
    _uid = pwd.getpwnam(_uname).pw_uid
    
print 'Showing processes for user: %-10s'%_uname+'  uid: %-8s'%str(_uid)

files = os.listdir(PROC_DIR)

if len(files):
    for f in files:
        l = re.findall(r'[0-9]', f)
        if len(l):
            tmpf = PROC_DIR+'/'+f
            fstat = os.stat(tmpf)
            if str(fstat[stat.ST_UID]) == str(_uid):
                p_cnt+=1;
                tmpf = tmpf+'/status'
                tf = open(tmpf)
                fl = tf.readline()
                if not sum_s:
                    if not zombie_s:
                        print 'Process id: %8s'%f+'   Command: '+fl[fl.index('\t')+1:len(fl)-1]
                fl = tf.readline()
                if 'zombie' in fl:
                    if not sum_s:
                        if zombie_s:
                            s = fl[fl.index('\t')+1:fl.index('\t')+2]
                            s = s.lower()
                            print 'Process id: %8s'%f+'   Command: '+s
                        print '   ****  THIS PROCESS IS A ZOMBIE  ****'
                    fl = tf.readline()
                    fl = tf.readline()
                    fl = tf.readline()
                    if not sum_s:
                        print '   ****  You need to kill its parent process '+fl[fl.index('\t')+1:len(fl)-1]+'  ****'
                    z_cnt+=1
                if not sum_s and thread_s:
                    while 1:
                        fl = tf.readline()
                        if len(fl) == 0:
                            break
                        if 'Threads:' in fl:
                            print '  Thread count: '+fl[fl.index('\t')+1:len(fl)-1]
                            break
else:
    print 'could not open /proc'
    print 'ERROR: could not open /proc directory'
    sys.exit(2)

print ''
print 'You have '+str(p_cnt)+' processes running'
if z_cnt > 0:
    print ' **  You have '+str(z_cnt)+' zombie processes. **\a'
