
DBSSZ="200MB"
NUMDATADBS=1
NUMCONTDBS=8


################################################
# Make storage Pool & specail dbspaces
################################################

dbaccess sysadmin - <<!
execute function admin ("storagepool add", "$INFORMIXDIR/storage", 0,0,'$DBSSZ',1);

execute function admin("create dbspace from storagepool", "plog", '$DBSSZ');
execute function admin("create dbspace from storagepool", "llog", '$DBSSZ');
execute function admin("create tempdbspace from storagepool", "tmp_dbs1", '$DBSSZ');
execute function admin("create tempdbspace from storagepool", "tmp_dbs2", '$DBSSZ');

!



################################################
# Move plog
################################################
echo "=============================="
echo "Moving Plog"
echo "=============================="

dbaccess sysadmin - <<!
execute function admin("alter plog", "plog", "195MB" );
!

################################################
# Add llog 
################################################
echo "=============================="
echo "Adding llog"
echo "=============================="

dbaccess sysadmin - <<!
execute function admin("add log", "llog", "50M",9,0 );
!




################################################
# Make Dbspaces for instance
################################################

for (( i=1; i<= $NUMDATADBS; i++ ));
do
echo "=============================="
echo "Creating Dbspace $i"
echo "=============================="
dbaccess sysadmin - <<!
   execute function admin("create dbspace from storagepool","data_dbs$i", "medium", '$DBSSZ');

!
done

for (( i=1; i<= $NUMCONTDBS; i++ ));
do
echo "=============================="
echo "Creating Dbspace $i"
echo "=============================="
dbaccess sysadmin - <<!
   execute function admin("create dbspace from storagepool","cont_dbs$i", "medium", '$DBSSZ');

!
done


