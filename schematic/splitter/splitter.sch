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
L Connector_Generic:Conn_01x03 P1
U 1 1 5FF4C6F4
P 5100 3600
F 0 "P1" V 5018 3412 50  0000 R CNN
F 1 "Conn_01x03" V 4973 3412 50  0001 R CNN
F 2 "keg01:LC_CONN" H 5100 3600 50  0001 C CNN
F 3 "~" H 5100 3600 50  0001 C CNN
	1    5100 3600
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 P2
U 1 1 5FF4D5DC
P 5600 3600
F 0 "P2" V 5518 3412 50  0000 R CNN
F 1 "Conn_01x03" V 5473 3412 50  0001 R CNN
F 2 "keg01:LC_CONN" H 5600 3600 50  0001 C CNN
F 3 "~" H 5600 3600 50  0001 C CNN
	1    5600 3600
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 P3
U 1 1 5FF4DB37
P 5600 4800
F 0 "P3" V 5518 4980 50  0000 L CNN
F 1 "Conn_01x03" V 5563 4980 50  0001 L CNN
F 2 "keg01:LC_CONN" H 5600 4800 50  0001 C CNN
F 3 "~" H 5600 4800 50  0001 C CNN
	1    5600 4800
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 P4
U 1 1 5FF4E425
P 5100 4800
F 0 "P4" V 5018 4980 50  0000 L CNN
F 1 "Conn_01x03" V 5063 4980 50  0001 L CNN
F 2 "keg01:LC_CONN" H 5100 4800 50  0001 C CNN
F 3 "~" H 5100 4800 50  0001 C CNN
	1    5100 4800
	0    1    1    0   
$EndComp
Text Notes 5700 4900 3    50   ~ 0
WHITE\nRED\nBLACK
Text Notes 5200 3500 1    50   ~ 0
BLACK\nRED\nWHITE
Text Notes 5200 4900 3    50   ~ 0
BLACK\nRED\nWHITE
Text Notes 5700 3500 1    50   ~ 0
WHITE\nRED\nBLACK
Wire Wire Line
	5500 3800 5500 3900
Wire Wire Line
	5100 3800 5100 4000
Wire Wire Line
	5100 4000 4700 4000
Wire Wire Line
	5600 4600 5600 4300
Wire Wire Line
	5600 4300 4700 4300
Wire Wire Line
	4700 4100 5600 4100
Wire Wire Line
	5600 4100 5600 3800
Wire Wire Line
	5100 4600 5100 4200
Wire Wire Line
	5100 4200 4700 4200
Wire Wire Line
	5500 3900 5200 3900
Wire Wire Line
	5200 3900 5200 3800
Text Label 5250 3900 0    50   ~ 0
WH1
Wire Wire Line
	5700 3800 5700 4600
Wire Wire Line
	5200 4600 5200 4500
Wire Wire Line
	5200 4500 5500 4500
Wire Wire Line
	5500 4500 5500 4600
Text Label 5250 4500 0    50   ~ 0
WH2
Text Label 5700 4250 1    50   ~ 0
BL1
Wire Wire Line
	5000 3800 5000 4600
Text Label 5000 4550 1    50   ~ 0
BL2
Text Label 5600 4550 1    50   ~ 0
E-
Text Label 5100 3950 1    50   ~ 0
E+
Text Label 5600 3950 1    50   ~ 0
A-
Text Label 5100 4550 1    50   ~ 0
A+
$Comp
L Connector_Generic:Conn_01x04 P5
U 1 1 605CD83E
P 4500 4200
F 0 "P5" H 4500 3750 50  0000 C CNN
F 1 "Conn_01x04" H 4550 3850 50  0000 C CNN
F 2 "keg01:DISP_CONN" H 4500 4200 50  0001 C CNN
F 3 "~" H 4500 4200 50  0001 C CNN
	1    4500 4200
	-1   0    0    1   
$EndComp
Text Notes 4400 4300 2    50   ~ 0
E+\nA-\nA+\nE-
$EndSCHEMATC
