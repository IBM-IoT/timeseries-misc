dt=`date +"%Y-%m-%d %H:%M:%S.00000"`
dt='2016-01-01 00:00:00.00000'


echo "======================================="
echo "Creating database"
echo "======================================="

dbaccess sysadmin - <<!
drop database if exists simpletsjson;
create database simpletsjson in rootdbs with buffered log;
!

echo "======================================="
echo "Creat TS calendar/rowtype/table"
echo "======================================="

dbaccess simpletsjson - <<!

insert into calendartable (c_name, c_calendar)
     values (
            "ts_1sec",
            "startdate($dt), pattstart($dt), pattern( {1 on} second)"
     );


create row type sensors_row_t
(
   tstamp datetime year to fraction(5),
   json_data bson
);


create table sensors (
   id varchar(255) not null primary key,
   data timeseries(sensors_row_t)
);


!

echo "======================================="
echo "Creating Containers"
echo "======================================="
dbaccess simpletsjson - <<!
   execute procedure tscontainercreate(
           'container1', 'rootdbs','sensors_row_t', 1024, 1024 );
!


echo "======================================="
echo "Creating Virtual table"
echo "======================================="
dbaccess simpletsjson - <<!
   execute procedure tscreatevirtualtab(
           'sensors_vti', 'sensors',
          'calendar(ts_1sec), origin($dt), irregular');
!
