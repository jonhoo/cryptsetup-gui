# Maintainer: Jon Gjengset <jon@thesquareplanet.com>
pkgname=cryptsetup-gui
pkgver=0.1
pkgrel=1
epoch=
pkgdesc="Simple GUI for unlocking dm-crypt device"
arch=('any')
url="http://csg.thesquareplanet.com"
license=('MIT')
groups=()
depends=('glib2' 'cryptsetup')
makedepends=('gcc' 'glib2')
conflicts=()
replaces=()
install=$pkgname.install
changelog=
source=("http://csg.thesquareplanet.com/release/$pkgname-$pkgver.tar.gz")
noextract=()
md5sums=('491a720601de5b355ac908090409828b')

build() {
  cd "$srcdir/$pkgname-$pkgver/src"
  make
}

package() {
  cd "$srcdir/$pkgname-$pkgver"
  make DESTDIR="$pkgdir/" install
}

# vim:set ts=2 sw=2 et:
