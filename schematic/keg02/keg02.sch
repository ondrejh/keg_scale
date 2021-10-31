EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L keg02-rescue:ESP-12E-ESP8266 IC1
U 1 1 5FF2C7EC
P 4300 5800
F 0 "IC1" H 4300 6565 50  0000 C CNN
F 1 "ESP-12E" H 4300 6474 50  0000 C CNN
F 2 "ESP8266:ESP-12E_SMD" H 4300 5800 50  0001 C CNN
F 3 "http://l0l.org.uk/2014/12/esp8266-modules-hardware-guide-gotta-catch-em-all/" H 4300 5800 50  0001 C CNN
	1    4300 5800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R2
U 1 1 5FF2DF8C
P 3300 5300
F 0 "R2" H 3359 5346 50  0000 L CNN
F 1 "10k" H 3359 5255 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" H 3300 5300 50  0001 C CNN
F 3 "~" H 3300 5300 50  0001 C CNN
	1    3300 5300
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R1
U 1 1 5FF2E49F
P 3000 5300
F 0 "R1" H 2850 5350 50  0000 L CNN
F 1 "10k" H 2800 5250 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" H 3000 5300 50  0001 C CNN
F 3 "~" H 3000 5300 50  0001 C CNN
	1    3000 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 5500 3300 5500
Wire Wire Line
	3300 5500 3300 5400
$Comp
L Device:R_Small R6
U 1 1 5FF2EFFB
P 5600 5300
F 0 "R6" H 5659 5346 50  0000 L CNN
F 1 "10k" H 5659 5255 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" H 5600 5300 50  0001 C CNN
F 3 "~" H 5600 5300 50  0001 C CNN
	1    5600 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 5400 5600 5900
Wire Wire Line
	5600 5900 5200 5900
$Comp
L power:GND #PWR0101
U 1 1 5FF2F779
P 5300 6800
F 0 "#PWR0101" H 5300 6550 50  0001 C CNN
F 1 "GND" H 5305 6627 50  0000 C CNN
F 2 "" H 5300 6800 50  0001 C CNN
F 3 "" H 5300 6800 50  0001 C CNN
	1    5300 6800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R7
U 1 1 5FF2FDFC
P 5600 6400
F 0 "R7" H 5659 6446 50  0000 L CNN
F 1 "10k" H 5659 6355 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" H 5600 6400 50  0001 C CNN
F 3 "~" H 5600 6400 50  0001 C CNN
	1    5600 6400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5FF3081E
P 5600 6800
F 0 "#PWR0102" H 5600 6550 50  0001 C CNN
F 1 "GND" H 5605 6627 50  0000 C CNN
F 2 "" H 5600 6800 50  0001 C CNN
F 3 "" H 5600 6800 50  0001 C CNN
	1    5600 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 6100 5600 6100
Wire Wire Line
	5600 6100 5600 6300
Wire Wire Line
	5200 6200 5300 6200
Wire Wire Line
	5300 6200 5300 6800
Wire Wire Line
	5600 6800 5600 6500
$Comp
L power:+3V3 #PWR0103
U 1 1 5FF315F4
P 5600 5100
F 0 "#PWR0103" H 5600 4950 50  0001 C CNN
F 1 "+3V3" H 5615 5273 50  0000 C CNN
F 2 "" H 5600 5100 50  0001 C CNN
F 3 "" H 5600 5100 50  0001 C CNN
	1    5600 5100
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0104
U 1 1 5FF31BB1
P 3300 5100
F 0 "#PWR0104" H 3300 4950 50  0001 C CNN
F 1 "+3V3" H 3315 5273 50  0000 C CNN
F 2 "" H 3300 5100 50  0001 C CNN
F 3 "" H 3300 5100 50  0001 C CNN
	1    3300 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 5200 5600 5100
Wire Wire Line
	3300 5200 3300 5100
$Comp
L Connector_Generic:Conn_01x06 X1
U 1 1 5FF333FF
P 3700 3600
F 0 "X1" H 3700 3150 50  0000 C CNN
F 1 "NA" H 3700 3950 50  0000 C CNN
F 2 "keg01:PROG_CON" H 3700 3600 50  0001 C CNN
F 3 "~" H 3700 3600 50  0001 C CNN
	1    3700 3600
	1    0    0    1   
$EndComp
Text GLabel 5700 5500 2    35   Output ~ 0
TXD
Text GLabel 5700 5600 2    35   Input ~ 0
RXD
Wire Wire Line
	5200 5500 5700 5500
Wire Wire Line
	5700 5600 5200 5600
Text GLabel 5700 5900 2    35   Input ~ 0
GPIO0
Wire Wire Line
	5700 5900 5600 5900
Connection ~ 5600 5900
Text GLabel 2500 5500 0    35   Input ~ 0
RST
Wire Wire Line
	2500 5500 3300 5500
Connection ~ 3300 5500
$Comp
L power:GND #PWR0106
U 1 1 5FF38459
P 2700 3900
F 0 "#PWR0106" H 2700 3650 50  0001 C CNN
F 1 "GND" H 2705 3727 50  0000 C CNN
F 2 "" H 2700 3900 50  0001 C CNN
F 3 "" H 2700 3900 50  0001 C CNN
	1    2700 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 3800 2700 3800
Wire Wire Line
	2700 3800 2700 3900
$Comp
L power:+3V3 #PWR0107
U 1 1 5FF38C19
P 2700 3300
F 0 "#PWR0107" H 2700 3150 50  0001 C CNN
F 1 "+3V3" H 2715 3473 50  0000 C CNN
F 2 "" H 2700 3300 50  0001 C CNN
F 3 "" H 2700 3300 50  0001 C CNN
	1    2700 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 3700 2700 3700
Wire Wire Line
	2700 3700 2700 3600
Text GLabel 3250 3600 0    35   Output ~ 0
RST
Text GLabel 3250 3500 0    35   Output ~ 0
GPIO0
Text GLabel 3250 3400 0    35   Output ~ 0
RXD
Text GLabel 3250 3300 0    35   Input ~ 0
TXD
Wire Wire Line
	3250 3600 3500 3600
Wire Wire Line
	3500 3500 3250 3500
Wire Wire Line
	3250 3400 3500 3400
Wire Wire Line
	3500 3300 3250 3300
$Comp
L Device:R_Small R4
U 1 1 5FF3F9C1
P 2700 3500
F 0 "R4" H 2759 3546 50  0000 L CNN
F 1 "NA" H 2759 3455 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" H 2700 3500 50  0001 C CNN
F 3 "~" H 2700 3500 50  0001 C CNN
	1    2700 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 3400 2700 3300
Text GLabel 3200 5900 0    35   Input ~ 0
LOADCELL_DOUT
Wire Wire Line
	3200 5900 3400 5900
Text GLabel 3200 6000 0    35   Output ~ 0
LOADCELL_SCK
Wire Wire Line
	3200 6000 3400 6000
Text GLabel 6100 5700 2    35   Output ~ 0
DISP_SCL
Text GLabel 6100 5800 2    35   BiDi ~ 0
DISP_SDA
Wire Wire Line
	6100 5700 5200 5700
Wire Wire Line
	5200 5800 6100 5800
$Comp
L power:GND #PWR0108
U 1 1 5FF4ABC3
P 9500 3100
F 0 "#PWR0108" H 9500 2850 50  0001 C CNN
F 1 "GND" H 9505 2927 50  0000 C CNN
F 2 "" H 9500 3100 50  0001 C CNN
F 3 "" H 9500 3100 50  0001 C CNN
	1    9500 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	9600 3000 9500 3000
Wire Wire Line
	9500 3000 9500 3100
Wire Wire Line
	9600 2700 9500 2700
Text GLabel 8900 2800 0    35   Output ~ 0
LOADCELL_DOUT
Text GLabel 8900 2900 0    35   Input ~ 0
LOADCELL_SCK
Wire Wire Line
	8900 2800 9100 2800
Wire Wire Line
	9600 2900 9300 2900
Text Notes 3800 3800 0    50   ~ 0
TXD\nRXD\nGPIO0\nRST\n3V3\nGND
$Comp
L Device:R_Small R8
U 1 1 5FF765FA
P 9200 2800
F 0 "R8" V 9004 2800 50  0000 C CNN
F 1 "100" V 9095 2800 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9200 2800 50  0001 C CNN
F 3 "~" H 9200 2800 50  0001 C CNN
	1    9200 2800
	0    1    1    0   
$EndComp
Wire Wire Line
	9300 2800 9600 2800
$Comp
L Device:R_Small R9
U 1 1 5FF77577
P 9200 2900
F 0 "R9" V 9400 2900 50  0000 C CNN
F 1 "100" V 9300 2900 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9200 2900 50  0001 C CNN
F 3 "~" H 9200 2900 50  0001 C CNN
	1    9200 2900
	0    1    1    0   
$EndComp
Wire Wire Line
	9100 2900 8900 2900
$Comp
L Device:R_Small R5
U 1 1 5FF77982
P 5300 5300
F 0 "R5" H 5359 5346 50  0000 L CNN
F 1 "10k" H 5359 5255 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" H 5300 5300 50  0001 C CNN
F 3 "~" H 5300 5300 50  0001 C CNN
	1    5300 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 6000 5300 6000
Wire Wire Line
	5300 6000 5300 5400
$Comp
L Device:C_Small C13
U 1 1 5FF7AFD8
P 9100 2100
F 0 "C13" H 9200 2050 50  0000 L CNN
F 1 "100n" H 9150 1950 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 9100 2100 50  0001 C CNN
F 3 "~" H 9100 2100 50  0001 C CNN
	1    9100 2100
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C12
U 1 1 5FF7CE34
P 8800 2100
F 0 "C12" H 8600 2050 50  0000 L CNN
F 1 "4u7" H 8600 1950 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 8800 2100 50  0001 C CNN
F 3 "~" H 8800 2100 50  0001 C CNN
	1    8800 2100
	1    0    0    -1  
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB2
U 1 1 5FF7DDDA
P 8600 1900
F 0 "FB2" V 8363 1900 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 8454 1900 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 8530 1900 50  0001 C CNN
F 3 "~" H 8600 1900 50  0001 C CNN
	1    8600 1900
	0    1    1    0   
$EndComp
Wire Wire Line
	8700 1900 8800 1900
Wire Wire Line
	8800 1900 8800 2000
Wire Wire Line
	8800 1900 9100 1900
Wire Wire Line
	9100 1900 9100 2000
Connection ~ 8800 1900
Wire Wire Line
	9100 1900 9500 1900
Connection ~ 9100 1900
Wire Wire Line
	9500 1900 9500 2700
$Comp
L power:GND #PWR0110
U 1 1 5FF8452E
P 9100 2300
F 0 "#PWR0110" H 9100 2050 50  0001 C CNN
F 1 "GND" H 9105 2127 50  0000 C CNN
F 2 "" H 9100 2300 50  0001 C CNN
F 3 "" H 9100 2300 50  0001 C CNN
	1    9100 2300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0111
U 1 1 5FF848DB
P 8800 2300
F 0 "#PWR0111" H 8800 2050 50  0001 C CNN
F 1 "GND" H 8805 2127 50  0000 C CNN
F 2 "" H 8800 2300 50  0001 C CNN
F 3 "" H 8800 2300 50  0001 C CNN
	1    8800 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	8800 2200 8800 2300
Wire Wire Line
	9100 2200 9100 2300
$Comp
L power:+3V3 #PWR0114
U 1 1 5FF943DD
P 8100 1800
F 0 "#PWR0114" H 8100 1650 50  0001 C CNN
F 1 "+3V3" H 8115 1973 50  0000 C CNN
F 2 "" H 8100 1800 50  0001 C CNN
F 3 "" H 8100 1800 50  0001 C CNN
	1    8100 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	8100 1800 8100 1900
$Comp
L keg02-rescue:AP2112K-3.3-Regulator_Linear IC3
U 1 1 5FFBFBAB
P 4400 1600
F 0 "IC3" H 4400 1942 50  0000 C CNN
F 1 "AP2112K-3.3" H 4400 1851 50  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23-5" H 4400 1925 50  0001 C CNN
F 3 "https://www.diodes.com/assets/Datasheets/AP2112.pdf" H 4400 1700 50  0001 C CNN
	1    4400 1600
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R3
U 1 1 5FFC11B4
P 3900 1600
F 0 "R3" V 4000 1600 50  0000 C CNN
F 1 "10k" V 4100 1600 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 3900 1600 50  0001 C CNN
F 3 "~" H 3900 1600 50  0001 C CNN
	1    3900 1600
	0    1    1    0   
$EndComp
Wire Wire Line
	3700 1500 3700 1600
Wire Wire Line
	3700 1600 3800 1600
Wire Wire Line
	4000 1600 4100 1600
$Comp
L Device:C_Small C6
U 1 1 5FFC6849
P 2800 1700
F 0 "C6" H 2600 1650 50  0000 L CNN
F 1 "4u7" H 2600 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 2800 1700 50  0001 C CNN
F 3 "~" H 2800 1700 50  0001 C CNN
	1    2800 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 1600 2800 1500
Connection ~ 2800 1500
$Comp
L power:GND #PWR0115
U 1 1 5FFCDD1C
P 4400 2000
F 0 "#PWR0115" H 4400 1750 50  0001 C CNN
F 1 "GND" H 4405 1827 50  0000 C CNN
F 2 "" H 4400 2000 50  0001 C CNN
F 3 "" H 4400 2000 50  0001 C CNN
	1    4400 2000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0116
U 1 1 5FFCE368
P 2800 2000
F 0 "#PWR0116" H 2800 1750 50  0001 C CNN
F 1 "GND" H 2805 1827 50  0000 C CNN
F 2 "" H 2800 2000 50  0001 C CNN
F 3 "" H 2800 2000 50  0001 C CNN
	1    2800 2000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0117
U 1 1 5FFCE7E8
P 2500 2000
F 0 "#PWR0117" H 2500 1750 50  0001 C CNN
F 1 "GND" H 2505 1827 50  0000 C CNN
F 2 "" H 2500 2000 50  0001 C CNN
F 3 "" H 2500 2000 50  0001 C CNN
	1    2500 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 2000 2500 1800
Wire Wire Line
	2800 1800 2800 2000
Wire Wire Line
	4400 1900 4400 2000
Wire Wire Line
	2500 1600 2500 1500
Wire Wire Line
	2500 1500 2800 1500
$Comp
L Device:C_Small C5
U 1 1 5FFDBE7C
P 2500 1700
F 0 "C5" H 2250 1650 50  0000 L CNN
F 1 "100n" H 2200 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 2500 1700 50  0001 C CNN
F 3 "~" H 2500 1700 50  0001 C CNN
	1    2500 1700
	1    0    0    -1  
$EndComp
Connection ~ 3700 1500
$Comp
L Diode:1N5819 D1
U 1 1 5FFF7E99
P 2150 1500
F 0 "D1" H 2150 1284 50  0000 C CNN
F 1 "B5819W" H 2150 1375 50  0000 C CNN
F 2 "Diodes_SMD:D_SOD-123" H 2150 1325 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88525/1n5817.pdf" H 2150 1500 50  0001 C CNN
	1    2150 1500
	-1   0    0    1   
$EndComp
Wire Wire Line
	2300 1500 2500 1500
Connection ~ 2500 1500
$Comp
L power:+3V3 #PWR0121
U 1 1 600113DF
P 5500 1400
F 0 "#PWR0121" H 5500 1250 50  0001 C CNN
F 1 "+3V3" H 5515 1573 50  0000 C CNN
F 2 "" H 5500 1400 50  0001 C CNN
F 3 "" H 5500 1400 50  0001 C CNN
	1    5500 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C4
U 1 1 60015263
P 2300 6400
F 0 "C4" H 2400 6350 50  0000 L CNN
F 1 "100n" H 2350 6250 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 2300 6400 50  0001 C CNN
F 3 "~" H 2300 6400 50  0001 C CNN
	1    2300 6400
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C3
U 1 1 6001526D
P 2000 6400
F 0 "C3" H 1800 6350 50  0000 L CNN
F 1 "4u7" H 1800 6250 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 2000 6400 50  0001 C CNN
F 3 "~" H 2000 6400 50  0001 C CNN
	1    2000 6400
	1    0    0    -1  
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB1
U 1 1 60015277
P 1800 6200
F 0 "FB1" V 1563 6200 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 1654 6200 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 1730 6200 50  0001 C CNN
F 3 "~" H 1800 6200 50  0001 C CNN
	1    1800 6200
	0    1    1    0   
$EndComp
Wire Wire Line
	1900 6200 2000 6200
Wire Wire Line
	2000 6200 2000 6300
Wire Wire Line
	2000 6200 2300 6200
Wire Wire Line
	2300 6200 2300 6300
Connection ~ 2000 6200
Connection ~ 2300 6200
$Comp
L power:GND #PWR0122
U 1 1 60015288
P 2300 6600
F 0 "#PWR0122" H 2300 6350 50  0001 C CNN
F 1 "GND" H 2305 6427 50  0000 C CNN
F 2 "" H 2300 6600 50  0001 C CNN
F 3 "" H 2300 6600 50  0001 C CNN
	1    2300 6600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0123
U 1 1 60015292
P 2000 6600
F 0 "#PWR0123" H 2000 6350 50  0001 C CNN
F 1 "GND" H 2005 6427 50  0000 C CNN
F 2 "" H 2000 6600 50  0001 C CNN
F 3 "" H 2000 6600 50  0001 C CNN
	1    2000 6600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 6500 2000 6600
Wire Wire Line
	2300 6500 2300 6600
$Comp
L power:+3V3 #PWR0126
U 1 1 600152CD
P 1300 6100
F 0 "#PWR0126" H 1300 5950 50  0001 C CNN
F 1 "+3V3" H 1315 6273 50  0000 C CNN
F 2 "" H 1300 6100 50  0001 C CNN
F 3 "" H 1300 6100 50  0001 C CNN
	1    1300 6100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 6100 1300 6200
Wire Wire Line
	2300 6200 3400 6200
$Comp
L Connector_Generic:Conn_01x02 P5
U 1 1 6002E3BC
P 1500 1600
F 0 "P5" H 1418 1275 50  0000 C CNN
F 1 "Conn_01x02" H 1418 1366 50  0000 C CNN
F 2 "keg01:PWR_CONN" H 1500 1600 50  0001 C CNN
F 3 "~" H 1500 1600 50  0001 C CNN
	1    1500 1600
	-1   0    0    1   
$EndComp
Wire Wire Line
	1700 1500 2000 1500
$Comp
L power:GND #PWR0127
U 1 1 600334B3
P 1800 2000
F 0 "#PWR0127" H 1800 1750 50  0001 C CNN
F 1 "GND" H 1805 1827 50  0000 C CNN
F 2 "" H 1800 2000 50  0001 C CNN
F 3 "" H 1800 2000 50  0001 C CNN
	1    1800 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	1700 1600 1800 1600
Wire Wire Line
	1800 1600 1800 2000
$Comp
L Connector_Generic:Conn_01x04 P6
U 1 1 60077674
P 9800 5000
F 0 "P6" H 9800 4550 50  0000 C CNN
F 1 "Conn_01x04" H 9850 4650 50  0000 C CNN
F 2 "keg01:DISP_CONN" H 9800 5000 50  0001 C CNN
F 3 "~" H 9800 5000 50  0001 C CNN
	1    9800 5000
	1    0    0    1   
$EndComp
Text Notes 9900 5100 0    50   ~ 0
SDA\nSCK\nVCC\nGND
$Comp
L Device:C_Small C17
U 1 1 6007CE0E
P 9100 5200
F 0 "C17" H 9200 5150 50  0000 L CNN
F 1 "100n" H 9150 5050 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 9100 5200 50  0001 C CNN
F 3 "~" H 9100 5200 50  0001 C CNN
	1    9100 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C16
U 1 1 6007CE18
P 8800 5200
F 0 "C16" H 8600 5150 50  0000 L CNN
F 1 "4u7" H 8600 5050 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 8800 5200 50  0001 C CNN
F 3 "~" H 8800 5200 50  0001 C CNN
	1    8800 5200
	1    0    0    -1  
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB3
U 1 1 6007CE22
P 8600 5000
F 0 "FB3" V 8363 5000 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 8454 5000 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 8530 5000 50  0001 C CNN
F 3 "~" H 8600 5000 50  0001 C CNN
	1    8600 5000
	0    1    1    0   
$EndComp
Wire Wire Line
	8700 5000 8800 5000
Wire Wire Line
	8800 5000 8800 5100
Wire Wire Line
	8800 5000 9100 5000
Wire Wire Line
	9100 5000 9100 5100
Connection ~ 8800 5000
$Comp
L power:GND #PWR0128
U 1 1 6007CE32
P 9100 5400
F 0 "#PWR0128" H 9100 5150 50  0001 C CNN
F 1 "GND" H 9105 5227 50  0000 C CNN
F 2 "" H 9100 5400 50  0001 C CNN
F 3 "" H 9100 5400 50  0001 C CNN
	1    9100 5400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0129
U 1 1 6007CE3C
P 8800 5400
F 0 "#PWR0129" H 8800 5150 50  0001 C CNN
F 1 "GND" H 8805 5227 50  0000 C CNN
F 2 "" H 8800 5400 50  0001 C CNN
F 3 "" H 8800 5400 50  0001 C CNN
	1    8800 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8800 5300 8800 5400
Wire Wire Line
	9100 5300 9100 5400
$Comp
L power:+3V3 #PWR0132
U 1 1 6007CE77
P 8100 4900
F 0 "#PWR0132" H 8100 4750 50  0001 C CNN
F 1 "+3V3" H 8115 5073 50  0000 C CNN
F 2 "" H 8100 4900 50  0001 C CNN
F 3 "" H 8100 4900 50  0001 C CNN
	1    8100 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8100 4900 8100 5000
Wire Wire Line
	9600 5000 9100 5000
Connection ~ 9100 5000
$Comp
L power:GND #PWR0133
U 1 1 600A5C6F
P 9500 5400
F 0 "#PWR0133" H 9500 5150 50  0001 C CNN
F 1 "GND" H 9505 5227 50  0000 C CNN
F 2 "" H 9500 5400 50  0001 C CNN
F 3 "" H 9500 5400 50  0001 C CNN
	1    9500 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	9600 5100 9500 5100
Wire Wire Line
	9500 5100 9500 5400
$Comp
L Device:R_Small R11
U 1 1 600AAD22
P 9100 4600
F 0 "R11" V 9300 4600 50  0000 C CNN
F 1 "100" V 9200 4600 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9100 4600 50  0001 C CNN
F 3 "~" H 9100 4600 50  0001 C CNN
	1    9100 4600
	0    1    1    0   
$EndComp
Wire Wire Line
	9600 4900 9400 4900
Wire Wire Line
	9400 4900 9400 4600
Wire Wire Line
	9400 4600 9200 4600
$Comp
L Device:R_Small R10
U 1 1 600B16F6
P 9100 4500
F 0 "R10" V 8904 4500 50  0000 C CNN
F 1 "100" V 8995 4500 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9100 4500 50  0001 C CNN
F 3 "~" H 9100 4500 50  0001 C CNN
	1    9100 4500
	0    1    1    0   
$EndComp
Wire Wire Line
	9600 4800 9500 4800
Wire Wire Line
	9500 4800 9500 4500
Wire Wire Line
	9500 4500 9200 4500
Text GLabel 8800 4500 0    35   BiDi ~ 0
DISP_SDA
Text GLabel 8800 4600 0    35   Input ~ 0
DISP_SCL
Wire Wire Line
	8800 4600 9000 4600
Wire Wire Line
	9000 4500 8800 4500
Text Notes 1400 1650 2    50   ~ 0
V+\nGND
$Comp
L power:GND #PWR0120
U 1 1 5FFF16A5
P 5100 2000
F 0 "#PWR0120" H 5100 1750 50  0001 C CNN
F 1 "GND" H 5105 1827 50  0000 C CNN
F 2 "" H 5100 2000 50  0001 C CNN
F 3 "" H 5100 2000 50  0001 C CNN
	1    5100 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 1800 5100 2000
Wire Wire Line
	5100 1500 5100 1600
$Comp
L Device:C_Small C9
U 1 1 5FFEA924
P 5100 1700
F 0 "C9" H 5200 1650 50  0000 L CNN
F 1 "100n" H 5150 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 5100 1700 50  0001 C CNN
F 3 "~" H 5100 1700 50  0001 C CNN
	1    5100 1700
	1    0    0    -1  
$EndComp
Text Label 1800 1500 0    50   ~ 0
PWIN
Text Label 2550 1500 0    50   ~ 0
PWIN2
Text Label 9200 1900 0    50   ~ 0
PWIC2
Text Label 9150 5000 0    50   ~ 0
PWDISP
Text Label 2450 6200 0    50   ~ 0
PWIC1
Wire Wire Line
	5300 4800 5300 5200
Text Label 5300 5100 1    50   ~ 0
PWDISP
Wire Wire Line
	3000 5400 3000 5700
Wire Wire Line
	3000 5700 3400 5700
Wire Wire Line
	3000 5200 3000 4800
Text Label 3000 5100 1    50   ~ 0
PWIC1
$Comp
L Connector_Generic:Conn_01x04 P1
U 1 1 605CC612
P 9800 2900
F 0 "P1" H 9800 2450 50  0000 C CNN
F 1 "Conn_01x04" H 9850 2550 50  0000 C CNN
F 2 "keg01:DISP_CONN" H 9800 2900 50  0001 C CNN
F 3 "~" H 9800 2900 50  0001 C CNN
	1    9800 2900
	1    0    0    1   
$EndComp
Text Notes 9900 3000 0    50   ~ 0
VCC\nDO / RX\nCK / TX\nGND
$Comp
L Connector_Generic:Conn_01x03 P2
U 1 1 605D2EFE
P 7100 4000
F 0 "P2" V 7018 4180 50  0000 L CNN
F 1 "Conn_01x03" V 7063 4180 50  0001 L CNN
F 2 "keg01:LC_CONN" H 7100 4000 50  0001 C CNN
F 3 "~" H 7100 4000 50  0001 C CNN
	1    7100 4000
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 605D946B
P 6800 4200
F 0 "#PWR0105" H 6800 3950 50  0001 C CNN
F 1 "GND" H 6805 4027 50  0000 C CNN
F 2 "" H 6800 4200 50  0001 C CNN
F 3 "" H 6800 4200 50  0001 C CNN
	1    6800 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 4100 6800 4100
Wire Wire Line
	6800 4100 6800 4200
$Comp
L Device:C_Small C18
U 1 1 605DFD65
P 6100 3300
F 0 "C18" H 6200 3250 50  0000 L CNN
F 1 "100n" H 6150 3150 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 6100 3300 50  0001 C CNN
F 3 "~" H 6100 3300 50  0001 C CNN
	1    6100 3300
	1    0    0    -1  
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB4
U 1 1 605DFD79
P 5600 3100
F 0 "FB4" V 5363 3100 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 5454 3100 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 5530 3100 50  0001 C CNN
F 3 "~" H 5600 3100 50  0001 C CNN
	1    5600 3100
	0    1    1    0   
$EndComp
Wire Wire Line
	5700 3100 5800 3100
Wire Wire Line
	5800 3100 5800 3200
Wire Wire Line
	5800 3100 6100 3100
Wire Wire Line
	6100 3100 6100 3200
Connection ~ 5800 3100
Wire Wire Line
	6100 3100 6500 3100
Connection ~ 6100 3100
Wire Wire Line
	6800 3100 6800 3900
$Comp
L power:GND #PWR0109
U 1 1 605DFD8B
P 6100 3500
F 0 "#PWR0109" H 6100 3250 50  0001 C CNN
F 1 "GND" H 6105 3327 50  0000 C CNN
F 2 "" H 6100 3500 50  0001 C CNN
F 3 "" H 6100 3500 50  0001 C CNN
	1    6100 3500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 605DFD95
P 5800 3500
F 0 "#PWR0113" H 5800 3250 50  0001 C CNN
F 1 "GND" H 5805 3327 50  0000 C CNN
F 2 "" H 5800 3500 50  0001 C CNN
F 3 "" H 5800 3500 50  0001 C CNN
	1    5800 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 3400 5800 3500
Wire Wire Line
	6100 3400 6100 3500
$Comp
L Device:C_Small C10
U 1 1 605DFDA1
P 6500 1700
F 0 "C10" H 6300 1650 50  0000 L CNN
F 1 "4u7" H 6300 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 6500 1700 50  0001 C CNN
F 3 "~" H 6500 1700 50  0001 C CNN
	1    6500 1700
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0125
U 1 1 605DFDBA
P 5100 3000
F 0 "#PWR0125" H 5100 2850 50  0001 C CNN
F 1 "+3V3" H 5115 3173 50  0000 C CNN
F 2 "" H 5100 3000 50  0001 C CNN
F 3 "" H 5100 3000 50  0001 C CNN
	1    5100 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 3000 5100 3100
Text Label 6200 3100 0    50   ~ 0
PWIC3
Wire Wire Line
	6800 3900 6900 3900
$Comp
L Device:R_Small R13
U 1 1 60612DF0
P 6500 3300
F 0 "R13" H 6559 3346 50  0000 L CNN
F 1 "4k7" H 6559 3255 50  0000 L CNN
F 2 "Resistors_SMD:R_0603" H 6500 3300 50  0001 C CNN
F 3 "~" H 6500 3300 50  0001 C CNN
	1    6500 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6500 3100 6500 3200
Connection ~ 6500 3100
Wire Wire Line
	6500 3100 6800 3100
Wire Wire Line
	6500 3400 6500 4000
Wire Wire Line
	6500 4000 6900 4000
$Comp
L Device:R_Small R12
U 1 1 6061F937
P 6200 4000
F 0 "R12" V 6400 4000 50  0000 C CNN
F 1 "100" V 6300 4000 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 6200 4000 50  0001 C CNN
F 3 "~" H 6200 4000 50  0001 C CNN
	1    6200 4000
	0    1    1    0   
$EndComp
Wire Wire Line
	6300 4000 6500 4000
Connection ~ 6500 4000
Wire Wire Line
	6100 4000 5900 4000
Text GLabel 5900 4000 0    35   BiDi ~ 0
TEMP
Text GLabel 3200 6100 0    35   BiDi ~ 0
TEMP
Wire Wire Line
	3200 6100 3400 6100
Text Notes 7200 4100 0    50   ~ 0
VCC\nDQ\nGND
$Comp
L Device:CP_Small C1
U 1 1 6066A3D6
P 3400 1700
F 0 "C1" H 3450 1550 50  0000 L CNN
F 1 "220u / 10V" H 3450 1450 50  0000 L CNN
F 2 "Capacitors_SMD:CP_Elec_6.3x7.7" H 3400 1700 50  0001 C CNN
F 3 "~" H 3400 1700 50  0001 C CNN
	1    3400 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 1600 3400 1500
Connection ~ 3400 1500
Wire Wire Line
	3400 1500 3700 1500
$Comp
L power:GND #PWR0131
U 1 1 6067932B
P 3400 2000
F 0 "#PWR0131" H 3400 1750 50  0001 C CNN
F 1 "GND" H 3405 1827 50  0000 C CNN
F 2 "" H 3400 2000 50  0001 C CNN
F 3 "" H 3400 2000 50  0001 C CNN
	1    3400 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 1800 3400 2000
Wire Wire Line
	5100 1500 5500 1500
Wire Wire Line
	5500 1500 5500 1400
Connection ~ 5100 1500
Wire Wire Line
	5500 1500 5500 1600
Connection ~ 5500 1500
$Comp
L power:GND #PWR0134
U 1 1 60696930
P 5500 2000
F 0 "#PWR0134" H 5500 1750 50  0001 C CNN
F 1 "GND" H 5505 1827 50  0000 C CNN
F 2 "" H 5500 2000 50  0001 C CNN
F 3 "" H 5500 2000 50  0001 C CNN
	1    5500 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 1800 5500 2000
Wire Wire Line
	4700 1500 5100 1500
Wire Wire Line
	3700 1500 4100 1500
Wire Wire Line
	2800 1500 3400 1500
Wire Wire Line
	1300 6200 1700 6200
Wire Wire Line
	8100 5000 8500 5000
$Comp
L power:GND #PWR0112
U 1 1 5FF8D405
P 7000 2000
F 0 "#PWR0112" H 7000 1750 50  0001 C CNN
F 1 "GND" H 7005 1827 50  0000 C CNN
F 2 "" H 7000 2000 50  0001 C CNN
F 3 "" H 7000 2000 50  0001 C CNN
	1    7000 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C11
U 1 1 5FF8C15A
P 7000 1700
F 0 "C11" H 6800 1650 50  0000 L CNN
F 1 "4u7" H 6800 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 7000 1700 50  0001 C CNN
F 3 "~" H 7000 1700 50  0001 C CNN
	1    7000 1700
	1    0    0    -1  
$EndComp
$Comp
L Device:CP_Small C8
U 1 1 6065BDA6
P 5500 1700
F 0 "C8" H 5588 1746 50  0000 L CNN
F 1 "220u / 10V" H 5588 1655 50  0000 L CNN
F 2 "Capacitors_SMD:CP_Elec_6.3x7.7" H 5500 1700 50  0001 C CNN
F 3 "~" H 5500 1700 50  0001 C CNN
	1    5500 1700
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C14
U 1 1 605DFD6F
P 5800 3300
F 0 "C14" H 5600 3250 50  0000 L CNN
F 1 "4u7" H 5600 3150 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206" H 5800 3300 50  0001 C CNN
F 3 "~" H 5800 3300 50  0001 C CNN
	1    5800 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 3100 5500 3100
$Comp
L power:GND #PWR0118
U 1 1 60742BFE
P 6500 2000
F 0 "#PWR0118" H 6500 1750 50  0001 C CNN
F 1 "GND" H 6505 1827 50  0000 C CNN
F 2 "" H 6500 2000 50  0001 C CNN
F 3 "" H 6500 2000 50  0001 C CNN
	1    6500 2000
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0119
U 1 1 60742EF1
P 6500 1400
F 0 "#PWR0119" H 6500 1250 50  0001 C CNN
F 1 "+3V3" H 6515 1573 50  0000 C CNN
F 2 "" H 6500 1400 50  0001 C CNN
F 3 "" H 6500 1400 50  0001 C CNN
	1    6500 1400
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0124
U 1 1 607432E0
P 7000 1400
F 0 "#PWR0124" H 7000 1250 50  0001 C CNN
F 1 "+3V3" H 7015 1573 50  0000 C CNN
F 2 "" H 7000 1400 50  0001 C CNN
F 3 "" H 7000 1400 50  0001 C CNN
	1    7000 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6500 1400 6500 1600
Wire Wire Line
	7000 1400 7000 1600
Wire Wire Line
	7000 1800 7000 2000
Wire Wire Line
	6500 2000 6500 1800
Wire Wire Line
	8100 1900 8500 1900
NoConn ~ 3400 5600
NoConn ~ 3400 5800
NoConn ~ 4050 6700
NoConn ~ 4150 6700
NoConn ~ 4250 6700
NoConn ~ 4350 6700
NoConn ~ 4450 6700
NoConn ~ 4550 6700
$EndSCHEMATC
