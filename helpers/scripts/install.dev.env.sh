#!/usr/bin/env bash

mkdir ~/src/
cd ~/src/
git clone https://github.com/appsmonkey/air.nodemcu.full.git

#curl -L --silent "https://github.com/appsmonkey/air.nodemcu.full/archive/master.zip" --output air.nodemcu.full.zip
#unzip air.nodemcu.full.zip
#rm -f air.nodemcu.full.zip

/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew cask install Caskroom/cask/atom
brew install uncrustify

apm install platformio-ide
apm install atom-beautify
apm install symbols-tree-view

cp ~/src/air.nodemcu.full/helpers/config.cson ~/.atom/

cd ./air.nodemcu.full/
git update-index --assume-unchanged lib/Config/Config.h
