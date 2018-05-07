#!/bin/source

ACTUL_BUILD=$1
if [ -z "$1" ];
then
	ACTUL_BUILD=`pwd`/build
fi


export CC="bash $ACTUL_BUILD/test_actul"
export CXX="bash $ACTUL_BUILD/test_actul++"

echo "CC set to'$CC'"
echo "CXX set to'$CXX'"
