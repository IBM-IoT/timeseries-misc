-- File:  
-- auth: Darin Tracy
-- date: 2015-05-26
-- desc: Pass in a timestamp value and get the previous seconds 
--       worth of hertz data 
--
--   in: Timestamp value of last inserted data
--  out: a seconds worth of data for all phasors
--
-- Mod   Date        Desc
-- 001   2015-05-26  Initial Draft


drop procedure if exists getcurrentseconddata( lvarchar );
create procedure getcurrentseconddata( where_clause lvarchar ) returning char(100), datetime year to fraction(5), float, float;

   define l_tss char(30);  
   define l_start char(30);  
   define l_stop char(30);  
   define l_id char(100);
   define l_ts datetime year to fraction(5);
   define l_f1 float;
   define l_f2 float;
   define l_sql lvarchar;

  

--set debug file to "/tmp/sp2.out";
--trace on;

set isolation to dirty read;
   select first 1 getpreviousvalid(data,
               getlastelem(data).tstamp:: 
               datetime year to second).tstamp::
               datetime year to second
 into l_tss 
from signalvalue where signalvalue_id="CALIFORNIA1.CALIFORNIA1_IM";

   let l_start = trim(l_tss) || ".00000";
   let l_stop  = trim(l_tss) || ".99999";

let l_sql = "select signalvalue_id,tstamp,col1,col2 from signalvalue_vti where tstamp >= '" || trim(l_start) || "'::datetime year to fraction(5) and tstamp <= '" || trim(l_stop) || "'::datetime year to fraction(5) and  " || where_clause ;


prepare stmt_sql from l_sql;
declare cur_sql cursor for stmt_sql;
open cur_sql;
while (SQLCODE != 100)
   fetch cur_sql into l_id, l_ts, l_f1, l_f2;
   return l_id,l_ts,l_f1,l_f2 with resume; 
end while;




end procedure; 
