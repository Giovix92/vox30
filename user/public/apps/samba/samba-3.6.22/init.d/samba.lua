#!/bin/lua
require "ubus"
require "uloop"

local conn = ubus.connect()
if not conn then
    error("Failed to connect to ubusd")
end

--function config()

    function get_value(path)
            name=conn: call("v1.DataModel", "get", {object={path}})
               if #name ~= nil then
                     for k,v in pairs(name) do
                            if type(v) == "table" then
                                 for k2, v2 in pairs(v) do
                                       if type(v2) == "table" then
                                             for k3, v3 in pairs(v2) do
                                                  -- print(k3 .. "\t" .. tostring(v3))
                                                  return tostring(v3)
                                             end
                                       else
                                             return tostring(v2)
                                                                                                                                                                                                                                                                                 
                                       end                                                                                                                                                                                                                                                                        
                                 end
                            else
                                      return tostring(v)                                                                                                                                                                                                                                                                                 
                            end
                     end
               end
      end
      
      
local a,b,c,d
a=get_value("InternetGatewayDevice.Services.StorageService.1.X_VODAFONE_WorkGroup")
b=get_value("InternetGatewayDevice.Services.StorageService.1.X_VODAFONE_ServerName")
c=get_value("InternetGatewayDevice.Services.StorageService.1.X_VODAFONE_NetbiosName")
d=get_value("InternetGatewayDevice.Services.StorageService.1.NetworkServer.X_VODAFONE_SMBSigning")

-- check if /var/samba exists
if os.execute("cd \"/var/samba/\">nul 2>nul") ~=0 then
   os.execute("mkdir /var/samba/")
end

-- check if open ok, or close conn:close
local f = io.open("/var/samba/smb.conf","w+")
if (f==nil)then
   conn:close()
end

-- check if open ok, or close conn:close, close f1
--[[local f1 = io.open("/etc/passwd","a+")
if (f1==nil)then
    f1:close()
    conn:close()
end--]]



f:write("[global]\n config file=/var/samba/smb.conf\n display charset = utf8\n unix charset = utf8\n dos charset = utf8\n")
f:write("workgroup ="..a.."\n server string ="..b.."\n netbios name ="..c.."\n")
f:write("load printers = yes\n printing = cups\n printcap = /tmp/printcap\n cups options = raw\n disable spoolss = no\n encrypt passwords = yes\n passdb backend =tdbsam\n smb passwd file = /var/samba/smbpasswd\n socket options = TCP_NODELAY SO_KEEPALIVE SO_SNDBUF=16384 SO_RCVBUF=1638\n bind interfaces only = yes\n interfaces = br0\n dns proxy = no\n preserve case = yes\n short preserve case = yes\n default case =  upper\n case sensitive = yes\n mangled names = yes\n null passwords = yes\n")

if d == "0" then
   f:write("server signing = no\n")
else
      f:write("server signing = yes\n")
end 

f:write("dos filetimes = yes\n kernel oplocks = no\n max protocol = SMB2\n delete veto files = False\n force directory mode=771\n force create mode=660\n create mask=771\n ;if set map system as true, it would cause file treat as executable file\n map system=no\n syslog=0\n log file=/dev/null\n guest ok=yes\n map to guest=Bad User\n guest account=nobody\n oplocks=no\n wins server = yes\n") 


folder_name = conn: call("v1.DataModel","list",{object={"InternetGatewayDevice.Services.StorageService.1.LogicalVolume."}})
       for k,v in pairs(folder_name) do
            if #folder_name ~= nil then
                 for k,v in pairs(folder_name) do
                       if type(v) == "table" then
                             for k2, v2 in pairs(v) do
                                   if type(v2) == "table" then
                                         for k3, v3 in pairs(v2) do
                                              status = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume.".. v3 .. ".Status")
                                              if status == "online" then
                                                  folder1 = conn:call("v1.DataModel","list",{object={"InternetGatewayDevice.Services.StorageService.1.LogicalVolume.".. v3 ..".Folder."}})
                                                      for k5, v5 in pairs(folder1) do
                                                            if type(v5) == "table" then
                                                                 for k6,v6 in pairs(v5) do
                                                                      if type(v6)=="table" then
                                                                            for k7,v7 in pairs(v6) do
                                                                                  named = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume." .. v3 .. ".Folder.".. v7..".X_VODAFONE_Ns_Name" )
                                                                                  access = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume." .. v3..".Folder." .. v7 ..".UserAccountAccess")
                                                                                  permission = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume."..v3..".Folder."..v7..".UserAccess.1.Permissions")
                                                                                  enable = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume." .. v3..".Enable")
                                                                                  sname = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume." .. v3..".Name")
                                                                                  line = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume." .. v3 ..".Folder.".. v7 ..".Name") 
                                                                                  mods = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume." .. v3 .. ".PhysicalReference")                                                                                                                                                                                                                                                
                                                                                  model = get_value("InternetGatewayDevice.Services.StorageService.1" .. mods .. ".Model")
                                                                                  usraccount = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume."..v3..".Folder."..v7..".UserAccess.1.UserReference")
                                                                                  seria = get_value("InternetGatewayDevice.Services.StorageService.1.LogicalVolume."..v3..".X_VODAFONE_SerialNum")
                                                                                  -- print(seria)
                                                                                  local id = 99 + v7
                                                                                 
                                                                                  if model ~= nil then
                                                                                     if seria == "None:1" then 
                                                                                       if enable == "1" then
                                                                                             f:write("\n[".. named .."]\n")
                                                                                           if access == "0" then
                                                                                                aa = "path=/mnt/shares/".. model .. line .. sname
                                                                                                f:write("\t"..aa)
                                                                                                cc = "\n\tsecurity = share\n\tfollow symlinks = no\n\twide links = no\n\twritable = yes\n"
                                                                                                f:write("\t"..cc)
                                                                                              
                                                                                           else
                                                                                                                                                             
                                                                                                ee = "\n\tfollow symlinks = no\n\twide links = no\n\tsecurity = user\n"
                                                                                                f:write("\t"..ee)
                                                                                                ff = "path=/mnt/shares/".. model .. line .. sname
                                                                                                f:write("\t"..ff.."\n")
                                                                                                
                                                                                                if   usraccount ~= nil then 
                                                                                                         username = get_value("InternetGatewayDevice.Services.StorageService.1"..usraccount ..".Username")
                                                                                                         passwd = get_value("InternetGatewayDevice.Services.StorageService.1"..usraccount..".Password")
                                                                                                         
                                                                                                        
                                                                                                
                                                                                                         local f1 = io.open("/etc/passwd","r")
                                                                                                            for cnn in f1:lines() do
                                                                                                               ll = string.match(cnn,username)
                                                                                                           
                                                                                                            end
                                                                                                              print(ll)
                                                                                                               if ll == username then
                                                                                                                   f1:close()
                                                                                                                   conn:close()
                                                                                                               else
                                                                                                                    f1:close()
                                                                                                                    f1 =  io.open("/etc/passwd","a+")  
                                                                                                                    f1:write("\n"..username.."::"..id..":"..id..":Nobody:/:/bin/false")  
                                                                                                                    --print("/bin/echo " ..username.."::"..id..":"..id..":Nobody:/:/bin/false >> /etc/passwd")
                                                                                                                    
                                                                                                               end
                                                                                                             os.execute("/usr/sbin/smbpasswd -a " .. username .." ".. passwd)
                                                                                                         
                                                                                                         
                                                                                                     if  permission == "0"  then
                                                                                                             ii = "read list=NULL,\n\twrite list=NULL,\n\tvalid users=NULL,\n"
                                                                                                             f:write("\t"..ii.."\n")
                                                                                                                     
                                                                                                     elseif  permission == "4" then
                                                                                                              ii = "read list=NULL,"..username.."\n\twrite list=NULL,\n\tvalid users=NULL,"..username.."\n"
                                                                                                              f:write("\t"..ii.."\n")
                                                                                               
                                                                                                     else
                                                                                                 
                                                                                                               ii = "read list=NULL,"..username.."\n\twrite list=NULL,"..username.."\n\tvalid users=NULL,"..username.."\n"
                                                                                                               f:write("\t"..ii.."\n")
                                                                                                     end
                                                                                                end
                                                                                           end 
                                                                                       
                                                                                       else
                                                                                                print("No equipment")
                                                                                       end
                                                                                     end
                                                                                  end
                                                                                  
                                                                            end
                                                                      else
                                                                             print(k6 .. "\t" .. tostring(v6))
                                                                      end
                                                                 end
                                                            else
                                                                    print(k5 .. "\t" .. tostring(v5))
                                                            end
                                                      end                          
                                              else
                                                    print("it is not online")
                                              end                               
                                                                                     
                                         end                                                                      
                                   else
                                             print(k2 .. "\t" .. tostring(v2))
                                   end                                                    
                             end
                       else
                                print(k .. "\t" .. tostring(v))
                       end                                                                                    
                 end                                                                                                    
            end                                                                                               
       end              
                   
   



f:close()
conn:close()                                                                                                                                                                                                                                                                                                                                                                                                                                                  
