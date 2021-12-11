To run the bare metal demonstration for system calls, you either compile the source code, or use the `kernel8.img` supplied by this repository.

To just run the demonstration without compiling, execute:

```
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
```

If you want to compile this source code, you will need a compiler that compiles for the AArch64 and it's companion utilities

# AArch64 Cross Compiler: download and installation

First of all, download binutils and gcc sources
```sh
wget https://ftpmirror.gnu.org/binutils/binutils-2.30.tar.gz
wget https://ftpmirror.gnu.org/gcc/gcc-8.1.0/gcc-8.1.0.tar.gz
wget https://ftpmirror.gnu.org/mpfr/mpfr-4.0.1.tar.gz
wget https://ftpmirror.gnu.org/gmp/gmp-6.1.2.tar.bz2
wget https://ftpmirror.gnu.org/mpc/mpc-1.1.0.tar.gz
wget https://gcc.gnu.org/pub/gcc/infrastructure/isl-0.18.tar.bz2
wget https://gcc.gnu.org/pub/gcc/infrastructure/cloog-0.18.1.tar.gz
```

Then unpack the tarballs with these commands:

```sh
for i in *.tar.gz; do tar -xzf $i; done
for i in *.tar.bz2; do tar -xjf $i; done
```

Remove the files, we don't need anymore:

```sh
rm -f *.tar.* sha512.sum
```

You'll need some symbolic links before you could start the compilation, so let's create them:

```sh
cd binutils-*
ln -s ../isl-* isl
cd ..
cd gcc-*
ln -s ../isl-* isl
ln -s ../mpfr-* mpfr
ln -s ../gmp-* gmp
ln -s ../mpc-* mpc
ln -s ../cloog-* cloog
cd ..
```
## Compiling the sources

The following builds are going to be installed in `/usr/local/cross-compiler`

Build binutils package:

```sh
mkdir aarch64-binutils
cd aarch64-binutils
../binutils-*/configure --prefix=/usr/local/cross-compiler --target=aarch64-elf \
--enable-shared --enable-threads=posix --enable-libmpx --with-system-zlib --with-isl --enable-__cxa_atexit \
--disable-libunwind-exceptions --enable-clocale=gnu --disable-libstdcxx-pch --disable-libssp --enable-plugin \
--disable-linker-build-id --enable-lto --enable-install-libiberty --with-linker-hash-style=gnu --with-gnu-ld\
--enable-gnu-indirect-function --disable-multilib --disable-werror --enable-checking=release --enable-default-pie \
--enable-default-ssp --enable-gnu-unique-object
make -j4
sudo make install
cd ..
```

(Maybe you will need to install `libgmp3-dev` package to build binutils)

Build gcc compiler:

```sh
mkdir aarch64-gcc
cd aarch64-gcc
../gcc-*/configure --prefix=/usr/local/cross-compiler --target=aarch64-elf --enable-languages=c \
--enable-shared --enable-threads=posix --enable-libmpx --with-system-zlib --with-isl --enable-__cxa_atexit \
--disable-libunwind-exceptions --enable-clocale=gnu --disable-libstdcxx-pch --disable-libssp --enable-plugin \
--disable-linker-build-id --enable-lto --enable-install-libiberty --with-linker-hash-style=gnu --with-gnu-ld\
--enable-gnu-indirect-function --disable-multilib --disable-werror --enable-checking=release --enable-default-pie \
--enable-default-ssp --enable-gnu-unique-object
make -j4 all-gcc
sudo make install-gcc
cd ..
```

The executables produced by these builds are located in `/usr/local/cross-compiler/bin`. Add this directory to your PATH environment variable:

```
export PATH=$PATH:/usr/local/cross-compiler/bin
```

# Compiling the bare metal demonstration

This directory has two Makefiles one for the GNU gcc, and one for LLVM clang. We only tested with the GNU gcc, thus it is recommended to use the GNU gcc Makefile.

To compile the source code, execute:

```
make
```

# Running the bare metal demonstration

```
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
```
