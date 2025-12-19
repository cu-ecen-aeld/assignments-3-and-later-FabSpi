#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    echo "Building the kernel..."
    # Säubert alte Konfigurationen
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper 
    # Erstellt das Standard-Image für ARM64 --> Standard 
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig 
    # Baut das Kernel-Image und den Device Tree
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
fi
# copy the fully built kernel image from linux-stable to outdir
echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -p "${OUTDIR}/rootfs"
cd "${OUTDIR}/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr/bin usr/lib usr/sbin var/log


cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git --depth 1 --branch ${BUSYBOX_VERSION}
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    #erstelle config für busybox --> bestimmt welche befehle busybox am schluss abdekcen soll
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
else
    cd busybox
fi

# TODO: Make and install busybox in root directory --> holt standardbefehle wie ls, cp ... in das Embedded Linux System
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG_PREFIX="${OUTDIR}/rootfs" install

#Nachdem dieser Befehl durchgelaufen ist, sieht dein Ordner etwa so aus:

#/tmp/aeld/rootfs/bin/busybox (Die eigentliche Logik)
#/tmp/aeld/rootfs/bin/ls -> verlinkt auf busybox
#/tmp/aeld/rootfs/bin/sh -> verlinkt auf busybox
# Wenn man dann im Terminal im Embedded Linux z.B. "ls" aufruft, wird automatisch busybox mit dem Parameter "ls" aufgerufen und führt die entsprechende Funktion aus.

echo "Library dependencies"

cd "${OUTDIR}/rootfs"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
# kopiere benötigte libs aus c-library in leeres root file system von embedded linux, um standardbefehle (printf...) ausführen zu können
cp -L $SYSROOT/lib/ld-linux-aarch64.so.1 "${OUTDIR}/rootfs/lib"
cp -L $SYSROOT/lib64/libm.so.6 "${OUTDIR}/rootfs/lib64"
cp -L $SYSROOT/lib64/libresolv.so.2 "${OUTDIR}/rootfs/lib64"
cp -L $SYSROOT/lib64/libc.so.6 "${OUTDIR}/rootfs/lib64"

# TODO: Make device nodes
# erstellt Schnittstellen zur Außenwelt
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1 # Schnittstelle für Konsole

# TODO: Clean and build the writer utility from writer.c file
cd "${FINDER_APP_DIR}"
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer "${OUTDIR}/rootfs/home/"
cp finder.sh "${OUTDIR}/rootfs/home/"
cp -r conf/ "${OUTDIR}/rootfs/home/"
cp finder-test.sh "${OUTDIR}/rootfs/home/"
cp autorun-qemu.sh "${OUTDIR}/rootfs/home/"

# 1.f.i: Modify finder-test.sh to reference the correct path
sed -i 's|\.\./conf/assignment.txt|conf/assignment.txt|g' "${OUTDIR}/rootfs/home/finder-test.sh"

# TODO: Chown the root directory
cd "${OUTDIR}/rootfs"
# legt fest, dass alle Dateien dem root user gehören, sodass alle Dateien ausgeführt werden können
# da der Kernel beim Boot Vorgang als Root-User beginnt, müssen auch eie restichen Dateien dem root user
# gehören, sodass er nach dem booten befehle wie ls etc. ausführen kann
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio # nimmt alle Dateien aus dem rootfs Ordner und schnürt sie zu einem einzigen Paket zusammen
cd "${OUTDIR}"
gzip -f initramfs.cpio ## komprimiere das Paket

echo "SUCCESS: Kernel Image and initramfs.cpio.gz are ready in ${OUTDIR}"