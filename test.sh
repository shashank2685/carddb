#!/bin/bash



# compile source code. with SQL_LITE OFF

DB_FILE=btree.db
FailCount=0
TotalCount=0

generateRandom()
{
  INPUT=""
  for (( i = 0; i < 19; i++ ))
  do
    INPUT=$INPUT$[$RANDOM % 10]
  done
  echo $INPUT
}

#sed -i -e 's/^COMPILE_FOR=SQL_LITE/#COMPILE_FOR=SQL_LITE/' Makefile

make clean
rm -f $DB_FILE
make

if [ ! -f ./carddb ]
then
  echo "Error: Cannot find 'carddb' program"
  exit ;
fi

#########################################################
#   Perform Normal Insert, find, delete and find 
#########################################################

echo "PERFOMRING 100 INSERTION"

rm -f temp_inserted
for (( i = 0; i < 100; i++ ))
do
  INPUT=$(generateRandom)
  echo $INPUT >> temp_inserted
  #echo "Performing Insert with $INPUT"
  ./carddb $DB_FILE insert $INPUT
  RES=$?
  TotalCount=`expr $TotalCount + 1`
  
  if [ $RES -ne 0 ]
  then
    echo "Error : Unable to insert $INPUT. Error Code = $RES"
    FailCount=`expr $FailCount + 1`
  fi
done

if [ $FailCount -eq 0 ]
then
  echo "PERFOMRING 100 INSERTION -- ( 100 INSERTION OK )"
else
  echo "PERFOMRING 100 INSERTION -- ( Failed = $FailCount OUT OF 100)"
fi

echo "PERFOMRING FIND ON INSERTED ELEMENTS"
TEMP=$FailCount
for LINE in `cat temp_inserted`
do
  #echo "Performing find with $LINE"
  ./carddb $DB_FILE find $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1`
  
  if [ $RES -ne 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi
done

if [ $FailCount -eq $TEMP ]
then
  echo "END OF 100 FIND -- ( 100 FIND OK )"
else
  echo "ENF OF 100 FIND -- ( Failed = `expr $FailCount - $TEMP` OUT OF 100)"
fi

TEMP=$FailCount
echo "PERFOMRING DELETE AND FIND ON INSERTED ELEMENTS"
for LINE in `cat temp_inserted`
do
  #echo "Performing delete with $LINE"
  ./carddb $DB_FILE delete $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1`
  
  if [ $RES -ne 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi

  ./carddb $DB_FILE find $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1`
  
  if [ $RES -eq 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi
done

if [ $FailCount -eq $TEMP ]
then
  echo "END OF DELETE AND FIND ON INSERTED ELEMENTS ( 100 DELETE 100 FIND OK )"
else
  echo "END OF DELETE AND FIND ON INSERTED ELEMENTS ( Failed = `expr $FailCount - $TEMP` OUT OF 200)"
fi


echo "PERFOMRING FIND AFTER DELETE"
TEMP=$FailCount
for LINE in `cat temp_inserted`
do
  #echo "Performing find with $LINE"
  ./carddb $DB_FILE find $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1`
  
  if [ $RES -eq 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi
done

if [ $FailCount -eq $TEMP ]
then
  echo "END OF PERFOMRING FIND AFTER DELETE ( 100 OK ) "
else
  echo "END OF PERFOMRING FIND AFTER DELETE ( Failed = `expr $FailCount - $TEMP` OUT OF 100)"
fi

rm -f temp_inserted


echo "PERFORMING DELETE ON EMPTY DATABASE"
./carddb $DB_FILE delete 123456
RES=$?
TotalCount=`expr $TotalCount + 1`

if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "PERFORMING DELETE ON EMPTY DATABASE -- ( OK )"
fi

echo "PERFORMING FIND ON EMPTY DATABASE"
./carddb $DB_FILE find 123456
RES=$?
TotalCount=`expr $TotalCount + 1`

if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "PERFORMING FIND ON EMPTY DATABASE -- ( OK )"
fi

echo "INSERTING VERY VERY LARGE NUMBER = 11111111111111111111111111111111111"
./carddb $DB_FILE insert 11111111111111111111111111111111111
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "INSERTING VERY VERY LARGE NUMBER = 11111111111111111111111111111111111 -- ( OK )"
fi

echo "PERFORMING FIND ON 11111111111111111111111111111111111"
./carddb $DB_FILE find 11111111111111111111111111111111111
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "PERFORMING FIND ON 11111111111111111111111111111111111 -- ( OK ) "
fi

echo "INSERTING NUMBER 18446744073709551616 = ( 2 ^ 64 )"
./carddb $DB_FILE insert 18446744073709551616
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "INSERTING NUMBER 18446744073709551616 = ( 2 ^ 64 ) -- ( OK )"
fi

echo "PERFORMING FIND ON 18446744073709551616 = ( 2 ^ 64 )"
./carddb $DB_FILE find 18446744073709551616
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "PERFORMING FIND ON 18446744073709551616 = ( 2 ^ 64 ) -- ( OK )"
fi

echo "INSERTING NUMBER 18446744073709551615 = ( 2^64 - 1)"
./carddb $DB_FILE insert 18446744073709551615
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -ne 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "INSERTING NUMBER 18446744073709551615 = ( 2^64 - 1) -- ( OK )"
fi

echo "PERFORMIING FIND ON 18446744073709551615 = ( 2^64 - 1)"
./carddb $DB_FILE find 18446744073709551615
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -ne 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "PERFORMIING FIND ON 18446744073709551615 = ( 2^64 - 1) -- ( OK )"
fi

echo "PERFORMING INSERT WITH ILLEGAL CHAR 123456abcd"
./carddb $DB_FILE insert 12345abcd
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "PERFORMING INSERT WITH ILLEGAL CHAR 123456abcd -- ( OK )"
fi

echo "PERFORMING DELETE ON ALREADY DELETED ITEMS"
./carddb $DB_FILE insert 123456
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -ne 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
fi
./carddb $DB_FILE delete 123456
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -ne 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
fi
./carddb $DB_FILE delete 123456
RES=$?
TotalCount=`expr $TotalCount + 1`
if [ $RES -eq 0 ]
then
  echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
  FailCount=`expr $FailCount + 1`
else
  echo "PERFORMING DELETE ON ALREADY DELETED ITEMS -- ( OK )"
fi


rm -f temp_inserted

TEMP=$FailCount
echo "PERFORMING 50 INSERTIONS"
for (( i = 0; i < 50; i++ ))
do
  INPUT=$(generateRandom)
  echo $INPUT >> temp_inserted
  ./carddb $DB_FILE insert $INPUT
  RES=$?
  TotalCount=`expr $TotalCount + 1` 
  if [ $RES -ne 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi
done
if [ $FailCount -eq $TEMP ]
then
  echo "END OF PERFORMING 50 INSERTIONS ( 50 OK ) "
else
  echo "END OF PERFORMING 50 INSERTIONS ( Failed = `expr $FailCount - $TEMP` OUT OF 100)"
fi

TEMP=$FailCount
echo "DELETING 25 FROM THEM"
Counter=0
for LINE in `cat temp_inserted`
do
  Counter=`expr $Counter + 1`
  if [ $Counter -gt 25 ]
  then
    break
  fi
  ./carddb $DB_FILE delete $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1` 
  if [ $RES -ne 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi
done 

if [ $FailCount -eq $TEMP ]
then
  echo "END OF DELETING 25 FROM THEM ( 25 OK ) "
else
  echo "END OF DELETING 25 FROM THEM ( Failed = `expr $FailCount - $TEMP` OUT OF 100)"
fi

TEMP=$FailCount
echo "PERFORMING FIND ON ALL 50 CARDIDS"
Counter=0
for LINE in `cat temp_inserted`
do
  Counter=`expr $Counter + 1`
  ./carddb $DB_FILE find $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1` 
  if [ $Counter -le 25 ]
  then
    if [ $RES -eq 0 ]
    then
      echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
    fi
  else
    if [ $RES -ne 0 ]
    then
      echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
    fi
  fi
done

if [ $FailCount -eq $TEMP ]
then
  echo "END OF PERFORMING FIND ON ALL 50 CARDIDS ( 50 OK ) "
else
  echo "END OF PERFORMING FIND ON ALL 50 CARDIDS ( Failed = `expr $FailCount - $TEMP` OUT OF 100)"
fi

rm -f temp_inserted1
TEMP=$FailCount
echo "INSERTING 25 NEW ELEMENTS"
for (( i = 0; i < 25; i++ ))
do
  INPUT=$(generateRandom)
  echo $INPUT >> temp_inserted1
  ./carddb $DB_FILE insert $INPUT
  RES=$?
  TotalCount=`expr $TotalCount + 1` 
  if [ $RES -ne 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi
done
if [ $FailCount -eq $TEMP ]
then
  echo "END OF INSERTING 25 NEW ELEMENTS ( 25 OK ) "
else
  echo "END OF INSERTING 25 NEW ELEMENTS ( Failed = `expr $FailCount - $TEMP` OUT OF 100)"
fi

echo "PERFORMING FIND ON OLD 25 ENTRIES"
Counter=0
TEMP=$FailCount
for LINE in `cat temp_inserted`
do
  Counter=`expr $Counter + 1`
  if [ $Counter -gt 25 ]
  then
  ./carddb $DB_FILE find $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1` 
    if [ $RES -ne 0 ]
    then
      echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
    fi
  fi
done
if [ $FailCount -eq $TEMP ]
then
  echo "END OF PERFORMING FIND ON OLD 25 ENTRIES ( 25 OK ) "
else
  echo "END OF PERFORMING FIND ON OLD 25 ENTRIES ( Failed = `expr $FailCount - $TEMP` OUT OF 100)"
fi

echo "PERFORMING FIND ON NEW 25 ENTRIES"
TEMP=$FailCount
for LINE in `cat temp_inserted1`
do
  ./carddb $DB_FILE find $LINE
  RES=$?
  TotalCount=`expr $TotalCount + 1` 
  if [ $RES -ne 0 ]
  then
    echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
    FailCount=`expr $FailCount + 1`
  fi
done
if [ $FailCount -eq $TEMP ]
then
  echo "END OF PERFORMING FIND ON NEW 25 ENTRIES ( 25 OK ) "
else
  echo "END OF PERFORMING FIND ON NEW 25 ENTRIES ( Failed = `expr $FailCount - $TEMP` OUT OF 25 )"
fi

Counter=0
TEMP=$FailCount
echo "PERFORMING FIND DELETE FIND ON ALL OLD AND NEW ENTRIES"
for LINE in `cat temp_inserted`
do
  Counter=`expr $Counter + 1`
  TotalCount=`expr $TotalCount + 1` 
  ./carddb $DB_FILE find $LINE
  RES=$?
  if [ $Counter -le 25 ]
  then
    #element should not be there in database
    if [ $RES -eq 0 ]
    then
      echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
    fi
  else
    #counter > 25 elements should be present in database
    if [ $RES -ne 0 ]
    then
      echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
    fi
  fi
 
  # Delete the element. 
  TotalCount=`expr $TotalCount + 1` 
  ./carddb $DB_FILE delete $LINE
  RES=$?
  if [ $Counter -le 25 ]
  then
    #element should not be there in database
    if [ $RES -eq 0 ]
    then
      echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
    fi
  else
    #counter > 25 elements should be present in database
    if [ $RES -ne 0 ]
    then
      echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
    fi
  fi

  #Search for the Deleted element.
  TotalCount=`expr $TotalCount + 1` 
  ./carddb $DB_FILE delete $LINE
  RES=$?
  if [ $RES -eq 0 ]
  then
      echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
  fi
done
if [ $FailCount -eq $TEMP ]
then
  echo "END OF PERFORMING FIND DELETE FIND ON ALL OLD AND NEW ENTRIES -- ( OK )"
else
  echo "END OF PERFORMING FIND DELETE FIND ON ALL OLD AND NEW ENTRIES ( Failed = `expr $FailCount - $TEMP` )"
fi

rm -f temp_inserted1
rm -f temp_inserted
rm -f $DB_FILE


#insert 50 cardids
echo "INSERTING 50 CARDIDS FOR RANDOM OPERATION"
for (( i = 0; i < 50; i++ ))
do
  INPUT=$(generateRandom)
  echo $INPUT >> temp_inserted
  TotalCount=`expr $TotalCount + 1` 
  ./carddb $DB_FILE insert $INPUT
  RES=$?
  if [ $RES -ne 0 ]; then
      echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
      FailCount=`expr $FailCount + 1`
  fi
done
#delete 25 cardids
echo "DELETING 25 CARDIDS"
for (( i = 0; i < 25; i++ ))
do
  NUM=`wc -l < temp_inserted`
  INPUT=`head -$((${RANDOM} % $NUM + 1)) temp_inserted | tail -1`
  if [ -s temp_deleted ];then
    TEMP=`grep "^"$INPUT"$" temp_deleted | wc -l`
  else
    TEMP=0
  fi

  if [ $TEMP -gt 0 ]; then
    i=`expr $i - 1`
  else
    echo $INPUT >> temp_deleted
    TotalCount=`expr $TotalCount + 1` 
    ./carddb $DB_FILE delete $INPUT
    RES=$?
    if [ $RES -ne 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
    fi
  fi
done

# Doing 100 Random Operations.
echo "PERFORMING 100 RANDOM OPERATIONS"
INSERT=0
DELETE=0
FIND=0

for (( i = 0; i < 100; i++ ))
do
  op=$[$RANDOM % 3]
  TotalCount=`expr $TotalCount + 1` 
  case $op in
    0)
      INSERT=`expr $INSERT + 1`
      INPUT=$(generateRandom)
      ./carddb $DB_FILE insert $INPUT
      RES=$?
      if [ $RES -ne 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
      fi
      echo $INPUT >> temp_inserted
      ;;
    1)
     FIND=`expr $FIND + 1`
     switch=$[$RANDOM % 3]
     if [ $switch -eq 0 ]
     then
       #perform search on already  present number.
       NUM=`wc -l < temp_inserted`
       while [[ 1 ]]; do
        INPUT=`head -$((${RANDOM} % $NUM + 1)) temp_inserted | tail -1`
        TEMP=`grep "^"$INPUT"$" temp_deleted | wc -l`
        if [ $TEMP -eq 0 ]; then
          break
        fi
       done
       ./carddb $DB_FILE find $INPUT
       RES=$?
       if [ $RES -ne 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
       fi
     fi
     if [ $switch -eq 1 ]; then
       #perform search on number not present.
       while [[ 1 ]]
       do
         INPUT=$(generateRandom)
         TEMP=`grep "^"$INPUT"$" temp_inserted | wc -l`
         TEMP1=`grep "^"$INPUT"$" temp_deleted | wc -l`
         if [ $TEMP -eq 0 ] && [ $TEMP1 -eq 0 ]
         then
           break
         fi
       done
       ./carddb $DB_FILE find $INPUT
       RES=$?
       if [ $RES -eq 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
       fi
     fi
     if [ $switch -eq 2 ]; then
       #perform find on already deleted number
       NUM=`wc -l < temp_deleted`
       INPUT=`head -$((${RANDOM} % $NUM + 1)) temp_deleted | tail -1`
       ./carddb $DB_FILE find $INPUT
       RES=$?
       if [ $RES -eq 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
       fi
     fi
     ;;
    2)
     DELETE=`expr $DELETE + 1`
     switch=$[$RANDOM % 3]
     if [ $switch -eq 0 ]
     then
       #perform delete on already  present number.
       NUM=`wc -l < temp_inserted`
       while [[ 1 ]]; do
        INPUT=`head -$((${RANDOM} % $NUM + 1)) temp_inserted | tail -1`
        TEMP=`grep "^"$INPUT"$" temp_deleted | wc -l`
        if [ $TEMP -eq 0 ]; then
          break
        fi
       done
       ./carddb $DB_FILE delete $INPUT
       RES=$?
       if [ $RES -ne 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
       fi
       echo $INPUT >> temp_deleted
     fi
     if [ $switch -eq 1 ]; then
       #perform delete on number not present.
       while [[ 1 ]]
       do
         INPUT=$(generateRandom)
         TEMP=`grep "^"$INPUT"$" temp_inserted | wc -l`
         TEMP1=`grep "^"$INPUT"$" temp_deleted | wc -l`
         if [ $TEMP -eq 0 ] && [ $TEMP1 -eq 0 ]
         then
           break
         fi
       done
       ./carddb $DB_FILE delete $INPUT
       RES=$?
       if [ $RES -eq 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
       fi
     fi
     if [ $switch -eq 2 ]; then
       #perform delete on already deleted number
       NUM=`wc -l < temp_deleted`       
       INPUT=`head -$((${RANDOM} % $NUM + 1)) temp_deleted | tail -1`
       ./carddb $DB_FILE delete $INPUT
       RES=$?
       if [ $RES -eq 0 ]; then
        echo -e '\E[47;31m'"\033[1m Error : Expected Non Zero. Got ( $RES ). \033[0m"
        FailCount=`expr $FailCount + 1`
       fi
     fi
     ;;
  esac
done
rm -f temp_inserted
rm -f temp_deleted

echo "PERFORMED INSERT = $INSERT, FIND = $FIND, DELETE = $DELETE IN 100 RANDOM OPS"
echo "Total Number Of Operations = $TotalCount"
echo "Total Number Of Failed Operations = $FailCount"
