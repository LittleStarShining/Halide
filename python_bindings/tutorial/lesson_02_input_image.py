#!/usr/bin/python3
#
# Halide tutorial lesson 2.
#
# This lesson demonstrates how to pass in input images.
#
# With Halide for Python installed, run
#
#    python3 path/to/lesson_02_input_image.py
#
# in a shell.
#
# - To install Halide for Python from PyPI:
#   - python3 -m pip install halide
#
# - To install Halide for Python from source:
#   - Build and install Halide locally using CMake (see README_cmake.md)
#   - export HALIDE_INSTALL=path/to/halide/install
#   - export PYTHONPATH=$HALIDE_INSTALL/lib/python3/site-packages

import halide as hl
import numpy as np
import imageio.v2 as imageio
import os.path


def main():

    # This program defines a single-stage imaging pipeline that
    # brightens an image.

    # First we'll load the input image we wish to brighten.
    image_path = os.path.join(os.path.dirname(__file__), "images/rgb.png")

    # We create a hl.Buffer object to wrap the numpy array
    input = hl.Buffer(imageio.imread(image_path))
    assert input.type() == hl.UInt(8)

    # Next we define our hl.Func object that represents our one pipeline
    # stage.
    brighter = hl.Func("brighter")

    # Our hl.Func will have three arguments, representing the position
    # in the image and the color channel. Halide treats color
    # channels as an extra dimension of the image.
    x, y, c = hl.Var("x"), hl.Var("y"), hl.Var("c")

    # Normally we'd probably write the whole function definition on
    # one line. Here we'll break it apart so we can explain what
    # we're doing at every step.

    # For each pixel of the input image.
    value = input[x, y, c]
    assert type(value) == hl.Expr

    # Cast it to a floating point value.
    value = hl.cast(hl.Float(32), value)

    # Multiply it by 1.5 to brighten it.
    value = value * 1.5

    # Clamp it to be less than 255, so we don't get overflow when we
    # hl.cast it back to an 8-bit unsigned int.
    value = hl.min(value, 255.0)

    # Cast it back to an 8-bit unsigned integer.
    value = hl.cast(hl.UInt(8), value)

    # Define the function.
    brighter[x, y, c] = value

    # The equivalent one-liner to all of the above is:
    #
    #   brighter[x, y, c] = hl.cast(hl.UInt(8), hl.min(input[x, y, c] * 1.5, 255))
    #
    # In the shorter version:
    # - I skipped the hl.cast to float, because multiplying by 1.5f does
    #   that automatically.
    # - I also used integer constants in hl.clamp, because they get hl.cast
    #   to match the type of the first argument.
    # - I left the h. off hl.clamp. It's unnecessary due to Koenig
    #   lookup.

    # Remember. All we've done so far is build a representation of a
    # Halide program in memory. We haven't actually processed any
    # pixels yet. We haven't even compiled that Halide program yet.

    # So now we'll realize the hl.Func. The size of the output image
    # should match the size of the input image. If we just wanted to
    # brighten a portion of the input image we could request a
    # smaller size. If we request a larger size Halide will throw an
    # error at runtime telling us we're trying to read out of bounds
    # on the input image.
    output_image = brighter.realize(
        [input.width(), input.height(), input.channels()])
    assert output_image.type() == hl.UInt(8)

    # Save the output for inspection. It should look like a bright parrot.
    imageio.imsave("brighter.png", output_image)
    print("Created brighter.png result file.")

    print("Success!")
    return 0


if __name__ == "__main__":
    main()
