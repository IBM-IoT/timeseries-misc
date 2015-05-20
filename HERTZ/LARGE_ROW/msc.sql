-- execute function tsl_init ('pmu_large','data');

--execute function tsl_attach ('pmu_large', 'data');
--execute function tsl_put ('pmu_large|data', 'FILE:/home/informix/WORK/timeseries-misc/HERTZ/LARGE_ROW/ins1.unl')


-- Add tsl_flushall
-- begin work;
-- execute function tsl_flushall ('pmu_large|data');
-- commit work;

--execute function tsl_commit ('pmu_large|data', 1 , 3600);


--execute function tsl_sessionclose ('pmu_large|data');
