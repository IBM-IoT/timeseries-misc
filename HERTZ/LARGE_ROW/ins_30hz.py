import sys

if len(sys.argv) != 2:
   print "Usage: %s <id> " % (sys.argv[0])
   exit(0) 

for hr in range(8,12): 
  for min in range(0,60):
   #print "begin work;"

   for sec in range(0, 60):
      for ms in range (0,30):
         val = "%.5f" %(ms/30.0)
         ## Print insert for vti
         ##
         #print "insert into pmu_large_vti values(%s,'2015-01-01 08:%02d:%02d.%s',1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.10,11.11,12.12,13.13,14.14,15.15);" % (sys.argv[1],min,sec,val[2:8])

         ## Print load for for use the TSloader (tsl_put)
         ##
         print "%s|2015-01-01 %02d:%02d:%02d.%s|1.1|2.2|3.3|4.4|5.5|6.6|7.7|8.8|9.9|10.10|11.11|12.12|13.13|14.14|15.15" % (sys.argv[1],hr,min,sec,val[2:8])

   #print "commit work;"



