#!/bin/bash
description=$(git describe)
if [[ $description =~ ^v[0-9]+[.][0-9]+[.][0-9]+$ ]]
then
    echo "$description" | sed -e 's/v//g'
elif [[ $description =~ ^v[0-9]+[.][0-9]+[.][0-9]+-.*$ ]]
then
     echo "$description" | sed -re 's/v([0-9]+[.][0-9]+[.][0-9]+)(-[a-z0-9]+)?(-([0-9]+)-[0-9a-z]+)/\1.\4/g'
else
    exit 1
fi


