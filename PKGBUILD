pkgname=wayland-defuser
pkgver=1
pkgrel=2
pkgdesc='Makes Wayland go away (until it is usuable, aka never)'
arch=('i686' 'x86_64' 'armv6h' 'armv7h' 'aarch64')
license=('WTFPL')
provides=('wayland' 'wayland-protocols')
conflicts=('wayland' 'wayland-protocols')
makedepends=('gcc' 'gcc-libs')

build() {
	# Build the shared library that spoofs a Wayland client interface.
	gcc -shared -w -o libwayland-client.so libwayland-client.c libwayland-list.c libwayland-map.c
	# Build the shared library that spoofs a Wayland server interface.
	gcc -shared -w -o libwayland-server.so libwayland-server.c libwayland-list.c libwayland-map.c
	# Build the shared library that spoofs the Wayland cursor API.
	gcc -shared -w -o libwayland-cursor.so libwayland-cursor.c
	# Build the shared library that spoofs the Wayland EGL interface.
	gcc -shared -w -o libwayland-egl.so libwayland-egl.c
}

package() {
	install -Dm755 "libwayland-client.so" "${pkgdir}/usr/lib/libwayland-client.so.0"
	install -Dm755 "libwayland-server.so" "${pkgdir}/usr/lib/libwayland-server.so.0"
	install -Dm755 "libwayland-cursor.so" "${pkgdir}/usr/lib/libwayland-cursor.so.0"
	install -Dm755 "libwayland-egl.so" "${pkgdir}/usr/lib/libwayland-egl.so.1"
	
	cd "$pkgdir"/usr/lib/

	ln -s "libwayland-client.so.0" "libwayland-client.so"
	ln -s "libwayland-server.so.0" "libwayland-server.so"
	ln -s "libwayland-cursor.so.0" "libwayland-cursor.so"
	ln -s "libwayland-egl.so.1" "libwayland-egl.so"
}
