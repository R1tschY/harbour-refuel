# Refuel
*Sailfish OS App to search for filling station prices in Germany*

<p align="center">
   <a href="https://github.com/R1tschY/harbour-refuel/actions?query=workflow%3A%22SailfishConnect+build%22">
      <img src="https://img.shields.io/github/workflow/status/R1tschY/harbour-refuel/SailfishConnect%20build.svg?style=flat&logo=github" />
   </a>
   <a href="https://openrepos.net/content/r1tschy/refuel">
      <img src="https://img.shields.io/badge/dynamic/json.svg?color=yellow&label=OpenRepos&query=%24.downloads&url=https%3A%2F%2Fopenrepos.net%2Fapi%2Fv1%2Fapps%2F12210&suffix=+downloads&style=flat&cacheSeconds=3600" />
   </a>
</p>

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
