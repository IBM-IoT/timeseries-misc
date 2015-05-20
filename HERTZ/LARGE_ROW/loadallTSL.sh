dbaccess epg <<!
execute function tsl_init("pmu_large", "data");
!

FILELIST=`ls *.unl`;

for f in $FILELIST
do

./TSL_FileLoad  epg pmu_large data /home/informix/WORK/timeseries-misc/HERTZ/LARGE_ROW/$f &
done


wait
