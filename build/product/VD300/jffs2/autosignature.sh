#!/usr/bin/expect 

set KeyServer 172.21.67.199
set USER pd7-1
set SshPasswd sdc.2017
set timeout -1
set DIR [lindex $argv 0]
set YEAR 20*
set ROOTFS_FILE rootfs.jffs2
set SIGN_ROOTFS sig.rootfs
set SIGN_LIBFS  sig.libfs
set SIGN_PACKAGE  sig.package
set PACKAGE_FILE  package.jffs2
set LIB_FILE rootfs_lib.jffs2
set SIGN_BOOT  sig.bootloader
set BOOT_FILE  cferam.jffs2.img.tmp
set CMD generate_kfs_info.sh

spawn ssh $USER@$KeyServer
expect {
"*continue connecting*" {send "yes\r"}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*Last login*"  {send "mkdir $DIR\r"}
}
send "cp -v ./$CMD ./$DIR\r"
expect {
"*No such file or dir*" {exit -1}
"*>*"   {send "exit\r"}
}
expect "*logout*"
send "exit\r"

#send file
spawn scp $ROOTFS_FILE $USER@172.21.67.199:./$DIR
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}
spawn scp $LIB_FILE $USER@172.21.67.199:./$DIR
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}
spawn scp $PACKAGE_FILE $USER@172.21.67.199:./$DIR
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}

spawn scp $BOOT_FILE $USER@172.21.67.199:./$DIR
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}

spawn ssh $USER@$KeyServer
expect {
"*continue connecting*" {send "yes\r"}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*Last login*"  {send "cd $DIR\r"
                 send "./$CMD\r"}
}
expect "*script end*"
send "exit\r"
expect "*logout*"
send "exit\r"
#download file
spawn scp $USER@172.21.67.199:./$DIR/$SIGN_ROOTFS ./
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}

spawn scp $USER@172.21.67.199:./$DIR/$SIGN_LIBFS ./
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}
spawn scp $USER@172.21.67.199:./$DIR/$SIGN_PACKAGE ./
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}
spawn scp $USER@172.21.67.199:./$DIR/$SIGN_BOOT ./
expect {
"*lost connection*" {exit -1}
"*yes/no*"     {send "yes\r" continue}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*No such file or dir*" {exit -1}
 "*100%*" {send_user  "file transfer sucess\r"}
}
#remove dir

spawn ssh $USER@$KeyServer
expect {
"*continue connecting*" {send "yes\r"}
"*password*"   {send "$SshPasswd\r"}
}
expect {
"*Permission denied*" {exit -1}
"*Last login*"  {send "rm -rf $DIR\r"}
}
expect "*"
send "exit\r"
expect "*logout*"
send "exit\r"
expect eof
