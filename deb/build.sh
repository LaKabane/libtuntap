#!/bin/sh

die () {
    >&2 echo "$1"
    exit 1
}


(
    rm -rf package || die "Failed to remove old packages"
    rm -rf ../build || die "Failed to remove build directory"
)

(
    mkdir ../build
    cd ../build
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr .. || die "Failed to configure project"
    make -j "$(nproc)" || die "Failed to build project"
    make DESTDIR="../deb/package/" install || die "Failed to install project into package directory"
)

mkdir -p package/DEBIAN/
VERSION="$(git describe --long | sed 's/^libtuntap-//')"
cp control.template package/DEBIAN/control
sed "s/%VERSION%/$VERSION/" -i package/DEBIAN/control
sed "s/%ARCHITECTURE%/$(dpkg --print-architecture)/" -i package/DEBIAN/control

mkdir -p package/usr/share/doc/libtuntap-dev/
cp copyright package/usr/share/doc/libtuntap-dev/copyright

echo "Building package..."
dpkg-deb --root-owner-group --build package libtuntap-dev.deb || die "Failed to build package"

echo "Package $(dpkg-deb --show libtuntap-dev.deb | sed "s/\t/ /") successful builded!"

