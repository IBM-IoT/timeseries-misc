dt=`date +"%Y-%m-%d %H:%M:%S.00000"`
dt='2015-01-01 00:00:00.00000'

NUMDBS=8                 # 1 cont per dbspace
NUMPMU=48                # Number of pmu's to seed 


echo "======================================="
echo "Creat TS calendar/rowtype/table"
echo "======================================="

dbaccess epg - <<!


drop table pmu_large;
drop table pmu_large_vti;

create table pmu_large
(
   pmu_id integer not null primary key,
   data timeseries(pmu_large_row_t)

);


!

echo "======================================="
echo "Creating Virtual table"
echo "======================================="
dbaccess epg - <<!
   execute procedure tscreatevirtualtab(
           'pmu_vti', 'pmu', 384, 'data' ); 
   execute procedure tscreatevirtualtab(
           'pmu_large_vti', 'pmu_large', 384, 'data' ); 
!


echo "======================================="
echo "Seed pmu table - $NUMPMU pmu's"
echo "======================================="

let CONTNUM=1;
for ((i=1; i<= $NUMPMU; i++));
do

echo "Insert pmu_id:$NUMPMU into CONT:$CONTNUM"
dbaccess epg - <<!
   -- 60 hertz
   begin work;
   insert into pmu_large values($i, tscreateirr('ts_1sec', '$dt',
                          0,60,0,'contlarge$CONTNUM')); 
   commit work;
!

let CONTNUM=CONTNUM+1;
if [ $CONTNUM -gt $NUMDBS ]
then
   let CONTNUM=1;
fi

done








