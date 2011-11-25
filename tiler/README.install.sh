#!/bin/sh

# *****************************************************************************
#  THIS DOCUMENT DESCRIBES THE BUILD STEPS NEEDED TO BUILD THE TWO COMPONENTS
#  OF THIS GIT TREE: MEMMGR AND D2C
#
#  The document is formatted as a shell script and is organized into 4
#  sections.  The first section sets up the environment variables needed
#  for the build sections.  You will need to set up the same variables if
#  you plan to build the components.
#
#  Then you need to build memmgr, syslink and d2c components in that order
#  as they depend on one another.
#
#  You may also choose to execute this document and manually enter the
#  required environment variables
#
# *****************************************************************************

DIR=`dirname $0`
HOST=arm-none-linux-gnueabi

# 0) Gather requirements
# =============================================================================

# PREFIX: path to target filesystem
echo "Enter PREFIX (currently '$PREFIX'):\c"
read VALUE
export PREFIX=${VALUE:=$PREFIX}

# USERSPACE_SYSLINK: path to userspace-syslink git root
echo "Enter path to userspace-syslink git root (currently '$USERSPACE_SYSLINK'):\c"
read VALUE
export USERSPACE_SYSLINK=${VALUE:=$USERSPACE_SYSLINK}

# PATH: should contain the build toolchain

#.. first find gcc
TOOL=`which ${HOST}-gcc`
if [ "$TOOL" ]
then
    TOOLBIN=`dirname $TOOL`
else
    echo "Could not find ${HOST}-gcc in PATH."
    TOOLBIN=.
fi

echo "Enter tool path (currently '$TOOLBIN'):\c"
read VALUE
TOOLBIN=${VALUE:=$TOOLBIN}

echo TOOLBIN           is ${TOOLBIN}
echo PREFIX            is ${PREFIX}
echo USERSPACE_SYSLINK is ${USERSPACE_SYSLINK}

export PATH=${TOOLBIN}:$PATH

export TILER_USERSPACE=`readlink -f ${DIR}`

# 1) First you need to build memmgr
# =============================================================================

#.. uncomment to include our unit tests as well
ENABLE_TESTS=--enable-tests

#.. uncomment to export the tilermgr.h header - this is currently needed by
#   syslink
# ENABLE_TILERMGR=--enable-tilermgr

cd ${TILER_USERSPACE}
./bootstrap.sh
./configure --prefix ${PREFIX} --host ${HOST} ${ENABLE_TESTS} ${ENABLE_TILERMGR}
make clean
make
make install

# 2) Second you need to build syslink
# =============================================================================

cd ${USERSPACE_SYSLINK}/syslink
export PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig
./bootstrap.sh
./configure --prefix ${PREFIX} --host ${HOST} ${ENABLE_DEBUG}
make clean
make
make install
cd -

