# .bashrc

# User specific aliases and functions

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

export TCOP_HOME=$HOME
export TCOP_BANKID=15
export TCOP_BANKADDR=10.112.9.60:8088:30 #localhost:6789:30
#export TCOP_DBURL=sybase://tcop:tcoppw@SYBASE/opdb/
export TCOP_DBURL=sybase://exbadm:exbadm@SYBASE/cddb/
export TCOP_DBDEBUG=1

set -o vi
export PS1=[$LOGNAME:\$PWD];
export JAVA_HOME=/opt/jdk1.5.0_06
export PATH=.:$JAVA_HOME/bin:$PATH

PATH=.:$HOME/bin:$PATH
LD_LIBRARY_PATH=.:$HOME/lib:$LD_LIBRARY_PATH
export PATH LD_LIBRARY_PATH

export FILES_DIR=$HOME/file
export LANG=en_US
export LANGUAGE=en_US
export KEYDIR=$HOME/key
alias pf='cd $HOME/src/process/PF'
alias sz='cd $HOME/src/region/sz'
alias etc='cd $HOME/spdbsvr/etc'
alias map='cd $HOME/spdbsvr/mapfile'
alias src='cd $HOME/spdbsvr/src'

export APP_DIR=$HOME/spdbsvr
export SOPHOME=$HOME/spdbsvr
export SVN_EDITOR=vi
export EDITOR=vi
export PLAT=LINUX
export TFTCFG=/home/tcop/etc/transfer.cfg
export TFT_CLIENT_PRINTDIR=/home/tcop/tft/data
export LD_LIBRARY_PATH=/home/tcop/tft/lib:.:$HOME/lib:$LD_LIBRARY_PATH
export TFTFILE=/home/tcop/etc/tft.xml
ulimit -S -c unlimited

alias lls=ls
alias cls=clear
alias sopcli=~/spdbsvr/bin/sopcli
alias pf="cd ~/src/process/PF"
alias hy="cd ~/src/process/HY"
export title="浦发开发环境"
