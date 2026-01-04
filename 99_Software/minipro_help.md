
Minipro miniguide

For EEPROM like AT28C*-15PU reference to folloing devices:

-  AT28C64-15PU : CAT28C64A (EEPROM 8k x 8)
- AT28C256-15PU : CAT28C256A (EEPROM 32k x 8)

Per le memorie che hai comprato di tipo AT28C64-15PU e AT28C256-15PU bisogna
fare riferimento al chip CAT28C64A nel primo caso e xxx nel secondo.

Per estrarre il contenuto di un chip:
	minipro -d CAT28C64A -r rom.bin

  -d specifica il modello della memoria target
  -r l'utilizzo in lettura
  rom.bin e' il nome del file binario in cui fare l'estrazione

Per poterlo leggere si puo' usare il programma xxd nell'alias xxdr:
	xxdr rom.bin
Altrimenti nella forma non abbreviata il comando e':
	xxd -g1 -c8 -R never rom.bin
Per creare l'alias bisogna inserire alias 'xxdr' = 'xxd -g1 -c8 -R never'
nel file ~/.bash_aliases e fare il source del file.

Se si vuole salvare il contenuto dell'hexdump in un file:
	xxdr rom.bin rom.hex

Il file rom.hex puo' essere modificato e il binario rigenerato:
	xxdr -r rom.hex rom2.bin

Se si vuole poi scrivere il contenuto del binario nella memoria:
	minipr -d CAT28C64A -w rom2.bin	
