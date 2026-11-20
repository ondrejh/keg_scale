# web20 – Kegator web UI

Plain HTML/CSS/JS, no build step. Served by fw20 from the `storage` (LittleFS) partition.

## Local testing

```bash
python3 dev_server.py                       # mock backend, http://localhost:8000/
python3 dev_server.py --keg                 # mock, start with a tapped keg
python3 dev_server.py --device 192.168.42.1 # local pages, live API from a real scale
```

Mock state is seeded from `data.json`, `conf.json`, `keglist.txt`, kept in memory
only and resets on restart (files are never modified).
