#!/bin/sh
DIR="/tmp/tcpdump"		   
SN_FILE="/tmp/sal/misc.sal"
upload_flag=0
dump_flag=0
tcpdump_flag="/tmp/tcpdump_running"
i=1
get_sn()
{
	if test -f $SN_FILE;then
		board_csn=`cat $SN_FILE | grep Board_CSN`
		echo ${board_csn##*=}
	fi
}

echo "0">$tcpdump_flag
##acquire the base timestamp
base=`date +%s`


if [ ! -d $DIR ];then
	mkdir -p $DIR	  
fi
dumppid=$(ps | grep tcpdump |awk '{print $1}')
if [[ $dumppid ]] ;then
	killall tcpdump
fi


#get the free ramsize
get_free_ram()
{
	echo `free`| while read line
	do
		echo `expr ${line##* } \* 1024`
	done
}

#according time to tcpdump packet
functime()
{
	echo $1 | awk '{$1=strftime("%Y_%m_%d_%H_%M_%S",$1);print}'
}


#according size to tcpdump packet
funcsize()
{
	if [ -f $1 ];then
		echo `ls -la $1 | awk '{print $5}'`
	fi
}

#support anonymous access
upload()
{
	if test -f $1;then
		if [ `funcsize $1` -gt 0 ];then
			code=$(curl_s -f $1 -u $upload_url -n$upload_username -p$upload_passwd -r$upload_retry)
			if [ $? -eq 0 ];then
				rm -rf $1
			fi
		fi
	fi
}

tcpdump_file()
{
	while true;do
		sleep 1
		read flag <$tcpdump_flag
		if [ "$flag" -eq 1 ];then
			exit 0
		fi
		ramsize=`find $DIR -type f -exec ls -l {} \; | awk 'BEGIN {size=0;}{size+=$5};END{print size}'`
		if [[ "$ramsize" -lt `get_free_ram` && "$ramsize" -lt "$limitsize" ]];then  
			FIRSTLOG=$DIR/tcpdump_`get_sn`_`date +%Y_%m_%d_%H_%M_%S`.pcap
			if [ "$dump_flag" -eq 1 ];then
				dumppid=$(ps | grep tcpdump |awk '{print $1}')
				if [ -n "$dumppid" ];then
					continue
				else
					if [ $tcpdump_file_time -gt 0 ];then
						tcpdump  $traffic_filter -G $tcpdump_file_time	-w $DIR/tcpdump_`get_sn`_%Y_%m_%d_%H_%M_%S.pcap 2> /dev/null & 
						dump_flag=1
					fi
					if [ $tcpdump_file_size -gt 0 ];then
						tcpdump  $traffic_filter -C $tcpdump_file_size	-w $FIRSTLOG 2> /dev/null & 
						dump_flag=1
					fi
				fi
			else
				dumppid=$(ps | grep tcpdump |awk '{print $1}')
				if [[ $dumppid ]] ;then
					killall tcpdump
				fi
				if [ $tcpdump_file_time -gt 0 ];then
					tcpdump  $traffic_filter -G $tcpdump_file_time	-w $DIR/tcpdump_`get_sn`_%Y_%m_%d_%H_%M_%S.pcap 2> /dev/null &
					dump_flag=1
				fi
				if [ $tcpdump_file_size -gt 0 ];then
					tcpdump  $traffic_filter -C $tcpdump_file_size	-w $FIRSTLOG 2> /dev/null & 
					dump_flag=1
				fi

			fi
		else
			stopdumppid=`ps | grep tcpdump | awk '{print $1}'`
			if test -n "$stopdumppid";then
				kill -9 $stopdumppid
				dump_flag=0					
			else
				continue
			fi
		fi
	done
}


#upload file
upload_file()
{
	while true;do
	
		if [ ! -d $DIR ];then
		    continue
		 fi	
		for file in $(ls -ltr $DIR | grep .pcap | awk '{print $9}')
        	do
        	    if [ ! -f $DIR/$file ];then
        	        continue
        	    fi
        	    is_busy=`lsof $DIR/$file`
        	    if [ "$is_busy" = "" ] ; then
			        upload $DIR/$file
				       	    
        	    fi
        	done				
		
	usleep 100
	read flag <$tcpdump_flag
	if [ "$flag" -eq 1 ];then
		exit 0
	fi		
done
}

funcstart()
{
	echo "0">$tcpdump_flag
	tcpdump_file &
	echo $!>/tmp/dump.pid
	upload_file &
}


funcstop()
{
	# kill dump child process
	echo "1">$tcpdump_flag

	sleep 1

	if test -f "/tmp/dump.pid";then
		read dump_ppid < /tmp/dump.pid
		dump_exist_ppid=`ps | grep $dump_ppid | awk '{print $1}'`
		if test -n "$dump_exist_ppid";then
			kill -9 $dump_exist_ppid
		fi
	fi



	#rm -rf the destination file
	if [ -d $DIR ];then
		rm -rf $DIR		
	fi

	exit 0
}

#get parameters
ARGS=`getopt -o "sf::t:z:u:n:p:r:m:e" -l "start,filter::,time:,size:,url:,username:,passwd:,retry:,maxsize:,end" -n "tcpdump.sh" -- "$@"`
eval set -- "${ARGS}"

while [ $# -gt 0 ];do
	case "${1}" in
		-s|--start)
		funcstart
		shift;
		;;
		-f|--filter)
		traffic_filter=$2
		shift 2;
		;;
		-t|--time)
		tcpdump_file_time=$2
		shift 2;
		;;

		-z|--size)
		tcpdump_file_size=$2
		tcpdump_file_size_bytes=`expr $2 \* 1000000`
		shift 2;
		;;
		-u|--url)
		upload_url=$2
		shift 2;
		;;
		-n|--username)
		upload_username=$2
		shift 2;
		;;
		-p|--passwd)
		upload_passwd=$2
		shift 2;
		;;
		-r|--retry)
		upload_retry=$2
		shift 2;
		;;
		-m|--maxsize)
		max_ram_size=$2
		limitsize=`expr $max_ram_size \* 1024`
		shift 2;
		;;
		-e|--end)
		shift;
		funcstop
		exit 
		;;
		--)
		shift;
		break
		;;
	esac
done


