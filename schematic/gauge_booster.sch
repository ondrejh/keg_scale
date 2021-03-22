EESchema Schematic File Version 4
EELAYER 26 0
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
L Transistor_BJT:BC338 Q1
U 1 1 6058ABF2
P 5000 3600
F 0 "Q1" H 5191 3646 50  0000 L CNN
F 1 "BC338" H 5191 3555 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 5200 3525 50  0001 L CIN
F 3 "http://diotec.com/tl_files/diotec/files/pdf/datasheets/bc337" H 5000 3600 50  0001 L CNN
	1    5000 3600
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R2
U 1 1 6058AC61
P 5100 4000
F 0 "R2" H 5159 4046 50  0000 L CNN
F 1 "22R" H 5159 3955 50  0000 L CNN
F 2 "" H 5100 4000 50  0001 C CNN
F 3 "~" H 5100 4000 50  0001 C CNN
	1    5100 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 6058AD34
P 5100 4200
F 0 "#PWR01" H 5100 3950 50  0001 C CNN
F 1 "GND" H 5105 4027 50  0000 C CNN
F 2 "" H 5100 4200 50  0001 C CNN
F 3 "" H 5100 4200 50  0001 C CNN
	1    5100 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 4100 5100 4200
Wire Wire Line
	5100 3800 5100 3900
Wire Wire Line
	5100 3400 5100 3300
$Comp
L Device:CP_Small C1
U 1 1 6058AE5D
P 5100 3100
F 0 "C1" H 5188 3146 50  0000 L CNN
F 1 "47u" H 5188 3055 50  0000 L CNN
F 2 "" H 5100 3100 50  0001 C CNN
F 3 "~" H 5100 3100 50  0001 C CNN
	1    5100 3100
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R1
U 1 1 6058AF90
P 4600 3600
F 0 "R1" V 4404 3600 50  0000 C CNN
F 1 "1k0" V 4495 3600 50  0000 C CNN
F 2 "" H 4600 3600 50  0001 C CNN
F 3 "~" H 4600 3600 50  0001 C CNN
	1    4600 3600
	0    1    1    0   
$EndComp
Wire Wire Line
	4700 3600 4800 3600
Wire Wire Line
	4500 3600 2600 3600
Text GLabel 2600 3600 0    50   Input ~ 0
PWM
Wire Wire Line
	5100 3300 5700 3300
Wire Wire Line
	5100 3200 5100 3300
Connection ~ 5100 3300
Wire Wire Line
	5100 3000 5100 2900
Wire Wire Line
	5100 2900 5700 2900
Text GLabel 2600 2900 0    50   Input ~ 0
5V
Text GLabel 5700 2900 2    50   Output ~ 0
12V
Text GLabel 5700 3300 2    50   Output ~ 0
OUT
Text GLabel 2600 3300 0    50   Input ~ 0
GND
Wire Notes Line
	2900 2800 2900 3400
Wire Notes Line
	2900 3400 3900 3400
Wire Notes Line
	3900 3400 3900 2800
Wire Notes Line
	3900 2800 2900 2800
Wire Wire Line
	2600 2900 2900 2900
Connection ~ 5100 2900
Wire Wire Line
	3900 3300 4200 3300
Wire Wire Line
	4200 3300 4200 4200
$Comp
L power:GND #PWR02
U 1 1 6058E615
P 4200 4200
F 0 "#PWR02" H 4200 3950 50  0001 C CNN
F 1 "GND" H 4205 4027 50  0000 C CNN
F 2 "" H 4200 4200 50  0001 C CNN
F 3 "" H 4200 4200 50  0001 C CNN
	1    4200 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 3300 2900 3300
Wire Wire Line
	3900 2900 5100 2900
Text Notes 3100 3150 0    50   ~ 0
5V / 12V\nDC-DC converter
$Comp
L Device:Voltmeter_DC MES1
U 1 1 6058F1BE
P 6300 3100
F 0 "MES1" H 6453 3146 50  0000 L CNN
F 1 "GAUGE" H 6453 3055 50  0000 L CNN
F 2 "" V 6300 3200 50  0001 C CNN
F 3 "~" V 6300 3200 50  0001 C CNN
	1    6300 3100
	1    0    0    -1  
$EndComp
Wire Notes Line
	5950 3300 6300 3300
Wire Notes Line
	5950 2900 6300 2900
$EndSCHEMATC
