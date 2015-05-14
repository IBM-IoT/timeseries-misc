

for min in range(0,59):
   print "begin work;"
   for sec in range(0, 59):
      for ms in range (0,59):
         val = "%.5f" %(ms/60.0)
         print "insert into pmu_vti values(8,'2015-01-01 08:%02d:%02d.%s',5,2.3);" % (min,sec,val[2:8])
   print "commit work;"



