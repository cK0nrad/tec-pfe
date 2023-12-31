# Simple POC SAEIV/PFE

SAEIV/PFE = TEC's bus tracking system
Equivalent to an ITS or OSS.
(inteligent transport system / operational support system)

Assuming a Linux distros
Compatible with RaspPI 4

## Intellectual Property Notice

This repository has been published in the absence of any discovered patents or copyright records for terms including "letec pfe," "otw pfe," "srwt pfe," "letec sae," among others in various combinations. Should there exist a patent or any other form of intellectual property that this repository infringes upon, please reach out to me. Upon verification, I will promptly remove the repository.


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

## Skills

This project is meant to demonstrate skills abilities (and also because I do a lot of TEC clones/data analysis) such as :

- C++
- FLTK
- Linux
- Sqlite
- Git
- Multi-threading 
- Sockets (uds here)
- IPC (due to uds)

## Time spent

It was just a POC, so I did it in few nights (~30h with 6h-8h of fltk doc reading) to experiment with FLTK and clone TEC OSS.

## Screenshot

## Video

## Credits

- TEC employees on TikTok for the UI
- cKonrad (aka. me)



 //When emulating, the bus is always on time or always constantly
            // late or early
            // Thus whatever the average speed, it'll never change the actual delay