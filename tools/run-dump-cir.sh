#!/bin/sh

rm  -rf ./sphinx/source/dump/
mkdir ./sphinx/source/dump/

./j dump-cir \
  --cir-dump-path-pattern="sphinx/source/dump/cir.{}.{}." \
  --air-dump-path-pattern="sphinx/source/dump/air.{}." \
  "$*"

cd sphinx
make html
