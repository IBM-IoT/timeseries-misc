#!/usr/bin/python

import sys

if len(sys.argv) != 8:
   print "Usage: %s <hz> <hr start> <hr stop> <min start> <min stop> <start id> <stop id> " % (sys.argv[0])
   exit(0) 

p_hz = int(sys.argv[1]);
p_hr_start = int(sys.argv[2]);
p_hr_stop  = int(sys.argv[3]);
p_min_start = int(sys.argv[4]);
p_min_stop = int(sys.argv[5]);
p_id_start = int(sys.argv[6]);
p_id_stop = int(sys.argv[7]);


for hr in range( p_hr_start , p_hr_stop + 1 ): 
  for min in range( p_min_start, p_min_stop + 1 ):
   #print "begin work;"

   for sec in range(0, 60):
     for id in range ( p_id_start , p_id_stop +1 ):
        for ms in range ( 0, p_hz ):
          val = "%.5f" %( ms / float(p_hz) )

          ## Print load for for use the TSloader (tsl_put)
          ##
          print "%s|2015-01-01 %02d:%02d:%02d.%s|1.1|2.2|4|" % ( id ,hr ,min ,sec ,val[2:8] )

   #print "commit work;"
        
        


