-- Example to get previous second TS
-- Used to get previous and select a full second (hz) range of data
--
{
select first 1
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".00000")::
               lvarchar::datetime year to fraction(5),
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".99999")::
               lvarchar::datetime year to fraction(5)
from signalvalue;
}

--  Returns a Timeseries for each signavalue_id
--
{
select signalvalue_id,
   apply(  '$col1,,$col2,$col3', 
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".00000")::
               lvarchar::datetime year to fraction(5),
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".9999")::
               lvarchar::datetime year to fraction(5),
           data
        )::Timeseries(signalvalue_row_t)
from signalvalue
   where signalvalue_id in ( "CALIFORNIA1.CALIFORNIA1_IM", 
                             "CALIFORNIA1.CALIFORNIA1_VM");
}


--
--
select * from signalvalue_vti

where 
     tstamp >= (select 
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".00000")::
               lvarchar::datetime year to fraction(5) from signalvalue
               where signalvalue_id in ("CALIFORNIA1.CALIFORNIA_IM"))
AND
     tstamp <= (select 
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".99999")::
               lvarchar::datetime year to fraction(5) from signalvalue
               where signalvalue_id in ("CALIFORNIA1.CALIFORNIA_IM"))
;



-- Working On
--
{
select t.*

from table ( transpose ((

select 
   apply(  '$col1,$col2,$col3', 
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".00000")::
               lvarchar::datetime year to fraction(5),
           (getpreviousvalid(data,getlastelem(data).tstamp::
               datetime year to second).tstamp::
               datetime year to second||".9999")::
               lvarchar::datetime year to fraction(5),
           data
        )::Timeseries(signalvalue_row_t)
from signalvalue
   where signalvalue_id in ( "CALIFORNIA1.CALIFORNIA1_IM", 
                             "CALIFORNIA1.CALIFORNIA1_VM")

                      ))) as tab(t)
;

}


