dbaccess epg <<!
execute function tsl_init("pmu_large", "data");
!

./TSL_FileLoad  epg pmu_large data /home/informix/WORK/timeseries-misc/HERTZ/LARGE_ROW/ins1.unl &
done


