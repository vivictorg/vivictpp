#!/bin/bash
description=$(git describe)
if [[ $description =~ ^v[0-9]+[.][0-9]+[.][0-9]+(-.*)?$ ]]
then
    echo "${description%%-*}" | sed -e 's/v//g'
else
    exit 1
fi


