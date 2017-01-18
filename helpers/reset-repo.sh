cd ..
rm -rf .git
cp lib/Config/Config.h  /var/tmp/Config.h
cp lib/Config/Config.h.simple  lib/Config/Config.h
git init
git add .
git commit -m "Repo Reset"
git remote add origin https://github.com/appsmonkey/air.nodemcu.full.git
git push -u --force origin master
git update-index --assume-unchanged lib/Config/Config.h
cp /var/tmp/Config.h lib/Config/Config.h