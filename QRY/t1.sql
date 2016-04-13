 
select count(*) 
from table (transpose 
 (( select  clip( data,
             '2015-05-15 16:30:00.00000'::datetime year to fraction(5),
             '2015-05-15 16:31:00.00000':: datetime year to fraction(5)
                 )
             from signalvalue where
             signalvalue_id  in (  "CALIFORNIA1.CALIFORNIA1_IM", "CALIFORNIA1.CALIFORNIA1_VM")
 ))::signalvalue_row_t ) as tab(t);


