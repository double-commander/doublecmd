#!/bin/sh

export REVISION_INC=$1/dcrevision.inc

rm -f $REVISION_INC
cp ../units/dcrevision.inc $REVISION_INC

export REVISION=$(git rev-parse --short HEAD)

if [ ! -z $REVISION ]; then

  echo "// Created by Svn2RevisionInc"    >  $REVISION_INC
  echo "const dcRevision = '$REVISION';"  >> $REVISION_INC

fi

echo "Subversion revision" $REVISION
