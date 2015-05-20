dbaccess epg <<!
execute function tsl_init("pmu", "data");
!

FILELIST=`ls *.unl`;

for f in $FILELIST
do

./TSL_FileLoad  epg pmu data /home/informix/WORK/timeseries-misc/HERTZ/SCAT/$f &
done


wait
