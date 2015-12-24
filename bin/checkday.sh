#!/bin/bash

. $HOME/.bashrc
echo "同城清算每日检查开始:" >/tmp/ccrccheckday.log
date >>/tmp/ccrccheckday.log
echo "Script written by ShenJ." >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
echo "1. df -v . " >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
df -v >>/tmp/ccrccheckday.log

echo "=====================================================" >>/tmp/ccrccheckday.log
echo "2. sybase status ( four process ) . " >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
showserver >>/tmp/ccrccheckday.log

echo "=====================================================" >>/tmp/ccrccheckday.log
echo "3. process status ( two tcpsvr process, one tftserver process, two spdbsvr process ). " >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
ps -ef | grep tcp >>/tmp/ccrccheckday.log
ps -ef | grep tft >>/tmp/ccrccheckday.log
ps -ef | grep spdb >>/tmp/ccrccheckday.log

echo "=====================================================" >>/tmp/ccrccheckday.log
echo "4. ping ccrc status . " >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
ping -c 3 9.48.175.99 >>/tmp/ccrccheckday.log

echo "=====================================================" >>/tmp/ccrccheckday.log
echo "5. ping MYJ status . " >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
ping -c 3 10.9.128.200 >>/tmp/ccrccheckday.log
ping -c 3 10.9.128.203 >>/tmp/ccrccheckday.log

echo "=====================================================" >>/tmp/ccrccheckday.log
echo "6. Sybase backup . " >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
ls -l /home/tcop/db.bak >>/tmp/ccrccheckday.log

echo "=====================================================" >>/tmp/ccrccheckday.log
echo "7. opdb 库剩余空间. " >>/tmp/ccrccheckday.log
echo "=====================================================" >>/tmp/ccrccheckday.log
sh /home/tcop/bin/tj_db.sh >>/tmp/ccrccheckday.log
