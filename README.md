# N18-3106
Update aplikátoru pro konfiguraci z FTP serveru pro IFE Brno
První úprava, vložená do dvou aplikátorů ve vestavku a ve výrobní hale





Jedná se o klasický projekt lepící stanice, který je doplněn o dodatečnou funkcionalitu, kterými jsou:
* komunikace s FTP serverem a načítání vstupních parametrů
* komunikace s FTP serverem a mazání starých dat
* komunikace s FTP serverem a ukládání výstupních dat
* kontrola zda je lepící pistole v domovské pozici a automatické odpouštění lepidla
* propojení vizualizace s externím počítačem prostřednictvím WINCC runtime




Pomocí protokolu FTP je do plc načten vstupní soubor, informující o požadovaném množství lepidla, které je nutné vytlačit. Software lepící stanice je navržen jako řízený událostí (obsluha tlačítek, senzorů a vnitřních stavů stanice), ale požadavkem je, aby stanice byla řízena pracovním cyklem (opakující se sekvence načítání vstupů, vykonání lepení a uložení výstupu).
Poměr lepidla a aktivátoru je dán lepícím programem, ale množství vytlačeného lepidla je dáno vstupním souborem z FTP serveru.
Lepící stanice je umístěna v VLAN počítačové sítě u zákazníka a FTP server se nachází v jiné podsítí, než lepící stanice. Ke komunikaci je zapotřebí využít router, který bude přeposílat komunikaci mezi FTP serverem a PLC lepící stanice. 
Důležité je, že na daném routeru je nutné nastavit přeposílání TCP portů z FTP serveru na PLC. FTP protokol funguje tak, že vytváří paralelně dvě TCP spojení - komunikační a datové. Komunikační spojení pracuje na portu 21 a jeho přeposílání bývá automatické. TCP port datového spojení je ale určen komunikujícím klientem a je předem neznámé. Protože datové spojení se při ukládání dat z FTP serveru vytváří na straně serveru a je adresováno výchozí bránou na podsíť lepící stanice, nebylo by bez předem známého TCP portu komunikace a přeposílání portů na straně routeru možné obostranně komunikavat s ftp serverem. Na PLC lepící stanici je fixně nastaven TCP port komunikačního kanálu na 1039, veškerá komunikace s tímto portem musí být na routeru přeposílána na PLC lepící stanice.

**********************************************************************************************************
*													 *
*	FTP				    ROUTER						PLC	 *
* 10.1.2.5:1039 -------------> 10.1.2.1:1039 -> 100.30.2.1:1039 -----------------------> 100.30.2.2:1039 *
*													 *
**********************************************************************************************************

Pro účely obsluhy ftp serveru je vytvořena samostantní funkce FTP, která je volána v bloku MAIN a k ní samostatný datový blok s proměnnými. Uvnitř funkce FTP je definován samotný pracovní cycklus spojený s načítáním dat z FTP serveru a dávkování daného množství lepidla.
Uvnitř datového bloku pro obsluhu FTP jsou definované dvě pole typu char s 1024 prvky indexovanými od nuly. Jedno pole slouží k načítání dat z FTP serveru a druhé slouží k odesílaní předpřipravené zprávy zpět na server


** Automatické odpouštění lepidla **
V programu lepící stanice se nachází bloku "FB602_Timer2", který je zodpovědný za hlídání časů expirace lepící směsi v hadicích a pistoli (hlídá před zatuhnutím) a životnost mixéru míchající lepící směs (jedná se čas po který je možné jeden mixér používat).
Tyto hodnoty je možné nastavit v lepícím programu v HMI. Do programu je nutné doplnit, aby před expirací lepící směsi se na HMI zobrazila hláška, že je požadováno odpuštění lepidla a je nutné, aby lepící pistole byla v domovské pozici. Domovská pozice je identifikována aktivací čidla přítomnosti. Pokud jsou splněny podmínky že je lepící pistole v domovské pozici a zároveň se blíží expirace lepidla, je automticky odpuštěno dané množství lepidla. Blok "FB602_Timer2" dává na výstup dvě informativní hodnoty pro každé časové měření (expiraci směsi a expiraci mixéru). Jedna je binární udávající zda došlo k expiraci daného parametru a druhá je kolik zbývá do expirace daného parametru v milisekundách.
Pokud není lepící pistole v domovské pozici a zároveň je třeba odpustit lepidlo, je nutné vytvořit časovou prodlevu pro start automatického odpuštění lepidla poté co je pistole vrácena do domovské pozice. Do procesu automatického odpuštění je nutné přidat podmínku, že pokud jednou proces automatického odpuštění začne musí se dokončit, pro případ, že by se lepící pistole vložila do domovské pozice těsně před expirací lepící směsi a expirace by vypršela v průběhu odpouštění. Nejjednodušší způsob jak toto ošetřit a zároveň nedělat velké změny v softwaru je přidat podmínku, že proces automatického odpouštění nelze spustit pokud již expirace lepidla je kratší než doba automatického odpouštění (proces by se nestihl vykonat).




** Poznámky **
* Jaký je rozdíl mezi životností směsi a životnost mixéru? V programu jsou tyto doby počítany samostatně - životnost namíchané směsy lepidla před zatuhnutím, životnost používání jednoho mixéru než je třeba jej vyměnit kvůli opotřebení
* Co se má stát, když vyprší životnost směsy/mixéru a pistole nebude v domovské pozici? Korektně se zahlásit alarm jako v případě běžného režimu.
* co se stane když někdo přepne stroj do manuálního režimu v kroku 6 pracovního cyklu, kdy se vytlačuje požadované množství lepidla
* mělo by se kontrololovat zda lepidlo bylo vytlačeno v určtém intervalu kvůli tuhnutí?


* zjistit software na existujícím stroji v IFE pro účely update komikace s ftp serverem


** TODO **


** DONE **
* identifikovat kód pro odpouštění lepidla
* parsovat vstupní parametry z ftp serveru a reagovat na chybnýformát přijatých dat
* vytvořit vizualizaci na WINCC runtime a propojit s PLC
* parametrizovat lepící stanici hodnotamy z FTP serveru
* přidat upozornění na odpouštění
* přidat automatické vypnutí stanice při neaktivitě
* přidat automatické odpouštění lepidla pokud je lepící pistole v domovské pozici
* upravit kód, pro vytlačování lepidla z lepící pistole, aby nebylo možné vytlačovat více lepidla, než je zadáno z ftp serveru
* přepínání jmen souborů pro ftp
* sestavit řetězec do výstupního souboru pro odeslání přes ftp
* vytvořit logickou funkci, která bude určovat zda je monžné vytlačovat lepidlo nebo již byl požadovaný objem vytlačen
* vytvořit knihovnu pro komunikaci s ftp serverem
* implementovat pracovní cyklus práce s ftp serverem
* identifikovat kód pro výpočet vytlačeného objemu lepidla




