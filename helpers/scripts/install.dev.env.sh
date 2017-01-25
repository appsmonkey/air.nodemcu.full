#!/usr/bin/env bash

/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

#Desktop Apps
brew cask install Caskroom/cask/silicon-labs-vcp-driver
brew cask install Caskroom/cask/github-desktop
brew cask install Caskroom/cask/atom

#Code beautifier
brew install uncrustify

#arp-scan
brew install arp-scan

#Atom packages
apm install platformio-ide
apm install atom-beautify
apm install symbols-tree-view

mkdir ~/src/
cd ~/src/
git clone https://github.com/appsmonkey/air.nodemcu.full.git

#Set atom configs
cp ~/src/air.nodemcu.full/helpers/config.cson ~/.atom/

#Set ignore on config file
cd ./air.nodemcu.full/
git update-index --assume-unchanged lib/Config/Config.h

#add useful aliases to bash
echo ". ~/src/air.nodemcu.full/helpers/.bashrc" >> ~/.bash_profile

#curl -L --silent "https://github.com/appsmonkey/air.nodemcu.full/archive/master.zip" --output air.nodemcu.full.zip
#unzip air.nodemcu.full.zip
#rm -f air.nodemcu.full.zip
