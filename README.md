# Spracherkennung Raspberry Pi

## Installation

### raspbian

* zur Installation wird ein minimaler raspbian netinstaller verwendet

```
wget -O raspbian-ua-netinst-v1.0.2.img.xz https://github.com/debian-pi/raspbian-ua-netinst/releases/download/v1.0.2/raspbian-ua-netinst-v1.0.2.img.xz
sudo su
xzcat raspbian-ua-netinst-v1.0.2.img.xz > /dev/meinSDDevice
exit
```

* nun muss die SD-Karte in das Raspberry eingelegt und das Raspberry gestartet werden
__Anmerkung:__ Das Raspberry muss an das Internet angeschlossen sein. Außerdem ist ein login über seriell per default ausgeschaltet.

* seriellen Zugriff aktivieren in datei __/etc/inittab__ folgendes ändern

```
>>>>> diese zeile suchen
#T0:23:respawn:/sbin/getty -L ttyS1 9600 vt100
<<<<< so ändern
T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100
```



### Vorbereitung

* folgende Programme müssen installiert werden

```
sudo apt-get update 
sudo apt-get install build-essential pkg-config libasound2-dev alsa-utils xserver-xorg-core xinit git vim cmake bison liballegro4.2-dev
sudo rpi-update
```

* um Raspberry in die GUI zu starten, bei folgendem Kommando __startlxde__ auswählen

```
update-alternatives --config x-session-manager
```

* die Tastatur auf Deutsch umstellen

```
dpkg-reconfigure console-data
dpkg-reconfigure keyboard-configuration
service keyboard-setup restart
```

* für das Robotlabor folgende Anpassung in der Date __/etc/networ/interfaces__ vornehmen

```
>>>>> diese Zeile ändern
iface eth0 inet dhcp
<<<<< in
iface eth0 inet static
        address 141.37.31.104
        netmask 255.255.252.0
        gateway 141.37.28.254
        dns-nameservers 141.37.120.101
```

---

### ALSA

* zur Überprüfung ob ALSA richtig eingerichtet ist, können folgende Befehle genutzt werden
* in beiden Fällen sollte die USB-Karte an erster Stelle stehen

```
cat /proc/asound/cards
cat /proc/asound/modules
```

* falls dies nicht der Fall ist
* in der Datei __/etc/modprobe.d/alsa-base.conf__ die Zeile mit  dem entsprechenden Eintrag suchen oder anlegen

```
>>>>> diese Zeile ändern
options snd-usb-audio index=-2
<<<<< in
options snd-usb-audio index=0
```

---

### pulseaudio

* am Desktop Rechner wird pulseaudio verwendet, daher muss das USB-Mikrofon als Default device ausgewählt werden

```
mkdir -p ~/.pulse
cp /etc/pulse/default.pa ~/.pulse
vim ~/.pulse/default.pa
```

* nun müssen folgende Einträge gemacht werden

```
>>>>> die Zeile ändern
set-default-source input
<<<<< in
set-default-source <device-name>
```

* den device name kann man mit dem befehl ```pacmd``` herausbekommen, dieser steht dort hinter dem tag __name:__
* der name wird ohne die spitzen Klammern ( <name> ) eingetragen
* nun muss noch eingestellt werden, dass das Mikrofon beim Start nicht stumm geschaltet wird
* auch in der Datei __~/.pulse/default.pa__ folgende Zeile hinzufügen


```
set-source-mute 1 0
```

* 0 bedeutet hierbei __false__, die 1 bedeutet die Anzahl der Geräte, angefangen beim Default Device

---

### pocketsphinx

* zuerst muss __sphinx base__ installiert werden (--enable-fixed sorgt, dafür, dass Fetskommazahlen benutzt werden; verbessert die performance)

```
wget -O sphinxbase-0.8.tar.gz http://sourceforge.net/projects/cmusphinx/files/sphinxbase/0.8/sphinxbase-0.8.tar.gz/download
tar xzf sphinxbase-0.8.tar.gz
cd sphinxbase-0.8
./configure --enable-fixed
make
sudo make install
```

* nun kann __pocketsphinx__ installiert werden

```
wget -O pocketsphinx-0.8.tar.gz http://sourceforge.net/projects/cmusphinx/files/pocketsphinx/0.8/pocketsphinx-0.8.tar.gz/download
tar xzf pocketsphinx-0.8.tar.gz
cd pocketsphinx-0.8
./configure
make
sudo make install
```

## Sprachmodell erzeugen

Die Erzeugung eines Sprachmodells ist mit [lmtool](http://www.speech.cs.cmu.edu/tools/lmtool-new.html) möglich.
Das Tool benutzt dabei ausschließlich die amerikanisch-englische Sprache. Außerdem sollte die corpus Datei im ASCII Format vorliegen. Unicode Formate bereiten Schwierigkeiten.

## Probleme

### ALSA

Pocketsphinx konnte die Mikrofonverstärkung nicht einstellen / ändern, wodurch sich die Sprachaufnahmen wohl überschlagen haben. Dadurch konnten Worte nicht korrekt interpretiert werden.

__Beispiel:__

Es wird __"Light"__ gesagt, ausgegeben wird aber __"Light Light Light On On Light On"__. Dies deutet auf ein __stark verrauschtes Signal__ (zu hohe Verstärkung)
oder ein kaum wahrnehmbares Signal (zu niedrige Verstärkung) hin.

Abhilfe hat hier die Installation von __libasound2-dev__ gebracht. Auch wenn diese Bibliothek nicht installiert ist, kompiliert pocketsphinx anstandslos. Zur Laufzeit kommt jedoch die Warnung, dass die Mikrofonverstärkung nicht eingestellt werden konnte. Nach Installation der Bibliothek, müssen pocketsphinx und sphinxbase __komplett gelöscht__ und die Schritte zu deren Installtion wiederholt werden.

---

### pocketsphinx

#### Schlechte Spracherkennung

Die __Vergrößerung des AudioBuffers__, der zur Übertragung der Sprachdaten zwischen InputThread und InterpreterThread genutzt wird, auf __16k mal int16__ war notwendig, da der Buffer sonst zu kurz war, um die Befehle aufzunehmen.
Aufgrund des kleinen Puffers sind in vorherigen Tests auch einige Befehle verloren gegangen.

Der Aufruf der Funktion ```ps_process_raw(p_thread->psDecoder, buffer->buffer, buffer->size, 0, 1);``` sollte als letztes Argument __true__ erhalten. Damit wird Pocketsphinx mitgeteilt, dass der übergebene Puffer __sämtliche Audiodaten__ für die Interpretation des Befehls beinhaltet und nichts weiter dazu kommt. Dadurch wird die Sicherheit und Effizienz der Interpretation erhöht.

Der Pocketsphinxdecoder muss vor den Audiodevices initialisiert werden, damit die Konfigurationen, die nur (!) an den Decoder übergeben werden, auch für die Devices übernommen werden.
Es handelt sich laut pocketsphinx Doku um eine globale Konfigurationseinheit.

Die Sprachaufnahme muss etwas länger gehalten werden, als eigentlich gesprochen wird. Durch das loslassen der Taste wird für pocketsphinx das Aufnehmen beendet. Aufgrund von Verzögerungen durch Treiber und das Betriebsystem
sind aber noch nicht alle Audiodaten, die im Mikrofon eingegangen sind, auch in pocketsphinx angekommen. Daher muss pocketsphinx etwas länger aufnehmen als gesprochen wird, was sehr unintuitiv ist.
Wird dieser Aspekt nicht beachtet, werden Befehle höchstwahrscheinlich gar nicht oder falsch interpretiert.

####  Einstellungen

Pocketsphinx registriert Input recht langsam (3,5 bis 6 Sekunden), daher müssen die Einstellungen für pocketsphinx geändert werden.
Mit dem Tool ```pocketsphinx_batch``` lassen sich alle Argumente für pocketsphinx und deren Default-Werte ausgeben. Für gute Einstellmöglichkeiten, um pocketsphinx schneller zu machen, kann [hier](http://cmusphinx.sourceforge.net/wiki/pocketsphinxhandhelds) nachgeschaut werden.
Bei richtiger Einstellung sind Aufnahme und Interpretation in unter 1 Sekunde machbar.

Auf Desktoprechnern ist meist pulseaudio installiert. Dieses wird von pocketsphinx vorzugsweise verwendet. Jedoch erfolgt die Sprachaufnahme mit pulseaudio sehr viel langsamer als mit ALSA. Subrealzeit ist nur
mit der Verwendung von ALSA möglich. Dazu muss pulseaudio __komplett__ deinstalliert werden.

---

### allegro

Allegro legt ein eigenes Signalhandling während ```allegro_init()``` an. Daher müssen die eigenen Signale nach dem Aufruf dieser Funktionen gesetzt werden, sonst werden sie von allegro überschrieben.

## Links

### ALSA

* https://sites.google.com/site/observing/Home/speech-recognition-with-the-raspberry-pi
* http://www.linuxcircle.com/2013/05/08/raspberry-pi-microphone-setup-with-usb-sound-card/

### pulseaudio

* http://askubuntu.com/questions/31206/how-can-my-audio-input-always-be-the-webcam-microphone

### pocketsphinx

* http://cmusphinx.sourceforge.net/wiki/pocketsphinxhandhelds

### xorg

* http://www.raspberrypi.org/forums/viewtopic.php?p=344408
