# Spracherkennung Raspberry Pi

## Installation

### Hilfreiche Tools

```
sudo apt-get install screen
sudo apt-get install git vim
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

* http://www.linuxcircle.com/2013/05/08/raspberry-pi-microphone-setup-with-usb-sound-card/
* https://sites.google.com/site/observing/Home/speech-recognition-with-the-raspberry-pi

---

### pulseaudio

* Sphinx benutzt standardmäßig __pulseaudio__, anstatt ALSA, wenn dieses zum Kompilierzeitpunkt vorhanden ist

* __pulseaudio__ installieren

```
sudo apt-get install pulseaudio
```

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

## Probleme

### ALSA

Pocketsphinx konnte die Mikrofonverstärkung nicht einstellen / ändern, wodurch sich die Sprachaufnahmen wohl überschlagen haben. Dadurch konnten Worte nicht korrekt interpretiert werden.

__Beispiel:__

Es wird "Light" gesagt, ausgegeben wird aber "Light Light Light On On Light On". Dies deutet auf ein stark verrauschtes Signal (zu hohe Verstärkung)
oder ein kaum wahrnehmbares Signal (zu niedriege Verstärkung) hin.

Abhilfe hat hier der Einsatz von __pulseaudio__ anstatt __alsa__ gebracht.
