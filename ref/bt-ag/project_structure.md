bt_ag_project/
│
├─ include/
│   ├─ headset.h          # headset struct, connection state
│   ├─ audio.h            # ALSA audio interface
│   ├─ rfcomm_hfp.h       # RFCOMM/HFP definitions and AT commands
│   ├─ sms.h              # SMS handling
|   └─ sco.h              # SCO handleing
│
├─ src/
│   ├─ main.c             # main entry, initializes threads, periodic tasks
│   ├─ headset.c          # multi-pairing, scan, connection management
│   ├─ audio.c            # ALSA capture/playback
│   ├─ rfcomm_hfp.c       # RFCOMM thread, HFP command parsing
│   ├─ sms.c              # SMS send/receive, reconnect SMS
│   └─ sco.c              # SMS send/receive, reconnect SMS
│
├─ Makefile               # build instructions
└─ README.md              # project description and usage