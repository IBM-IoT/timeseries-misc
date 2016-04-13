
select * from (
SELECT  TSTAMP as t1, tstamp::datetime year to second as t2, SIGNALVALUE_ID
--COL1,COL2,COL3 
FROM SIGNALVALUE_VTI
        WHERE TSTAMP >= '2015-05-15 17:00:00.00000' 
                AND TSTAMP <= '2015-05-15 17:01:00.00000')


group by t1,signalvalue_id

