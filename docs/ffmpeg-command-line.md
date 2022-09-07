# FFmpeg 基础

## 查看当前设备列表

通过下面这条命令可以查看当前平台可用接口，其中 `hide_banner` 是隐藏 **FFmpeg** 执行命令时显示编译配置等讯息。

```bash
ffmepg -hide_banner -devices
```

因为我的设备是 Mac，所以可以看到类似下面的内容。  
其中 D. 表示 Demuxing，E 表示 Muxing。

```
Devices:
 D. = Demuxing supported
 .E = Muxing supported
 --
  E audiotoolbox    AudioToolbox output device
 D  avfoundation    AVFoundation input device
 D  lavfi           Libavfilter virtual input device
  E sdl,sdl2        SDL2 output device
 D  x11grab         X11 screen capture, using XCB
```

我们先看看 avfoundation 有哪些其他的选项

```bash
ffmpeg -h demuxer=avfoundation
```

我们看到 `list_devices` 是列出可用的设备

```
Demuxer avfoundation [AVFoundation input device]:
AVFoundation indev AVOptions:
  -list_devices      <boolean>    .D......... list available devices (default false)
  -video_device_index <int>        .D......... select video device by index for devices with same name (starts at 0) (from -1 to INT_MAX) (default -1)
  -audio_device_index <int>        .D......... select audio device by index for devices with same name (starts at 0) (from -1 to INT_MAX) (default -1)
  -pixel_format      <pix_fmt>    .D......... set pixel format (default yuv420p)
  -framerate         <video_rate> .D......... set frame rate (default "ntsc")
  -video_size        <image_size> .D......... set video size
  -capture_cursor    <boolean>    .D......... capture the screen cursor (default false)
  -capture_mouse_clicks <boolean>    .D......... capture the screen mouse clicks (default false)
  -capture_raw_data  <boolean>    .D......... capture the raw data from device connection (default false)
  -drop_late_frames  <boolean>    .D......... drop frames that are available later than expected (default true)
```

那就查看可用的设备

```bash
ffmpeg -hide_banner -list_devices true -f avfoundation -i ''
```

可以看到音频设备 0 跟 1，其中 0 是我当前电脑安装得一个虚拟音频设备软件 BlackHole。  
譬如当你录屏的时候，你想把你当前电脑放得歌给录进去，这个软件就能派上用场。
1 就是一个外接耳机带得麦克风。

```
[AVFoundation indev @ 0x12d704440] AVFoundation video devices:
[AVFoundation indev @ 0x12d704440] [0] Capture screen 0
[AVFoundation indev @ 0x12d704440] AVFoundation audio devices:
[AVFoundation indev @ 0x12d704440] [0] BlackHole 2ch
[AVFoundation indev @ 0x12d704440] [1] External Microphone
: Input/output error
```

## 音频重采样

直接使用下面的命令处理。因为 PCM 是采集到的声音文件，其不含采样率、声道数、采样格式，所以需要分别指定一下。

```bash
ffmpeg -hide_banner -ar 48000 -ac 1 -f f32le -i ./out.pcm -ar 44100 -ac 2 -f s16le ./output.pcm
```

## 使用 AAC 编码器

上面把一个 48000 采样率、单声道、f32le 采样格式的 PCM 转成了一个 44100 采样率、双声道、s16le 采样格式的文件，
然后把新的文件转成 aac 的文件。

```bash
ffmpeg -hide_banner -ar 44100 -ac 2 -f s16le -i ./output.pcm output.aac
```

通过上面这种方式，会使用 **FFmpeg** 自带的 AAC 编码器，我们可以指定它使用第三方的编码器

```bash
ffmpeg -hide_banner -ar 44100 -ac 2 -f s16le -i ./output.pcm -c:a libfdk_aac output.aac
```

因为当前的 **FFmpeg** 在之前手动构建时，已经启用了该编码器，所以这里可以使用它。  
还可以使用 `-profile:a` 来设置编码器规格，使用 `-b:a` 来设置比特率。

```bash
ffmpeg -hide_banner -ar 44100 -ac 2 -f s16le -i ./output.pcm -c:a libfdk_aac -profile:a aac_he_v2 -b:a 32k output.aac
```

**libfdk_aac** 还支持使用一些自定义的选项，譬如 vbr 模式压缩，

```bash
ffmpeg -hide_banner -ar 44100 -ac 2 -f s16le -i ./output.pcm -c:a libfdk_aac -vbr 5 output.aac
```
