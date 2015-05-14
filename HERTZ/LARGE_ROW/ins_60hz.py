import sys

if len(sys.argv) != 2:
   print "Usage: %s <id> " % (sys.argv[0])
   exit(0) 
 
for min in range(0,59):
   print "begin work;"
   for sec in range(0, 59):
      for ms in range (0,59):
         val = "%.5f" %(ms/60.0)
         print "insert into pmu_large_vti values(%s,'2015-01-01 08:%02d:%02d.%s',1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.10,11.11,12.12,13.13,14.14,15.15);" % (sys.argv[1],min,sec,val[2:8])
   print "commit work;"



