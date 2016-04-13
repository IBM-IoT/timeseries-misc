{
select 
   signalvalue_id,
   getlastelem(data).col1::int ,
   getlastelem(data).col2 
  from
signalvalue;


select * from
   signalvalue_vti where
   tstamp >= "2015-05-15 16:30:00.00000" and
   tstamp <= "2015-05-15 16:35:00.00000" and

signalvalue_id in ("CALIFORNIA1.CALIFORNIA1_IM", "CALIFORNIA1.CALIFORNIA1_VM")

}



select 
    t.tstamp
from table (transpose
  ((select clip( data,
             "2015-05-15 16:30:00 00:00:00.00000",
             "2015-05-15 16:35:00 00:00:00.00000")
    from signalvalue where
     signalvalue_id = "CALIFORNIA1.CALIFORNIA1_IM")):: signalvalue_row_t 
   as tab(t);


