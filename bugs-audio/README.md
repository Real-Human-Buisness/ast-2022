# spiffsgen

menuconfig, change partition type to custom, point to partitionTable.csv

```
python ~\esp\esp-idf\components\spiffs\spiffsgen.py 0x80000 ./spiffs/data_1 ./spiffs/img/data_1.bin
python ~\esp\esp-idf\components\esptool_py\esptool\esptool.py --chip esp32 --port COM10 --baud 115200 write_flash -z 0x110000 ./spiffs/img/data_1.bin
```