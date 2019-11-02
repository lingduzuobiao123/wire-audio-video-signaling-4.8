
# 维护和修改 Wire - Audio, Video, and Signaling (AVS) 4.8 

由于4.8以后的版本Wire不进行开源了，切不支持android arm64-v8a（google play审核需要） 只能自己根据项目情况进行改版
修改Android的版本 从android-16升级到android-21 使其能编译arm64-v8a
经过修改JDK已经支持1.8
由于源代码已经狠古老NDK还必须是版本:[android-ndk-r14b](https://developer.android.google.cn/ndk/downloads/older_releases.html?hl=zh-cn)


# Wire - Audio, Video, and Signaling (AVS)

This repository is part of the source code of Wire. You can find more information at [wire.com](https://wire.com) or by contacting opensource@wire.com.

You can find the published source code at [github.com/wireapp](https://github.com/wireapp). 

For licensing information, see the attached LICENSE file and the list of third-party licenses at [wire.com/legal/licenses/](https://wire.com/legal/licenses/).

# Build Requirements

Apart from the basic toolchain for each system, you need these:

* clang, libc++
* yasm (for video only)
* alsa (for Linux only).

For **OSX** and **iOS**, you should have Xcode and the Command Line Tools
for your specific version of both OSX and Xcode. Things *will* break if
you have the wrong version. You can install the latter via menu Xcode,
then Open Developer Tool, then More Developer Tools.

For getting autoconf, automake, libtool, and yasm, we suggest [Homebrew](http://brew.sh/).
Follow the instructions there, then:

```bash
$ brew install \
  autoconf \
  automake \
  libsodium \
  libtool \
  multirust \
  pkg-config \
  protobuf-c \
  readline \
  sdl \
  yasm
$ multirust default nightly
```



For **Android**, you need both the
[Android SDK](https://developer.android.com/sdk/index.html) as well as the
[Android NDK](https://developer.android.com/tools/sdk/ndk/index.html). NDK必须是版本:[android-ndk-r14b](https://developer.android.google.cn/ndk/downloads/older_releases.html?hl=zh-cn)
Just get the latest versions and install them somewhere cozy. You need to
export two environment variables ``ANDROID_SDK_ROOT`` and
``ANDROID_NDK_ROOT`` pointing to the respective location. Unless you do a
one-off, you probably want to add them to your ``.bash_profile``.

For **Linux**, you need to install the packages for the stuff mentioned
above or, of course, build it all from scratch. If you are on a
Debian-esque system, do this: 

```bash
$ sudo apt-get install \
  autoconf \
  automake \
  clang \
  libasound2-dev \
  libc++-dev \
  libc++abi-dev \
  libevent-dev \
  libprotobuf-c-dev \
  libreadline-dev \
  libsodium-dev \
  libtool \
  libx11-dev \
  libxcomposite-dev \
  libxdamage-dev \
  libxrender-dev \
  make \
  pkgconf \
  protobuf-c-compiler \
  yasm \
  zlib1g-dev \
  zip

$ curl -sSf https://static.rust-lang.org/rustup.sh | sh -s -- --channel=nightly
```



For **Windows**, you will have to start by adding your system to the build
system. Good luck!


# Build Instructions

The first time you need to fetch the submodules by doing:

```
$ ./prepare.sh    注意：这个不需要了已经帮你准备好了
```

Patches for webrtc are under mediaengine/webrtc_patches. Run apply_patches.sh to apply them 注意：这个也不需要了已经帮你准备好了

If you simply say ``make``, a selection of tools is being built for your
host machine.

The deliverables are being built by saying ``make dist``. You can limit
this to only select target platforms through ``make dist_android``,
``make dist_osx`` and ``make dist_ios``. All of them take quite a while
on a fresh checkout.

You'll find the deliverables in ``build/dist/{android,ios,osx}``.

You can also build just the wrappers for a given architecture by saying
``make wrappers AVS_OS=<os> AVS_ARCH=<arch>`` where ``<os>`` is one of
``android``, ``ios``, or ``osx``. There is no wrappers for Linux, so you
are out of luck there. For ``<arch>`` there are several possible values
depending on the OS. You can just leave the whole thing out and will
receive reasonable defaults (ARMv7 or X86-64). Have a look at
``mk/target.mk`` for more on this.


If you want to have a local version of a ``dist_*`` target that hasn't
all the necessary architectures but builds quicker, you can pass
``DIST_ARCH=<your_arch>`` to make and will only built for that
architecture:

```bash
$ make dist_ios DIST_ARCH=armv7
```

will build an iOS distribution that will only contain armv7 instead of
the usual five architectures.



# Using the Library

During the build, a set of static libraries is being built. You can use
this library in your own projects. 

You'll find the APIs in ``include/*.h``. ``avs.h`` is your catchall
include file. Always use that to protect yourself agains reorganizations.

Linking is a bit tricky, we'll add instructions soon. The easiest is
probably to add ``build/$(your-platform)/lib`` to your library path and
then add all ``.a`` files in there as ``-l`` arguments.


# Architecture overview:


```
                      .----------.  .------.
                      |  Engine  |  | Mill |
                      '----------'  '------'
                     /      |
    .--------------.        |
    |   EGCALL     |  .----------.  .-----------.  .----------.
    |   ECALL      |  | REST     |  | Media-mgr |  | Netprobe |
    |   ECONN      |  | Nevent   |  '-----------'  '----------'
    '--------------'  '----------'
       |      |
  .---------. |  .-----.
  |Mediaflow| |  | DCE |
  '---------'\|  '-----'
       |      |\                                   .----------.
   .-------.  | \.--------.                        | Protobuf |
   |aucodec|  |  |vidcodec|                        '----------'
   '-------'  |  '--------'
       |     / \     |                             .----------.
   .-------./   \.--------.                        | Conf-Pos |
   |  VOE  |     |  VIE   |                        '----------'
   '-------'     '--------'



    .------------------------------.
    | Low-level utility modules:   |
    | - audummy (Dummy audio-mod)  |
    | - base (Base module)         |
    | - cert (Certificates)        |
    | - dict (Dictionary)          |
    | - jzon (Json wrappers)       |
    | - log (Logging framework)    |
    | - queue (Packet queue)       |
    | - sem (Semaphores)           |
    | - store (Persistent Storage) |
    | - trace (Tracing tool)       |
    | - uuid (UUID helpers)        |
    | - zapi (ZETA-protocol API)   |
    | - ztime (Timestamp helpers)  |
    '------------------------------'
```




Some specifications implemented:
-------------------------------

* https://tools.ietf.org/html/draft-ietf-mmusic-trickle-ice-01
* https://tools.ietf.org/html/draft-ietf-rtcweb-stun-consent-freshness-11
* https://tools.ietf.org/html/rfc7845
* https://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13


# Reporting bugs

When reporting bugs against AVS please include the following:

- Wireshark PCAP trace ([download Wireshark](https://www.wireshark.org/download.html))
- Full logs from client
- Session-ID
- Which Backend was used
- Exact version of client
- Exact time when call was started/stopped
- Name/OS of device
- Adb logcat for Android


# 如果出现一下错误：
No rule to make target `crypto_stream/salsa20/xmm6/salsa20_xmm6-asm.S', needed by `crypto_stream/salsa20/xmm6/libsodium_la-salsa20_xmm6-asm.lo'.  Stop.


No rule to make target `crypto_scalarmult/curve25519/sandy2x/consts.S', needed by `crypto_stream/salsa20/xmm6/libsodium_la-salsa20_xmm6-asm.lo'.  Stop.


No rule to make target `crypto_scalarmult/curve25519/sandy2x/sandy2x.S', needed by `crypto_scalarmult/curve25519/sandy2x/libsodium_la-sandy2x.lo'.  Stop.

make[4]: *** No rule to make target `crypto_scalarmult/curve25519/sandy2x/fe51_mul.S', needed by `all-am'.  Stop.

make[4]: *** No rule to make target `crypto_scalarmult/curve25519/sandy2x/fe51_nsquare.S', needed by `all-am'.  Stop.

make[4]: *** No rule to make target `crypto_scalarmult/curve25519/sandy2x/fe51_pack.S', needed by `all-am'.  Stop.

make[4]: *** No rule to make target `crypto_scalarmult/curve25519/sandy2x/ladder.S', needed by `all-am'.  Stop.

make[4]: *** No rule to make target `crypto_scalarmult/curve25519/sandy2x/ladder_base.S', needed by `all-am'.  Stop.

把 crypto_stream.zip和crypto_scalarmult.zip 解压替换：contrib/sodium/src/libsodium/下的crypto_stream和crypto_scalarmult文件

#更新contrib/中第三方库的版本
* cryptobox-c @ 9a9df46             9a9df46   2017/9/1
* generic-message-proto @ ea08048    ea08048   2017/9/25
* googletest @ ec44c6c               ec44c6c  2016/7/15
* libvpx @ dc2656f                  f80be22a1         2018年1月25日 GMT+8 上午6:25:44 vp8/common/postproc.c -- 69行-  const MODE_INFO *mode_info_context = mi;注释用上一行了
* openssl @ b2758a2                 b2758a2   2017/11/2 
* opus @ aa32042                   defbc370         2017年6月27日 GMT+8 上午2:04:09
* rem @ 81834ad                  8d572e6         2018年11月23日 GMT+8 下午8:59:48
* rew @ 9ce0a92                  24c91fd         2018年3月27日 GMT+8 下午7:26:59
* sodium @ 7d5d920                  675149b9         2017年12月13日 GMT+8 下午5:24:13
* usrsctp @ 0e07626                  0e07626         2017年11月25日 GMT+8 上午3:45:25

注意：更新之后执行 make 有下面错误：（但是执行 make dist_android 正常）
dongxulin:wire-audio-video-signaling-4.8 mac8$ make
  LD      ztest
Undefined symbols for architecture x86_64:
  "_crypto_stream_aes128ctr", referenced from:
      sodiumoxide::crypto::stream::aes128ctr::stream::heb5936bfc6e1503f in libcryptobox.a(sodiumoxide-3025ede55992078f.sodiumoxide.7ny67lrf-cgu.2.rcgu.o)
  "_crypto_stream_aes128ctr_xor", referenced from:
      sodiumoxide::crypto::stream::aes128ctr::stream_xor::ha6931bba5605dd72 in libcryptobox.a(sodiumoxide-3025ede55992078f.sodiumoxide.7ny67lrf-cgu.2.rcgu.o)
      sodiumoxide::crypto::stream::aes128ctr::stream_xor_inplace::h86df1ad51b40afd7 in libcryptobox.a(sodiumoxide-3025ede55992078f.sodiumoxide.7ny67lrf-cgu.2.rcgu.o)
ld: symbol(s) not found for architecture x86_64
clang: error: linker command failed with exit code 1 (use -v to see invocation)
make: *** [ztest] Error 1
