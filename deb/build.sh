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
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -B build -S ".." || die "Failed to configure project"
    cmake --build build || die "Failed to build project"
    DESTDIR="../deb/package/" cmake --install build
)

mkdir -p package/DEBIAN/
VERSION="$(git describe --long | sed 's/^libtuntap-//')"
[ "$VERSION" ] || die "Failed to get version string"
ARCH="$(dpkg --print-architecture)"
[ "$ARCH" ] || die "Failed to get architecture string"

cp control.template package/DEBIAN/control
sed "s/%VERSION%/$VERSION/" -i package/DEBIAN/control
sed "s/%ARCHITECTURE%/$ARCH/" -i package/DEBIAN/control

mkdir -p package/usr/share/doc/libtuntap-dev/
cp copyright package/usr/share/doc/libtuntap-dev/copyright

echo "Building package..."
dpkg-deb --root-owner-group --build package libtuntap-dev.deb || die "Failed to build package"

echo "Package $(dpkg-deb --show libtuntap-dev.deb | sed "s/\t/ /") successful builded!"

