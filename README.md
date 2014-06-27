# Spracherkennung Raspberry Pi

## About
Hierbei handelt es sich um ein Realzeitsystem für das Raspberry Pi mit Raspbian. Ziel ist es Spracherkennung in (Sub-) Realzeit zu realisieren. Hierzu wird die Bibliothek Pocketsphinx verwendet. Mehr Informationen können im [Wiki](https://github.com/RealzeitsystemeSS14/SpeechRecognition/wiki) nachgelesen werden.

## Compile
Um das Projekt zu kompilieren müssen __cmake__ und die restlichen Abhängigkeiten installiert sein. Dazu mehr im Wiki. Nun einfach im Projekt verzeichnis folgende Befehle ausführen.

```
mkdir build
cd build
cmake ..
make
```

Alternativ kann auch das Skript __deploy__ ausgeführt werden.
