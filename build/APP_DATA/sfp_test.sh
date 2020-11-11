#!/bin/sh
num=1
while [ $num -le $1 ];do
echo "test sfp pin count $num"
num=$(expr $num '+' 1)
echo o119 > /proc/sc_led
sleep 1;
/usr/sbin/srcm_i2c w 0x51 0xc6 0x01
sleep 1;
sfp_gpio=`cat /proc/sc_led`
if [ "$sfp_gpio" = "1,1,1,1" ]; then
echo "test gpio pin 19,35,36,37 high OK"
else
echo "test gpio pin 19,35,36,37 high fail"
fi

echo o019 > /proc/sc_led
sleep 1;
/usr/sbin/srcm_i2c w 0x51 0xc6 0x00
sleep 1;
sfp_gpio=`cat /proc/sc_led`
if [ "$sfp_gpio" = "0,0,0,0" ]; then
echo "test gpio pin 19,35,36,37 Low OK"
else
echo "test gpio pin 19,35,36,37 Low fail"
fi

i2c=`/usr/sbin/srcm_i2c r 0x51 0xc9 4`
sleep 1;
scom="53 43 4f 4d"
result=$(echo $i2c | grep "$scom")
if [[ "$result" != "" ]]; then
echo "test i2c pin 117,119 OK"
else
echo "test i2c pin 117,119 fail"
fi

pingtime=`ping 192.168.2.200 -c 1 -s 1 -w 1 |grep "100% packet loss" | wc -l`
if [ $pingtime -eq 0 ];then
echo "ping 192.168.2.200 ok"
else
echo "ping 192.168.2.200 fail"
fi

done
