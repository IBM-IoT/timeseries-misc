for i in {1..8}
do
java -cp .:./ifxjdbc.jar BulkInsert $i &
done

wait

