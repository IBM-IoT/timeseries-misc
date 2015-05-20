
#         <hz> <start hr> <stop hr> <start min> <stop min> <start id> <stop id>



for i in {1..6}
do
ins_hz.py 30 8 8 0 59 $i $i   > ins1.unl
ins_hz.py 30 8 8 0 59 $(($i+1)) $(($i+1))   > ins2.unl
ins_hz.py 30 8 8 0 59 $(($i+2)) $(($i+2))   > ins3.unl
ins_hz.py 30 8 8 0 59 $(($i+3)) $(($i+3))   > ins4.unl
ins_hz.py 30 8 8 0 59 $(($i+4)) $(($i+4))   > ins5.unl
ins_hz.py 30 8 8 0 59 $(($i+5)) $(($i+5))   > ins6.unl
ins_hz.py 30 8 8 0 59 $(($i+6)) $(($i+6))   > ins7.unl
ins_hz.py 30 8 8 0 59 $(($i+7)) $(($i+7))   > ins8.unl



done








