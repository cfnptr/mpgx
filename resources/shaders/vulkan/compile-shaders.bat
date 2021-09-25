@echo off

for %%f in (*.vert *.tesc *.tese *.geom *.frag *.comp *.rgen *.rahit *.rchit *.rmiss *.rint *.rcall *.task *.mesh) do (
    echo Compiling "%%f" shader...
    glslc -c -O %%f
)

pause
