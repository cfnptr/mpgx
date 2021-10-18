#!/bin/bash
shopt -s nullglob
cd $(dirname "$BASH_SOURCE")

for f in *.vert *.tesc *.tese *.geom *.frag *.comp *.rgen *.rahit *.rchit *.rmiss *.rint *.rcall *.task *.mesh
do
    glslc --target-env=vulkan1.2 -c -O $f
    echo "Compiled \"$f\" shader."
done
