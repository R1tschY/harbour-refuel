#!/bin/bash -e

SVG=$(dirname -- "${BASH_SOURCE[0]}")/harbour-refuel.svg
DIR=$(dirname "$SVG")
NAME=$(basename "$SVG")
NAME=${NAME%.*}

for SIZE in 86 108 128 172
do
    mkdir -p "${SIZE}x${SIZE}"
    inkscape -w $SIZE -h $SIZE "$SVG" --export-overwrite --export-type=png -o "$DIR/${SIZE}x${SIZE}/$NAME.png"
done
