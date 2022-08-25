#Example for running OpenSeaChest_Logs and OpenSeaChest_LogParser together on one cmd line 

sudo ./openSeaChest_Logs -d /dev/sg? --farm --logMode pipe | ./openSeaChest_LogParser_x86_64 --inputLog fromPipe --logType farmLog --printType json

