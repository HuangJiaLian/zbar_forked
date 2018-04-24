### zbar的编译与使用

#### 在PC端使用zbar

编译：
0. 命令行输入`export CFLAGS=""` 

1. 在`zbar_forked/`目录下新建一个文件夹`x64`用来存放64位Linux计算机使用的`zbar`库

2. 在`zbar_forked/`目录下执行下面的命令

   说明：`--prefix`后的路径要是绝对路径，针对你的路径做相应的修改。
``` bash
./configure --prefix=/Your/Path/zbar-0.10/x64/ --disable-video  --without-imagemagick --without-qt --without-gtk --without-python --enable-video=no --without-x --without-jpeg
```
3. `make clean`
4. `make`
5. `make install`

以上步骤顺利执行后，在`x64`目录下会生成宝库解码库在内的文件如下：

```
├── bin
├── include
│   ├── zbar
│   │   ├── Decoder.h
│   │   ├── Exception.h
│   │   ├── Image.h
│   │   ├── ImageScanner.h
│   │   ├── Processor.h
│   │   ├── Scanner.h
│   │   ├── Symbol.h
│   │   ├── Video.h
│   │   └── Window.h
│   └── zbar.h
├── lib
│   ├── libzbar.a
│   ├── libzbar.la
│   ├── libzbar.so -> libzbar.so.0.2.0
│   ├── libzbar.so.0 -> libzbar.so.0.2.0
│   ├── libzbar.so.0.2.0
│   └── pkgconfig
│       └── zbar.pc
└── share
    ├── doc
    │   └── zbar
    │       ├── COPYING
    │       ├── HACKING
    │       ├── INSTALL
    │       ├── LICENSE
    │       ├── NEWS
    │       ├── README
    │       └── TODO
    ├── man
    │   └── man1
    └── zbar

```

至此，zbar解码库已经生成好了。接下来就是如何调用zbar解码库了。

#### **在应用程序中使用生成好的zbar库**：

在这个例子中，我们要使用两个独立的头文件`stb_image.h`，`stb_image_write.h` 来加载写入图片：在[这里](https://github.com/nothings/stb/blob/master/stb_image.h)和[这里](https://github.com/nothings/stb/blob/master/stb_image_write.h)下载。

现在我在`x64`目录下建立一个文件夹`examples`来放我的例子程序，此时我文件结构是下面这个样子：

```bash
falcon@falconPro ~/桌面/Github/zbar_forked/x64 $ ls
bin  examples  include  lib  share
```

进入`examples`目录新建一个`main.c`文件如下：

```c
#include <stdio.h>
#include "../include/zbar.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


zbar_image_scanner_t *scanner = NULL;

int main(int argc, char const *argv[])
{
	if(argc < 2) 
		{
			printf("usage: %s imageName\n", argv[0]);
			return(1);
		}


	/* create a reader */
    scanner = zbar_image_scanner_create();

    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);


	/* load single channel image data */
	const char *inputFile = argv[1];
	int width = 0, height = 0, n = 0;
	void *raw = NULL;
	raw = stbi_load(inputFile, &width, &height, &n, 0);




    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"GREY");
    // zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);


    /* scan the image for barcodes */
    int num = zbar_scan_image(scanner, image);

    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *results = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n",zbar_get_symbol_name(typ), results);
    }

    /* clean up */
    zbar_image_scanner_destroy(scanner);
	stbi_image_free(raw);

	return(0);
}
```

**编译**：

```
export LIBRARY_PATH=../lib/
gcc main.c -o decoder -lm -lzbar 
```

**运行程序**:

在[这里](pic/01.png)下载测试图片，运行：

```
LD_LIBRARY_PATH=../lib/ ./decoder 01.png
```

注意：测试图片中的图片是单通道的png图片，这样做的目的是考虑到利用这些代码稍作修改便可以在嵌入式设备中使用。



### 在嵌入式端使用zbar

要在嵌入式端使用zbar最重要的就是利用交叉编译工具链，编译生成对应平台的zbar库。

下面是利用君正x1000平台的编译过程，供参考。

**配置工具链：**

1.编辑　~/.bashrc
添加下面几句，路径改为实际对应交叉工具链的路径
```
# MIPS toolchains
export PATH=/Your/Path/X1000/prebuilts/toolchains/mips-gcc472-glibc216/bin:$PATH
export LD_LIBRARY_PATH=/Your/Path/X1000/prebuilts/toolchains/mips-gcc472-glibc216/lib
```
2.使文件生效
```
source ~/.bashrc
```
3.测试交叉工具链是否搭建成功
在任意目录下编辑一个简单的C语言源文件例如
hello.c
利用交叉工具链的gcc来编译是否能够生成对应的二进制文件
```
mips-linux-gnu-gcc hello.c　-o hello 
```
若能生成可执行文件hello

则说明配置好交叉编译环境了

接下来就是交叉编译生成库了：

0. 命令行输入`export CFLAGS=""` 

1. 在`zbar_forked/`目录下新建一个文件夹`mips`用来存放嵌入式开发版使用的`zbar`库

2. 在`zbar_forked/`目录下以此执行下面4条命令

   说明：`--prefix`后的路径要是绝对路径，针对你的路径**做**相应的**修改**。
``` bash
CROSS_COMPILE=mips-linux-gnu-

CC=mips-linux-gnu-gcc

export CROSS_COMPILE CC

./configure --prefix=/Your/Path/zbar/mips/ --without-imagemagick --without-qt --without-gtk --without-python --enable-video=no --without-x --without-jpeg  --enable-pthread=no --host=mips --target=mips-linux
```
3. `make clean`
4. `make`
5. `make install`

以上步骤顺利执行后，在`mips`目录下会生成zbar解码库在内的文件。

在开发版上库的调用可以参考PC端库的调用。(我没有去测试过)