# Poor Man's Dialpad


Voltage at Key (in V, Vcc 3.3V)

 |||
-----|------|-----
0.00 | 0.93 | 1.45
0.30 | 1.09 | 1.54
0.55 | 1.22 | 1.63
0.76 | 1.35 | 1.71


### Schematic

```
                    Vcc --(100k)--+----- GPIO
                                  |
                                (10k)
 -- "1" -   -- "2" -   -- "3" -   |
 |       \--|-------\--|-------\--+----- ADC
 |          |          |          |
 |          |          |         (1k)
 |          |          |          |
 +- "4" -   +- "5" -   +- "6" -   |
 |       \--|-------\--|-------\--+
 |          |          |          |
 |          |          |         (1k)
 |          |          |          |
 +- "7" -   +- "8" -   +- "9" -   |
 |       \--|-------\--|-------\--+
 |          |          |          |
 |          |          |         (1k)
 |          |          |          |
 -- "*" -   +- "0" -   +- "3" -   |
 |       \--|-------\--|-------\--+
 |          |          |
 |          |          |
 |          |          |
 ---(3k9)---+--(3k9)---+-- GND
```
