#!/bin/bash

. $HOME/.bashrc

chk_process()
{
    ps -u $LOGNAME | egrep "$1\b" | grep -v grep > /dev/null 2>/dev/null
    if [ $? -ne 0 ]; then
        return -1
    fi
    return 0
}

chk_process tcpsvr
if [ $? -ne 0 ]; then
    cd $HOME/bin; keyctl stop; keyctl start; tcpsvr&
fi

chk_process spdbsvr 
if [ $? -ne 0 ]; then
    cd $HOME/bin; spdbsvr&
fi

chk_process spdbsvr_st
if [ $? -ne 0 ]; then
    cd $HOME/bin; spdbsvr_st&
fi
