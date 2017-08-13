#!/bin/bash

# NAME: Patrick Shih
# EMAIL: patrickkshih@gmail.com
# ID: 604580648

# SUCCESSFUL ARGUMENTS FOR LAB4B
# LOGFILE
./lab4b --log=log.txt &> /dev/null <<-EOF
OFF
EOF
if [ ! -f log.txt ]
then
echo "Failure to create log file" >> logerr.txt
fi
rm -f log.txt

# PERIOD
./lab4b --period=2 &> /dev/null <<-EOF
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
echo "Failure to recognize valid period argument" >> logerr.txt
fi

# SCALE
./lab4b --scale=C &> /dev/null <<-EOF
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
echo "Failure to recognize valid scale argument (C)" >> logerr.txt
fi

./lab4b --scale=F &> /dev/null <<-EOF
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
echo "Failure to recognize valid period argument(F)" >> logerr.txt
fi

# STANDARD CASE
./lab4b &> /dev/null <<-EOF
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
echo "Failure to run without any arguments" >> logerr.txt
fi

# BAD ARGUMENTS FOR PERIOD
# 0
./lab4b --period=0 &> /dev/null
ret=$?
if [ $ret -ne 1 ]
then
echo "Failure to test when period=0" >> logerr.txt
fi

# negative
./lab4b --period=-1 &> /dev/null
ret=$?
if [ $ret -ne 1 ]
then
echo "Failure to test when period<0" >> logerr.txt
fi

# BAD ARGUMENT FOR SCALE
./lab4b --scale=K &> /dev/null
ret=$?
if [ $ret -ne 1 ]
then
	echo "Failure to test invalid scale argument (K)" >> logerr.txt
fi

# UNRECOGNIZED ARGUMENT
./lab4b --badarg &> /dev/null
ret=$?
if [ $ret -ne 1 ] 
then
	echo "Failure to test invalid argument" >> logerr.txt
fi

# STDIN COMMAND
./lab4b &> /dev/null <<-EOF
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
echo "Failure to regcongize OFF during stdin" >> logerr.txt
fi

./lab4b &> /dev/null <<-EOF
START
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
echo "Failure to regcongize START during stdin" >> logerr.txt
fi

./lab4b &> /dev/null <<-EOF
STOP
OFF
EOF
ret=$?
if [ $ret -ne 0 ]
then
echo "Failure to regcongize STOP during stdin" >> logerr.txt
fi

# BAD ARGUMENT IN GENERAL
./lab4b &> /dev/null <<-EOF
BADARG
EOF
ret=$?
if [ $ret -ne 1 ] 
then 
	echo "Failure to catch invalid argument during stdin" >> logerr.txt
fi

# CHECK IF ALL TEST CASES PASSED
if [ -a logerr.txt ]
then
	echo "Failed the following test cases:"
	cat logerr.txt
	rm -f logerr.txt
else
	echo "Passed all test cases"
fi 

