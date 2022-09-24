@echo off

set FilesToCompile=main.c

if not exist build (
  mkdir build
)

where /q cl || (
  echo MSVC not found.
  exit /b 1
)

set CompilerFlags=/nologo /Z7 /W3 /Fe"build/software_renderer"
set LinkerFlags=/incremental:no /opt:icf /opt:ref /subsystem:windows
call cl %CompilerFlags% %FilesToCompile% /link %LinkerFlags%