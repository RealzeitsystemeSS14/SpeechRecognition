# Spracherkennung Raspberry Pi

## Installation

### Hilfreiche Tools

```
sudo apt-get install screen
sudo apt-get install git vim cmake
```

---

### ALSA

* für sphinx müssen die device files __/dev/dsp*__ vorhanden sein
* autom. anlegen:

```
sudo su
echo "snd_pcm_oss" >> /etc/modules
exit
```

* in der Datei __/etc/modprobe.d/alsa-base.conf__ die Zeile mit  dem entsprechenden Eintrag suchen oder anlegen

```
options snd-usb-audio index=-2
```

* ändern in 

```
options snd-usb-audio index=1
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

__Hilfreiche Links__


---

### pocketsphinx

* folgende Pakete müssen zuerst installiert werden

```
sudo apt-get install bison libasound2-dev
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

### allegro

* allegro wird zur grafischen Darstellung der Simulation genutzt

```
sudo apt-get install liballegro4.2-dev
```

## Probleme

### ALSA

Pocketsphinx konnte die Mikrofonverstärkung nicht einstellen / ändern, wodurch sich die Sprachaufnahmen wohl überschlagen haben. Dadurch konnten Worte nicht korrekt interpretiert werden.

__Beispiel:__

Es wird "Light" gesagt, ausgegeben wird aber "Light Light Light On On Light On". Dies deutet auf ein stark verrauschtes Signal (zu hohe Verstärkung)
oder ein kaum wahrnehmbares Signal (zu niedrige Verstärkung) hin.

Abhilfe hat hier die Installation von __libasound2-dev__ gebracht. Auch wenn diese Bibliothek nicht installiert ist, kompiliert pocketsphinx anstandslos. Zur Laufzeit kommt jedoch die Warnung, dass die Mikrofonverstärkung nicht eingestellt werden konnte. Nach Installation der Bibliothek, müssen pocketsphinx und sphinxbase __komplett gelöscht__ und die Schritte zu deren Installtion wiederholt werden.

### Schlechte Spracherkennung

Die Befehle wurden schlecht und nur sehr stark verzögert erkannt. Wird die Funktion ```ad_open()``` verwendet, ein Audio Device zu öffnen, wird das Default Gerät (__/dev/dsp__) mit einer Abtastrate von 8000Hz verwendet. 
Diese niedrige Abtastrate führt zu einer schlechten Qualität der Spracherkennung. Darum sollte die Funktion ```ad_open_sps(int sample_rate)``` verwendet werden, da hier die Abtastrate angegeben werden kann. Ein guter Wert
stell 44000Hz dar.

## Zu Testen

```
sudo apt-get install alsa-utils
```

## Links

### ALSA einrichten

* https://sites.google.com/site/observing/Home/speech-recognition-with-the-raspberry-pi
* http://www.linuxcircle.com/2013/05/08/raspberry-pi-microphone-setup-with-usb-sound-card/

