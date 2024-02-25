#include "Halide.h"
#include <stdio.h>

int main(int argc, char **argv) {

    // 'Func'对象代表一个图像处理阶段。它是一个纯函数，定义了每个像素应该具有的数值
    Halide::Func gradient;

    // Var对象是在定义Func时用作变量的名称。
    Halide::Var x, y;
    
    // 定义一个值为x + y的Expr
    Halide::Expr e = x + y;

    // 为Func对象添加一个定义。在像素坐标x, y处，图像的值将是Expr e的值
    gradient(x, y) = e;
    
    // 将创建一个 800 x 600 的图像。
    Halide::Buffer<int32_t> output = gradient.realize({800, 600});
    
    // check
    for (int j = 0; j < output.height(); j++) {
        for (int i = 0; i < output.width(); i++) {
            if (output(i, j) != i + j) {
                printf("Something went wrong!\n"
                       "Pixel %d, %d was supposed to be %d, but instead it's %d\n",
                       i, j, i + j, output(i, j));
                return -1;
            }
        }
    }
    
    // 成功！定义了一个Func，然后在其上调用了'realize'，以生成并运行产生一个Buffer的机器码
    printf("Success!\n");

    return 0;
}
