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
L Connector:USB_B J?
U 1 1 605F6CB1
P 2200 2700
F 0 "J?" H 2257 3167 50  0000 C CNN
F 1 "USB_B" H 2257 3076 50  0000 C CNN
F 2 "" H 2350 2650 50  0001 C CNN
F 3 " ~" H 2350 2650 50  0001 C CNN
	1    2200 2700
	1    0    0    -1  
$EndComp
$Comp
L Device:CP_Small C?
U 1 1 605F7CEA
P 3500 2800
F 0 "C?" H 3588 2846 50  0000 L CNN
F 1 "220u/10V" H 3588 2755 50  0000 L CNN
F 2 "" H 3500 2800 50  0001 C CNN
F 3 "~" H 3500 2800 50  0001 C CNN
	1    3500 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:CP_Small C?
U 1 1 605F89E2
P 4500 2800
F 0 "C?" H 4588 2846 50  0000 L CNN
F 1 "220u/10V" H 4588 2755 50  0000 L CNN
F 2 "" H 4500 2800 50  0001 C CNN
F 3 "~" H 4500 2800 50  0001 C CNN
	1    4500 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 2500 2900 2500
Wire Wire Line
	3500 2500 3500 2700
$Comp
L Device:L_Core_Ferrite_Small L?
U 1 1 605FA5BB
P 4000 2500
F 0 "L?" V 4205 2500 50  0000 C CNN
F 1 "L_Core_Ferrite_Small" V 4114 2500 50  0000 C CNN
F 2 "" H 4000 2500 50  0001 C CNN
F 3 "~" H 4000 2500 50  0001 C CNN
	1    4000 2500
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3900 2500 3500 2500
Connection ~ 3500 2500
Wire Wire Line
	4100 2500 4500 2500
Wire Wire Line
	4500 2500 4500 2700
$Comp
L my_device:UniTSV D?
U 1 1 605FC739
P 5000 2800
F 0 "D?" H 5068 2830 40  0000 L CNN
F 1 "SMAJ5.0A" H 5068 2762 30  0000 L CNN
F 2 "" V 4900 2810 60  0000 C CNN
F 3 "" V 4900 2810 60  0000 C CNN
	1    5000 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 605FD6B2
P 3300 2800
F 0 "C?" H 3392 2846 50  0000 L CNN
F 1 "100n" H 3392 2755 50  0000 L CNN
F 2 "" H 3300 2800 50  0001 C CNN
F 3 "~" H 3300 2800 50  0001 C CNN
	1    3300 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 605FDF4B
P 4700 2800
F 0 "C?" H 4792 2846 50  0000 L CNN
F 1 "100n" H 4792 2755 50  0000 L CNN
F 2 "" H 4700 2800 50  0001 C CNN
F 3 "~" H 4700 2800 50  0001 C CNN
	1    4700 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 2500 4700 2500
Wire Wire Line
	4700 2500 4700 2700
Connection ~ 4500 2500
Wire Wire Line
	4700 2500 5000 2500
Wire Wire Line
	5000 2500 5000 2700
Connection ~ 4700 2500
Wire Wire Line
	3300 2700 3300 2500
Connection ~ 3300 2500
Wire Wire Line
	3300 2500 3500 2500
$Comp
L power:GND #PWR?
U 1 1 605FEA72
P 2200 3200
F 0 "#PWR?" H 2200 2950 50  0001 C CNN
F 1 "GND" H 2205 3027 50  0000 C CNN
F 2 "" H 2200 3200 50  0001 C CNN
F 3 "" H 2200 3200 50  0001 C CNN
	1    2200 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 605FEEA3
P 3300 3200
F 0 "#PWR?" H 3300 2950 50  0001 C CNN
F 1 "GND" H 3305 3027 50  0000 C CNN
F 2 "" H 3300 3200 50  0001 C CNN
F 3 "" H 3300 3200 50  0001 C CNN
	1    3300 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 605FF2AF
P 3500 3200
F 0 "#PWR?" H 3500 2950 50  0001 C CNN
F 1 "GND" H 3505 3027 50  0000 C CNN
F 2 "" H 3500 3200 50  0001 C CNN
F 3 "" H 3500 3200 50  0001 C CNN
	1    3500 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 605FF4CF
P 4500 3200
F 0 "#PWR?" H 4500 2950 50  0001 C CNN
F 1 "GND" H 4505 3027 50  0000 C CNN
F 2 "" H 4500 3200 50  0001 C CNN
F 3 "" H 4500 3200 50  0001 C CNN
	1    4500 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 605FF99F
P 4700 3200
F 0 "#PWR?" H 4700 2950 50  0001 C CNN
F 1 "GND" H 4705 3027 50  0000 C CNN
F 2 "" H 4700 3200 50  0001 C CNN
F 3 "" H 4700 3200 50  0001 C CNN
	1    4700 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 605FFB98
P 5000 3200
F 0 "#PWR?" H 5000 2950 50  0001 C CNN
F 1 "GND" H 5005 3027 50  0000 C CNN
F 2 "" H 5000 3200 50  0001 C CNN
F 3 "" H 5000 3200 50  0001 C CNN
	1    5000 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 2900 5000 3200
Wire Wire Line
	4700 3200 4700 2900
Wire Wire Line
	4500 2900 4500 3200
Wire Wire Line
	3500 3200 3500 2900
Wire Wire Line
	3300 2900 3300 3200
Wire Wire Line
	2200 3100 2200 3200
$Comp
L Device:LED D?
U 1 1 606047ED
P 2900 2750
F 0 "D?" V 2939 2633 50  0000 R CNN
F 1 "LED" V 2848 2633 50  0000 R CNN
F 2 "" H 2900 2750 50  0001 C CNN
F 3 "~" H 2900 2750 50  0001 C CNN
	1    2900 2750
	0    -1   -1   0   
$EndComp
$Comp
L Device:R_Small R?
U 1 1 6060549B
P 2900 3000
F 0 "R?" H 2959 3046 50  0000 L CNN
F 1 "R_Small" H 2959 2955 50  0000 L CNN
F 2 "" H 2900 3000 50  0001 C CNN
F 3 "~" H 2900 3000 50  0001 C CNN
	1    2900 3000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 60605C2E
P 2900 3200
F 0 "#PWR?" H 2900 2950 50  0001 C CNN
F 1 "GND" H 2905 3027 50  0000 C CNN
F 2 "" H 2900 3200 50  0001 C CNN
F 3 "" H 2900 3200 50  0001 C CNN
	1    2900 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 3100 2900 3200
Wire Wire Line
	2900 2600 2900 2500
Connection ~ 2900 2500
Wire Wire Line
	2900 2500 3300 2500
$Comp
L Connector_Generic:Conn_01x02 J?
U 1 1 60606973
P 6200 2600
F 0 "J?" H 6118 2275 50  0000 C CNN
F 1 "Conn_01x02" H 6118 2366 50  0000 C CNN
F 2 "" H 6200 2600 50  0001 C CNN
F 3 "~" H 6200 2600 50  0001 C CNN
	1    6200 2600
	1    0    0    1   
$EndComp
Wire Wire Line
	5000 2500 6000 2500
Connection ~ 5000 2500
$Comp
L power:GND #PWR?
U 1 1 606079D3
P 5900 3200
F 0 "#PWR?" H 5900 2950 50  0001 C CNN
F 1 "GND" H 5905 3027 50  0000 C CNN
F 2 "" H 5900 3200 50  0001 C CNN
F 3 "" H 5900 3200 50  0001 C CNN
	1    5900 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 3200 5900 2600
Wire Wire Line
	5900 2600 6000 2600
$EndSCHEMATC
