#!/usr/bin/env bash

while true; do ls /dev/cu.* | grep -v Bluetooth; echo .; sleep 1; done
