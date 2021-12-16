#!/bin/bash

shopt -s nullglob
cd $(dirname "$BASH_SOURCE")

if ! glslc --version ; then
    echo "Failed to get GLSLC version, please check if Vulkan SDK is installed."
fi

echo ""
echo "Compiling shaders..."

for f in *.vert *.tesc *.tese *.geom *.frag *.comp *.rgen *.rahit *.rchit *.rmiss *.rint *.rcall *.task *.mesh
do
    if glslc --target-env=vulkan1.2 -c -O $f ; then
        echo "Compiled \"$f\" shader."
    else
        echo "Failed to compile \"$f\" shader."
        break
    fi
done
