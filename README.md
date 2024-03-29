# Simple POC SAEIV/PFE

SAEIV/PFE = TEC's bus tracking system
Equivalent to an ITS or OSS.
(inteligent transport system / operational support system)

Assuming a Linux distros
Compatible with RaspPI 4

## Intellectual Property Notice

This repository has been published in the absence of any discovered patents or copyright records for terms including "letec pfe," "otw pfe," "srwt pfe," "letec sae," among others in various combinations. Should there exist a patent or any other form of intellectual property that this repository infringes upon, please reach out to me. Upon verification, I will promptly remove the repository.

## Video

https://github.com/cK0nrad/tec-pfe/assets/13602291/8c4d4a7b-a1e0-47fb-af42-b76a7897599e

## Hierarchy

This is most likely spaghetti code, but it works. It wasn't mean to be really maintainable but most of the codebase can be splitted into serval submodule without difficulties.

## Requirements

- FLTK
- Sqlite3 (builded by default but you can change it in the Makefile)

> $ sudo apt-get install libfltk1.3-dev sqlite3

## Build

> $ make all
> 
> $ ./its
> 
> $ ./gps_emiter

## DB

The database is not a perfect GTFS replica, but it do work the same way.

## Screenshots

![afficheur](https://raw.githubusercontent.com/cK0nrad/tec-pfe/main/screenshot/afficheur.jpg)
![billetique](https://raw.githubusercontent.com/cK0nrad/tec-pfe/main/screenshot/billetique.jpg)
![ordibus](https://raw.githubusercontent.com/cK0nrad/tec-pfe/main/screenshot/ordibus.jpg)
![AutoPilot](https://raw.githubusercontent.com/cK0nrad/tec-pfe/main/screenshot/Capture.jpg)

## Credits

- TEC employees on TikTok for the UI
