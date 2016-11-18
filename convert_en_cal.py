#!/usr/bin/python
import sys

if len(sys.argv)!=4:
   print "usage: <peak_pos> <peak-index> <peak-file>"

peak_pos=float(sys.argv[1])
peak_idx=int(sys.argv[2])
peak_file=sys.argv[3]

fi=open(peak_file)

for febex in range(9):
    for ch in range(16):
        l=fi.readline().split(" ")
        d=(float(peak_pos)/int(l[peak_idx]))
        print "%x\t%x\t%x\t%f*x"%(0,febex, ch, d)

