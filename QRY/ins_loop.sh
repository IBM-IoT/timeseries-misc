for min in {0..59}
do
   lmin=`printf "%02d" $min`
   for sec in {0..59}
   do
      lsec=`printf "%02d" $sec`
      for ms in {1..10}
      do
         lms=`printf "%05d" $ms`
         echo "insert into sensors_rel values('id of some sort',
                   '2017-07-18 10:$lmin:$lsec.$lms');
      done
   done
done
