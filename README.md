# Refuel
*Sailfish OS App to search for filling station prices in Germany*

## Why?

What features are provided compared to [spritradar-fork](https://openrepos.net/content/poetaster/spritradar-fork):

* Builtin API key
* Map View
* Improved user experience


## Possible future features

* Save favorite filling stations
* Support for more countries
* Search for current geo position
* Show brand icons


## Build

Before build open `.env.sample` file and set your own API key and save as `.env`
file:

    cp .env.sample .env
    vim .env
    sfdk build
