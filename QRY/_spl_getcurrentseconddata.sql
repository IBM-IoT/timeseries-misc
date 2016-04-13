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


drop procedure if exists getcurrentseconddata( char(30));
create procedure getcurrentseconddata(p_ts char(30)) returning char(100), datetime year to fraction(5), float, float;

   define l_start char(30);  
   define l_stop char(30);  
   define l_id char(100);
   define l_ts datetime year to fraction(5);
   define l_f1 float;
   define l_f2 float;


--set debug file to "/tmp/spl.out";
--trace on;


   let l_start = trim(p_ts) || ".00000";
   let l_stop  = trim(p_ts) || ".99999";


   foreach c1 for
     select signalvalue_id,tstamp,col1,col2 
          into l_id,l_ts,l_f1,l_f2 from signalvalue_vti 
     where tstamp >= l_start::datetime year to fraction(5) and
           tstamp <= l_stop::datetime year to fraction(5) 
     return l_id,l_ts,l_f1,l_f2 with resume;
   end foreach;

end procedure; 
