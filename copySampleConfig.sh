#!/bin/bash
test -f CalifaConfig.h && exit
echo "Creating default configuration file"
echo "------------------------"
echo "  Backup of original configuration file is stored to CalifaConfig.h.back"
echo "------------------------"
touch CalifaConfig.h
cp CalifaConfig.h CalifaConfig.h.back
cp CalifaConfig.h.sample CalifaConfig.h
