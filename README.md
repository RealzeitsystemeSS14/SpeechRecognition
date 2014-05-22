## Installation

* für sphinx müssen die device files __/dev/dsp*__ vorhanden sein
* autom. anlegen:

```
sudo su
echo "snd_pcm_oss" >> /etc/modules
exit
```

* sphinx hat unter raspbian probleme mit __/dev/dsp__ es sollte immer auf __/dev/dsp1__ zugegriffen werden