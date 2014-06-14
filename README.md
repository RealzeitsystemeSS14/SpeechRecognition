# Spracherkennung Raspberry Pi

## Installation

### Vorbereitung

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

* diese Installations Schritte müssen noch ausgeführt werden

```
sudo apt-get update 
sudo apt-get install git vim cmake screen
sudo rpi-update
```

---

### ALSA

* zur Überprüfung ob ALSA richtig eingerichtet ist, können folgende Befehle genutzt werden
* in beiden Fällen sollte die USB-Karte an erster Stelle stehen

```
cat /proc/asound/cards
cat /proc/asound/modules
```

* für sphinx müssen die device files __/dev/dsp*__ vorhanden sein
* autom. anlegen:

```
sudo su
echo "snd_pcm_oss" >> /etc/modules
exit
```

* in der Datei __/etc/modprobe.d/alsa-base.conf__ die Zeile mit  dem entsprechenden Eintrag suchen oder anlegen

```
>>>>> diese Zeile ändern
options snd-usb-audio index=-2
<<<<< in
options snd-usb-audio index=0
```

* Sphinx hat unter Raspbian Probleme mit __/dev/dsp__ es sollte immer auf __/dev/dsp1__ zugegriffen werden
* um das USB Gerät als default Audiogerät auszuwählen, muss in die Datei __/etc/asound.conf__ folgendes eingetragen werden

```
pcm.!default {
    type plug
    slave {
        pcm "hw:1,0"
    }
}
ctl.!default {
    type hw
    card 1
}
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

* folgende Pakete müssen zuerst installiert werden

```
sudo apt-get install alsa-utils bison libasound2-dev
```

* nun muss __sphinx base__ installiert werden

```
wget -O sphinxbase-0.8.tar.gz http://sourceforge.net/projects/cmusphinx/files/sphinxbase/0.8/sphinxbase-0.8.tar.gz/download
tar xzf sphinxbase-0.8.tar.gz
cd sphinxbase-0.8
./configure
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

---

### allegro

* allegro wird zur grafischen Darstellung der Simulation genutzt

```
sudo apt-get install liballegro4.2-dev
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

### Schlechte Spracherkennung

Die Befehle wurden schlecht und nur sehr stark verzögert erkannt. Wird die Funktion ```ad_open()``` verwendet, ein Audio Device zu öffnen, wird das Default Gerät mit einer Abtastrate von 8000Hz verwendet. 
Diese niedrige Abtastrate führt zu einer schlechten Qualität der Spracherkennung. Darum sollte die Funktion ```ad_open_sps(int sample_rate)``` verwendet werden, da hier die Abtastrate angegeben werden kann. Ein guter Wert
stell __48000Hz__ dar.

Die __Vergrößerung des AudioBuffers__, der zur Übertragung der Sprachdaten zwischen InputThread und InterpreterThread genutzt wird, auf __16k mal int16__ war notwendig, da der Buffer sonst zu kurz war, um die Befehle aufzunehmen.

Aufgrund des kleinen Puffers sind in vorherigen Tests auch einige Befehle verloren gegangen. Pocketsphinx kann mit dem größeren Puffer Sprachdaten auch mit größerer Wahrscheinlichkeit korrekt interpretieren.

Der Aufruf der Funktion ```ps_process_raw(p_thread->psDecoder, buffer->buffer, buffer->size, 0, 1);``` sollte als letztes Argument __true__ erhalten. Damit wird Pocketsphinx mitgeteilt, dass der übergebene Puffer __sämtliche Audiodaten__ für die Interpretation des Befehls beinhaltet und nichts weiter dazu kommt. Dadurch wird die Sicherheit und Effizienz der Interpretation erhöht.

## Zu Testen

```
sudo apt-get install alsa-utils
```

## Links

### ALSA

* https://sites.google.com/site/observing/Home/speech-recognition-with-the-raspberry-pi
* http://www.linuxcircle.com/2013/05/08/raspberry-pi-microphone-setup-with-usb-sound-card/

### pulseaudio

* http://askubuntu.com/questions/31206/how-can-my-audio-input-always-be-the-webcam-microphone

### pocketsphinx

* http://cmusphinx.sourceforge.net/wiki/pocketsphinxhandhelds

