

LED indikacija:
PowerLED atlieka PRV status ir Modbus Link indikacijos. Kai pultas aktyvus ( 20 sekundziu po paskutinio palietimo ) PowerLED rodo PRV statusa. Kai pultas neaktyvus
(uzblokuotas), PowerLED indikuoja Modbus rysio aktyvuma.

TempLEDS atlieka TSET, TJ ir Alarm indikacijjos. Kai pultas aktyvus ( 20 sekundziu po paskutinio palietimo ) TempLEDS rodo TSET reiksme (rodoma stulpeliu). 
Kai pultas neaktyvus, TempLEDS rodo TJ reiksme (rodoma tasku). Kai PRV plokste yra avariniame rezime TempLEDS rodo Alarmus. Kiekvienam TempLEDui skirtas savo alarmas:

PRV:
LED1 - ANTIFROST alarmas
LED2 - FIRE alarmas
LED3 - OVERHEAT alarmas
LED4 - FAN alarmas
LED5 - TJ alarmas
LED6 - TE alarmas
LED7 - TA alarmas
LED8 - RH alarmas
LED9 - TWAT alarmas
LED10 - TOUT alarmas
LED11 - ROTOR alarmas
LED12 - 
LED13 - 
LED14 - 
LED15 - Old RC Compatibility aktivuota
LED16 - Scheduler/OFF Mode aktyvus

VENTIK:
LED1 - LOWVOLTAGE alarmas
LED2 - FIRE alarmas
LED3 - CRITICAL_RETURN_TEMP alarmas
LED4 - FAN alarmas
LED5 - TJ alarmas
LED6 - TSUP_TOOLOW alarmas
LED7 - TSUP_TOOHIGH alarmas
LED8 - 
LED9 - TWAT alarmas
LED10 - TOUT alarmas
LED11 - 
LED12 - 
LED13 - 
LED14 - 
LED15 - 
LED16 -

SpeedLEDS atlieka greicio indikacija. Normaliame rezime indikatorius nuolat sviecia. Jai indikatorius mirkcioja, PRV ploksteje aktyvus Decrease Speed rezimas.
BoostLED atlieka Boost rezimo indikacija. Mirkcioja, kai Boost rezimas aktyvuotas is pulto. Nuolat sviecia, kai Boost rezimas aktivuotas PRV ploksteje. 


Mygtuku funkcijos
OnOff:
trumpas palietimas: nieko nedaro, tik aktyvuoja indikacija
dvigubas palietimas: ijungia/isjungia valdymo blokavima
ilgas palietimas: kai PRV ploksteje aktyvus alarmai perkrauna (Reset) PRV plokste. Jai PRV plokste yra darbiniame rezime, stabdo darba arba paleidzia, jai jis
buvo pries tai sustabdytas. Plokste pradeda darba su nustatymais, kurie buvo aktyvus jos stabdymo momentu. Kombinacija mygtuku OnOff ir Boost darbiniame rezime
perkrauna (Reset) PRV plokste.

TUp, TDown:
trumpi palietimai didina/mazina nustatyta temperatura vienu zingsniu. Pailgintas palietimas didina/mazina temperatura automatiskai, kol paliestas sensorius.

Speed1, Speed2, Speed3:
trumpi palietimai pasirenka greiti. Ilgas palietimas aktyvaus mygtuko sustabdo PRV plokste.


Veikimo detales:
Boost aktyvumo metu greiciu mygtukai blokuojami - greicio nepakeisti.
Boost rezimas aktyvus tik tam tikra laika. Jai PRV ploksteje nustatytas Boost taimeris, Boost rezimas veikia pagal PRV plokstes taimeri. Jai PRV plokstes taimeris
nenustatytas Boost rezimas veikia pagal vidini 5 minuciu taimeri.


Duomenys per Bluetooth:
Modbus nustatymai (R/W)
TSet, Speed, Boost reiksmes (R/W)
PRV plokstes ID (R)
PRV plokstes nustatymai (R/W)
PRV plokstes Modbus registrai (BLE2UART mode, R/W)
Kiti temperaturos davikliai (R)
Alarmai (R)




