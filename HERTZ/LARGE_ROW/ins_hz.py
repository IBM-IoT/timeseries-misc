import sys

if len(sys.argv) != 3:
   print "Usage: %s <hz> <id> " % (sys.argv[0])
   exit(0) 

for hr in range(8,12): 
  for min in range(0,60):
   #print "begin work;"

   for sec in range(0, 60):
      for ms in range (0, int(sys.argv[1]) ):
         val = "%.5f" %(ms/ float(sys.argv[1]) )

         ## Print load for for use the TSloader (tsl_put)
         ##
         print "%s|2015-01-01 %02d:%02d:%02d.%s|1.1|2.2|4" % (sys.argv[2],hr,min,sec,val[2:8])

   #print "commit work;"



