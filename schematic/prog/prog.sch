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
L Interface_USB:FT231XS U1
U 1 1 5FF6C064
P 5250 4100
F 0 "U1" H 5250 5181 50  0000 C CNN
F 1 "FT231XS" H 5250 5090 50  0000 C CNN
F 2 "keg01:SSOP-20_3.9x8.7mm_P0.635mm" H 6250 3300 50  0001 C CNN
F 3 "https://www.ftdichip.com/Support/Documents/DataSheets/ICs/DS_FT231X.pdf" H 5250 4100 50  0001 C CNN
	1    5250 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 3500 4450 3500
Wire Wire Line
	4450 3500 4450 3150
Wire Wire Line
	4450 3150 5350 3150
Wire Wire Line
	5350 3150 5350 3200
Wire Wire Line
	4550 4100 4450 4100
Wire Wire Line
	4450 4100 4450 3500
Connection ~ 4450 3500
$Comp
L Device:C_Small C6
U 1 1 5FF6E050
P 4450 4300
F 0 "C6" H 4700 4250 50  0000 R CNN
F 1 "100n" H 4750 4350 50  0000 R CNN
F 2 "Capacitors_SMD:C_0603" H 4450 4300 50  0001 C CNN
F 3 "~" H 4450 4300 50  0001 C CNN
	1    4450 4300
	-1   0    0    1   
$EndComp
Wire Wire Line
	4450 4100 4450 4200
Connection ~ 4450 4100
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 5FF6EBC8
P 1050 3100
F 0 "J1" H 968 3417 50  0000 C CNN
F 1 "Conn_01x04" H 968 3326 50  0000 C CNN
F 2 "keg01:SOLDER_CONN_4p" H 1050 3100 50  0001 C CNN
F 3 "~" H 1050 3100 50  0001 C CNN
	1    1050 3100
	-1   0    0    -1  
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB1
U 1 1 5FF70EB8
P 2850 3000
F 0 "FB1" V 2613 3000 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 2704 3000 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2780 3000 50  0001 C CNN
F 3 "~" H 2850 3000 50  0001 C CNN
	1    2850 3000
	0    1    1    0   
$EndComp
Wire Wire Line
	1250 3000 1650 3000
Wire Wire Line
	5150 3000 5150 3200
$Comp
L power:GND #PWR0101
U 1 1 5FF730B9
P 1350 3600
F 0 "#PWR0101" H 1350 3350 50  0001 C CNN
F 1 "GND" H 1355 3427 50  0000 C CNN
F 2 "" H 1350 3600 50  0001 C CNN
F 3 "" H 1350 3600 50  0001 C CNN
	1    1350 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1250 3300 1350 3300
Wire Wire Line
	1350 3300 1350 3600
$Comp
L power:GND #PWR0102
U 1 1 5FF736D3
P 5150 5100
F 0 "#PWR0102" H 5150 4850 50  0001 C CNN
F 1 "GND" H 5155 4927 50  0000 C CNN
F 2 "" H 5150 5100 50  0001 C CNN
F 3 "" H 5150 5100 50  0001 C CNN
	1    5150 5100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5FF73C38
P 5350 5100
F 0 "#PWR0103" H 5350 4850 50  0001 C CNN
F 1 "GND" H 5355 4927 50  0000 C CNN
F 2 "" H 5350 5100 50  0001 C CNN
F 3 "" H 5350 5100 50  0001 C CNN
	1    5350 5100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5FF73E19
P 4450 5100
F 0 "#PWR0104" H 4450 4850 50  0001 C CNN
F 1 "GND" H 4455 4927 50  0000 C CNN
F 2 "" H 4450 5100 50  0001 C CNN
F 3 "" H 4450 5100 50  0001 C CNN
	1    4450 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4450 5100 4450 4400
Wire Wire Line
	5150 5000 5150 5100
Wire Wire Line
	5350 5100 5350 5000
$Comp
L Device:C_Small C1
U 1 1 5FF74879
P 1650 3400
F 0 "C1" H 1550 3350 50  0000 R CNN
F 1 "10n" H 1550 3450 50  0000 R CNN
F 2 "Capacitors_SMD:C_0603" H 1650 3400 50  0001 C CNN
F 3 "~" H 1650 3400 50  0001 C CNN
	1    1650 3400
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 5FF76239
P 1650 3600
F 0 "#PWR0105" H 1650 3350 50  0001 C CNN
F 1 "GND" H 1655 3427 50  0000 C CNN
F 2 "" H 1650 3600 50  0001 C CNN
F 3 "" H 1650 3600 50  0001 C CNN
	1    1650 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 3500 1650 3600
Wire Wire Line
	1650 3300 1650 3000
Connection ~ 1650 3000
Wire Wire Line
	1650 3000 2750 3000
$Comp
L Device:R_Small R1
U 1 1 5FF76B62
P 4250 3800
F 0 "R1" V 4150 3700 50  0000 C CNN
F 1 "27" V 4150 3900 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 4250 3800 50  0001 C CNN
F 3 "~" H 4250 3800 50  0001 C CNN
	1    4250 3800
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R2
U 1 1 5FF77251
P 4250 3900
F 0 "R2" V 4150 4000 50  0000 C CNN
F 1 "27" V 4150 3800 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 4250 3900 50  0001 C CNN
F 3 "~" H 4250 3900 50  0001 C CNN
	1    4250 3900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4350 3900 4550 3900
Wire Wire Line
	4550 3800 4350 3800
Wire Wire Line
	4150 3800 4050 3800
Wire Wire Line
	4050 3800 4050 3100
Wire Wire Line
	4150 3900 3950 3900
Wire Wire Line
	3950 3900 3950 3200
$Comp
L Device:C_Small C2
U 1 1 5FF78694
P 2250 3400
F 0 "C2" H 2450 3350 50  0000 R CNN
F 1 "47p" H 2500 3450 50  0000 R CNN
F 2 "Capacitors_SMD:C_0603" H 2250 3400 50  0001 C CNN
F 3 "~" H 2250 3400 50  0001 C CNN
	1    2250 3400
	-1   0    0    1   
$EndComp
$Comp
L Device:C_Small C3
U 1 1 5FF79098
P 2550 3400
F 0 "C3" H 2450 3350 50  0000 R CNN
F 1 "47p" H 2450 3450 50  0000 R CNN
F 2 "Capacitors_SMD:C_0603" H 2550 3400 50  0001 C CNN
F 3 "~" H 2550 3400 50  0001 C CNN
	1    2550 3400
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5FF7C802
P 2250 3600
F 0 "#PWR0106" H 2250 3350 50  0001 C CNN
F 1 "GND" H 2255 3427 50  0000 C CNN
F 2 "" H 2250 3600 50  0001 C CNN
F 3 "" H 2250 3600 50  0001 C CNN
	1    2250 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5FF7CA50
P 2550 3600
F 0 "#PWR0107" H 2550 3350 50  0001 C CNN
F 1 "GND" H 2555 3427 50  0000 C CNN
F 2 "" H 2550 3600 50  0001 C CNN
F 3 "" H 2550 3600 50  0001 C CNN
	1    2550 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 3500 2250 3600
Wire Wire Line
	2550 3600 2550 3500
Wire Wire Line
	2550 3300 2550 3200
Wire Wire Line
	2550 3200 1250 3200
Wire Wire Line
	2250 3300 2250 3100
Wire Wire Line
	2250 3100 1250 3100
Wire Wire Line
	2950 3000 3150 3000
Wire Wire Line
	2250 3100 4050 3100
Connection ~ 2250 3100
Wire Wire Line
	2550 3200 3950 3200
Connection ~ 2550 3200
$Comp
L Device:C_Small C4
U 1 1 5FF906ED
P 3150 3400
F 0 "C4" H 3350 3400 50  0000 R CNN
F 1 "100n" H 3400 3500 50  0000 R CNN
F 2 "Capacitors_SMD:C_0603" H 3150 3400 50  0001 C CNN
F 3 "~" H 3150 3400 50  0001 C CNN
	1    3150 3400
	-1   0    0    1   
$EndComp
$Comp
L Device:C_Small C5
U 1 1 5FF9117C
P 3450 3400
F 0 "C5" H 3350 3350 50  0000 R CNN
F 1 "4u7" H 3350 3450 50  0000 R CNN
F 2 "Capacitors_SMD:C_1206" H 3450 3400 50  0001 C CNN
F 3 "~" H 3450 3400 50  0001 C CNN
	1    3450 3400
	-1   0    0    1   
$EndComp
Wire Wire Line
	3150 3300 3150 3000
Connection ~ 3150 3000
Wire Wire Line
	3150 3000 3450 3000
Wire Wire Line
	3450 3300 3450 3000
Connection ~ 3450 3000
Wire Wire Line
	3450 3000 5150 3000
$Comp
L power:GND #PWR0108
U 1 1 5FF92460
P 3150 3600
F 0 "#PWR0108" H 3150 3350 50  0001 C CNN
F 1 "GND" H 3155 3427 50  0000 C CNN
F 2 "" H 3150 3600 50  0001 C CNN
F 3 "" H 3150 3600 50  0001 C CNN
	1    3150 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0109
U 1 1 5FF92658
P 3450 3600
F 0 "#PWR0109" H 3450 3350 50  0001 C CNN
F 1 "GND" H 3455 3427 50  0000 C CNN
F 2 "" H 3450 3600 50  0001 C CNN
F 3 "" H 3450 3600 50  0001 C CNN
	1    3450 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 3500 3450 3600
Wire Wire Line
	3150 3600 3150 3500
$Comp
L Connector_Generic:Conn_01x01 J2
U 1 1 5FF9B363
P 10150 3000
F 0 "J2" H 10230 3042 50  0000 L CNN
F 1 "Conn_01x01" H 10230 2951 50  0000 L CNN
F 2 "keg01:POGO_FLAT" H 10150 3000 50  0001 C CNN
F 3 "~" H 10150 3000 50  0001 C CNN
	1    10150 3000
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J3
U 1 1 5FF9BE92
P 10150 3200
F 0 "J3" H 10230 3242 50  0000 L CNN
F 1 "Conn_01x01" H 10230 3151 50  0000 L CNN
F 2 "keg01:POGO_FLAT" H 10150 3200 50  0001 C CNN
F 3 "~" H 10150 3200 50  0001 C CNN
	1    10150 3200
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J4
U 1 1 5FF9CA8B
P 10150 3400
F 0 "J4" H 10230 3442 50  0000 L CNN
F 1 "Conn_01x01" H 10230 3351 50  0000 L CNN
F 2 "keg01:POGO_FLAT" H 10150 3400 50  0001 C CNN
F 3 "~" H 10150 3400 50  0001 C CNN
	1    10150 3400
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J5
U 1 1 5FF9CD09
P 10150 3600
F 0 "J5" H 10230 3642 50  0000 L CNN
F 1 "Conn_01x01" H 10230 3551 50  0000 L CNN
F 2 "keg01:POGO_FLAT" H 10150 3600 50  0001 C CNN
F 3 "~" H 10150 3600 50  0001 C CNN
	1    10150 3600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J6
U 1 1 5FF9CF8D
P 10150 3800
F 0 "J6" H 10230 3842 50  0000 L CNN
F 1 "Conn_01x01" H 10230 3751 50  0000 L CNN
F 2 "keg01:POGO_FLAT" H 10150 3800 50  0001 C CNN
F 3 "~" H 10150 3800 50  0001 C CNN
	1    10150 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0110
U 1 1 5FF9D23A
P 9850 3900
F 0 "#PWR0110" H 9850 3650 50  0001 C CNN
F 1 "GND" H 9855 3727 50  0000 C CNN
F 2 "" H 9850 3900 50  0001 C CNN
F 3 "" H 9850 3900 50  0001 C CNN
	1    9850 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9950 3800 9850 3800
Wire Wire Line
	9850 3800 9850 3900
Wire Wire Line
	9950 3000 9550 3000
Wire Wire Line
	6050 3000 6050 3500
Wire Wire Line
	6050 3500 5950 3500
Wire Wire Line
	5950 3600 6150 3600
Wire Wire Line
	6150 3600 6150 3200
$Comp
L Device:R_Small R5
U 1 1 5FF9F652
P 9450 3000
F 0 "R5" V 9350 2900 50  0000 C CNN
F 1 "100" V 9350 3100 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9450 3000 50  0001 C CNN
F 3 "~" H 9450 3000 50  0001 C CNN
	1    9450 3000
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R6
U 1 1 5FF9FFAB
P 9450 3200
F 0 "R6" V 9350 3100 50  0000 C CNN
F 1 "100" V 9350 3300 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9450 3200 50  0001 C CNN
F 3 "~" H 9450 3200 50  0001 C CNN
	1    9450 3200
	0    1    1    0   
$EndComp
Wire Wire Line
	9550 3200 9950 3200
$Comp
L Transistor_BJT:BC847 Q1
U 1 1 5FFA0517
P 8350 3700
F 0 "Q1" H 8541 3746 50  0000 L CNN
F 1 "BC847" H 8541 3655 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 8550 3625 50  0001 L CIN
F 3 "http://www.infineon.com/dgdl/Infineon-BC847SERIES_BC848SERIES_BC849SERIES_BC850SERIES-DS-v01_01-en.pdf?fileId=db3a304314dca389011541d4630a1657" H 8350 3700 50  0001 L CNN
	1    8350 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6050 3000 9350 3000
Wire Wire Line
	6150 3200 9350 3200
Wire Wire Line
	8450 3500 8450 3400
Wire Wire Line
	8450 3400 9350 3400
$Comp
L Transistor_BJT:BC847 Q2
U 1 1 5FFAA1BE
P 8850 4300
F 0 "Q2" H 9041 4346 50  0000 L CNN
F 1 "BC847" H 9041 4255 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 9050 4225 50  0001 L CIN
F 3 "http://www.infineon.com/dgdl/Infineon-BC847SERIES_BC848SERIES_BC849SERIES_BC850SERIES-DS-v01_01-en.pdf?fileId=db3a304314dca389011541d4630a1657" H 8850 4300 50  0001 L CNN
	1    8850 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	8950 3600 9350 3600
$Comp
L Device:R_Small R3
U 1 1 5FFAF389
P 7950 3700
F 0 "R3" V 7850 3600 50  0000 C CNN
F 1 "10k" V 7850 3800 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 7950 3700 50  0001 C CNN
F 3 "~" H 7950 3700 50  0001 C CNN
	1    7950 3700
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R4
U 1 1 5FFAF8C3
P 8450 4300
F 0 "R4" V 8350 4200 50  0000 C CNN
F 1 "10k" V 8350 4400 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 8450 4300 50  0001 C CNN
F 3 "~" H 8450 4300 50  0001 C CNN
	1    8450 4300
	0    1    1    0   
$EndComp
Wire Wire Line
	8050 3700 8150 3700
Wire Wire Line
	8650 4300 8550 4300
Wire Wire Line
	8950 4600 7450 4600
Wire Wire Line
	7450 4600 7450 3900
Wire Wire Line
	7450 3700 7850 3700
Wire Wire Line
	5950 3900 7450 3900
Connection ~ 7450 3900
Wire Wire Line
	7450 3900 7450 3700
Wire Wire Line
	8450 3900 8450 4000
Wire Wire Line
	8450 4000 7250 4000
Wire Wire Line
	7250 4000 7250 4300
Wire Wire Line
	7250 4300 8350 4300
Wire Wire Line
	5950 3700 7250 3700
Wire Wire Line
	7250 3700 7250 4000
Connection ~ 7250 4000
Text Label 7600 3700 0    50   ~ 0
DTR
Text Label 8100 4300 0    50   ~ 0
RTS
$Comp
L Device:R_Small R7
U 1 1 5FFBD6AA
P 9450 3400
F 0 "R7" V 9350 3300 50  0000 C CNN
F 1 "100" V 9350 3500 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9450 3400 50  0001 C CNN
F 3 "~" H 9450 3400 50  0001 C CNN
	1    9450 3400
	0    1    1    0   
$EndComp
Wire Wire Line
	9550 3400 9950 3400
$Comp
L Device:R_Small R8
U 1 1 5FFBD8BD
P 9450 3600
F 0 "R8" V 9350 3500 50  0000 C CNN
F 1 "100" V 9350 3700 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" H 9450 3600 50  0001 C CNN
F 3 "~" H 9450 3600 50  0001 C CNN
	1    9450 3600
	0    1    1    0   
$EndComp
Wire Wire Line
	9550 3600 9950 3600
Text Label 9000 3000 0    50   ~ 0
TXD
Text Label 9000 3200 0    50   ~ 0
RXD
Text Label 9000 3400 0    50   ~ 0
RST
Text Label 9000 3600 0    50   ~ 0
GPIO0
Wire Wire Line
	8950 4600 8950 4500
Wire Wire Line
	8950 3600 8950 4100
Text Label 8200 4000 0    50   ~ 0
RTS
Text Label 8750 4600 0    50   ~ 0
DTR
Text Label 9650 3600 0    50   ~ 0
_GPIO0
Text Label 9650 3400 0    50   ~ 0
_RST
Text Label 9650 3200 0    50   ~ 0
_RXD
Text Label 9650 3000 0    50   ~ 0
_TXD
$Comp
L Mechanical:MountingHole H1
U 1 1 5FF6B7C5
P 3000 1500
F 0 "H1" H 3100 1546 50  0000 L CNN
F 1 "MountingHole" H 3100 1455 50  0000 L CNN
F 2 "Mounting_Holes:MountingHole_2.2mm_M2" H 3000 1500 50  0001 C CNN
F 3 "~" H 3000 1500 50  0001 C CNN
	1    3000 1500
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 5FF6C3F2
P 3000 1750
F 0 "H2" H 3100 1796 50  0000 L CNN
F 1 "MountingHole" H 3100 1705 50  0000 L CNN
F 2 "Mounting_Holes:MountingHole_2.2mm_M2" H 3000 1750 50  0001 C CNN
F 3 "~" H 3000 1750 50  0001 C CNN
	1    3000 1750
	1    0    0    -1  
$EndComp
$EndSCHEMATC
