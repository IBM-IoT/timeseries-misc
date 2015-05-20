dt=`date +"%Y-%m-%d %H:%M:%S.00000"`
dt='2015-01-01 00:00:00.00000'

NUMDBS=8                 # 1 cont per dbspace
NUMPMU=48                # Number of pmu's to seed 


echo "======================================="
echo "Creating database"
echo "======================================="

dbaccess sysadmin - <<!
drop database if exists epg;
create database epg in data_dbs1 with buffered log;
!

echo "======================================="
echo "Creat TS calendar/rowtype/table"
echo "======================================="

dbaccess epg - <<!

insert into calendartable (c_name, c_calendar) 
     values (
            "ts_1sec", 
            "startdate($dt), pattstart($dt), pattern( {1 on} second)" 
     );


create row type pmu_row_t
(
   tstamp datetime year to fraction(5),
   col1 integer,
   col2 float  
);


create row type pmu_large_row_t
(
   tstamp datetime year to fraction(5),
   col1 float,
   col2 float,  
   col3 float,  
   col4 float,  
   col5 float,  
   col6 float,  
   col7 float,  
   col8 float,  
   col9 float,  
   col10 float,  
   col11 float,  
   col12 float,  
   col13 float,  
   col14 float,  
   col15 float 
);

create row type pmu_json_row_t
(
   tstamp datetime year to fraction(5),
   json_data bson
);

create table pmu
(
   pmu_id integer not null primary key,
   data timeseries(pmu_row_t)

);


create table pmu_large
(
   pmu_id integer not null primary key,
   data timeseries(pmu_large_row_t)

);

create table pmu_json (
   pmu_id integer not null primary key,
   data timeseries(pmu_json_row_t)
);




!

echo "======================================="
echo "Creating Containers"
echo "======================================="
for ((i=1; i<= $NUMDBS; i++));
do

echo $i
dbaccess epg - <<!
   execute procedure tscontainercreate(
           'cont$i', 'cont_dbs$i','pmu_row_t', 1024, 1024 ); 
   execute procedure tscontainercreate(
           'contlarge$i', 'cont_dbs$i','pmu_large_row_t', 1024, 1024 ); 
   execute procedure tscontainercreate(
           'contjson$i', 'cont_dbs$i','pmu_json_row_t', 1024, 1024 ); 
!

done

echo "======================================="
echo "Creating Virtual table"
echo "======================================="
dbaccess epg - <<!
   execute procedure tscreatevirtualtab(
           'pmu_vti', 'pmu', 384, 'data' ); 
   execute procedure tscreatevirtualtab(
           'pmu_large_vti', 'pmu_large', 384, 'data' ); 
   execute procedure tscreatevirtualtab(
           'pmu_json_vti', 'pmu_json', 384, 'data' ); 
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
   insert into pmu values($i, tscreateirr('ts_1sec', '$dt',
                          0,60,0,'cont$CONTNUM')); 
   insert into pmu_large values($i, tscreateirr('ts_1sec', '$dt',
                          0,60,0,'contlarge$CONTNUM')); 
   insert into pmu_json values($i, tscreateirr('ts_1sec', '$dt',
                          0,0,0,'contjson$CONTNUM')); 
   commit work;
!

let CONTNUM=CONTNUM+1;
if [ $CONTNUM -gt $NUMDBS ]
then
   let CONTNUM=1;
fi

done








