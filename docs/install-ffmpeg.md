# 安装 FFmpeg

## 一般方式安装 FFmpeg

**Mac** 平台可以直接使用 [Homebrew](https://brew.sh/) 安装，
旧版本系统(**<= macOS 10.14**)可以使用 [MacPorts](https://www.macports.org) 安装。

```bash
brew install ffmpeg
# or
sudo port install ffmpeg
```

如果要在 **FFmpeg** 里集成第三方的库（譬如 **libfdk-aac**），可以使用其他的 **homebrew** 源配合可选参数安装。

```bash
brew tap homebrew-ffmpeg/ffmpeg
brew install homebrew-ffmpeg/ffmpeg/ffmpeg --with-fdk-aac
```

## 源码方式安装

另一种方式就是直接从 **FFmpeg** 源码构建对应的库跟命令程序，有这个需求是因为自己可能想启用或者禁用 **FFmpeg** 的某些功能。

#### 建议

可以先用 **homebrew** 或者 **MacPorts** 安装一遍 **FFmpeg**，再卸载 **FFmpeg**
，它自动会把很多相关的依赖安装好。

### 安装一些依赖

安装一下汇编工具

```bash
brew install yasm
or
sudo port install yasm
```

如果想成功构建 **ffplay** 则需要安装好 **SDL2**。

```bash
brew install sdl2
or
sudo port install libsdl2
```

如果想构建时同时加入 libfdk-aac 之类的依赖，也需要提前安装好对应的库

```bash
brew install fdk-aac
or
sudo port install libfdk-aac
```

先下载 **FFmpeg** 源码，进入目录后，再切换分支，譬如这里使用 4.4 的分支，
如果 **git** 版本不高，可以使用 checkout 命令。

```bash
git clone https://github.com/FFmpeg/FFmpeg.git

cd FFmpeg

git switch release/4.4 (git checkout release/4.4)
```

然后跑一下配置文件，其中 `$PREFIX` 指得是一段路径，到时安装时会把它安装到指定位置，可以在命令里直接写（譬如我用得是
/usr/local/Cellar/ffmpeg），也可以搞成环境变量。
因为这里用得是 **Mac** 设备，可以使用 **clang** 作为编译器的前端，如果是其他平台，就用对应平台的 **C** 编译器，
如果是 **ARM** 芯片的 **Mac** 需要改成 `--arch=arm64` 的编译选项，如果设备上有多个版本的
clang，建议用 `--cc=/usr/bin/clang`，
让它使用 **Xcode** 自带的命令行工具。

**如果 ARM 芯片的设备构建成功后，命令跑不起来，那还是直接用第三方的 homebrew 源来安装**

我这里启用了很多选项，根据自己当前的需求增加或者删除某些选项（譬如你根本不需要 libopus，那你就可以把 `--enable-libopus`
选项给删掉）。

```bash
./configure --prefix=$PREFIX \
           --enable-swscale \
           --enable-avfilter \
           --enable-avresample \
           --enable-libmp3lame \
           --extra-ldflags=-L/opt/local/lib --extra-cflags=-I/opt/local/include \
           --enable-libvorbis \
           --enable-libopus \
           --enable-librsvg \
           --enable-libtheora \
           --enable-libopenjpeg \
           --enable-libmodplug \
           --enable-libvpx \
           --enable-libsoxr \
           --enable-libspeex \
           --enable-libass \
           --enable-libbluray \
           --enable-libzimg \
           --enable-libzvbi \
           --enable-lzma \
           --enable-gnutls \
           --enable-fontconfig \
           --enable-libfreetype \
           --enable-libfribidi \
           --enable-zlib \
           --disable-libjack \
           --disable-libopencore-amrnb \
           --disable-libopencore-amrwb \
           --disable-libxcb \
           --disable-libxcb-shm \
           --disable-libxcb-xfixes \
           --disable-indev=jack \
           --enable-opencl \
           --disable-outdev=xv \
           --enable-audiotoolbox \
           --enable-videotoolbox \
           --enable-sdl2 \
           --disable-securetransport \
           --mandir=/usr/local/share/man \
           --enable-shared \
           --enable-pthreads 
           --cc=/usr/bin/clang \
           --enable-libdav1d \
           --enable-libaom \
           --enable-librav1e \
           --enable-libsvtav1 \
           --arch=x86_64 \
           --enable-x86asm \
           --enable-gpl \
           --enable-postproc \
           --enable-libx264 \
           --enable-libx265 \
           --enable-libxvid \
           --enable-libvidstab \
           --enable-libfdk-aac \
           --enable-nonfree
```

如果弄不清爽这些参数是干什么的，可以使用类似下面这种方式来查看参数的含意。

```bash
./configure --help | grep version3

结果
--enable-version3        upgrade (L)GPL to version 3 [no]
```

### libmp3lame 错误

执行 configure 期间发现在有一个问题，就是找不到依赖在哪，所以添加了两个依赖相关的参数，
因为我这个设备系统不是最新的，所以先是使用 **MacPorts** 安装 **FFmpeg**，
发觉它给我安装得是 4.4 版本以及一堆依赖，于是我给 `configure` 提供了第三方库跟所在的路径。

```bash
...
--extra-ldflags=-L/opt/local/lib --extra-cflags=-I/opt/local/include \
...
```

##

等 `configure` 执行结束后，就可以直接执行 make 编译代码，完事后再 make install 把构建后的东西安装到之前设置的
prefix 对应的路径里。

```bash
make -j8 && make install
```

亲测在 **macOS 10.14** 的设备上通过 **Xcode 11** 从源码安装成功。

----

**Linux** 系统构建 **FFmpeg** 跟 **Mac** 类似，安装依赖使用对应系统的安装工具就行。手头没有
**wINDOWS** 设备，弄不清爽 **wINDOWS** 版如何从源码构建。
