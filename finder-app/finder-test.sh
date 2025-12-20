#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Modified for assignment 4 Buildroot/QEMU requirements
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data

# ÄNDERUNG: Pfad zur username.txt auf /etc/finder-app/conf/ angepasst
username=$(cat /etc/finder-app/conf/username.txt)

if [ $# -lt 3 ]
then
    echo "Using default value ${WRITESTR} for string to write"
    if [ $# -lt 1 ]
    then
        echo "Using default value ${NUMFILES} for number of files to write"
    else
        NUMFILES=$1
    fi  
else
    NUMFILES=$1
    WRITESTR=$2
    WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# ÄNDERUNG: Pfad zur assignment.txt auf /etc/finder-app/conf/ angepasst
assignment=`cat /etc/finder-app/conf/assignment.txt`

if [ $assignment != 'assignment1' ]
then
    mkdir -p "$WRITEDIR"

    if [ -d "$WRITEDIR" ]
    then
        echo "$WRITEDIR created"
    else
        exit 1
    fi
fi

# Die Sektion für 'make' wurde entfernt, da Buildroot das Kompilieren übernimmt.

for i in $( seq 1 $NUMFILES)
do
    # ÄNDERUNG: './' entfernt, da 'writer' im PATH (/usr/bin) liegt
    writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"
done

# ÄNDERUNG: './' entfernt, da 'finder.sh' im PATH (/usr/bin) liegt
OUTPUTSTRING=$(finder.sh "$WRITEDIR" "$WRITESTR")

# ÄNDERUNG: Ausgabe in die spezifizierte Ergebnisdatei schreiben
echo "${OUTPUTSTRING}" > /tmp/assignment4-result.txt

# remove temporary directories
rm -rf /tmp/aeld-data

set +e
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
    echo "success"
    exit 0
else
    echo "failed: expected ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
    exit 1
fi