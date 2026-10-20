"""Optional native AVR tools when PlatformIO's bundled binaries cannot run.

EMBEDDED_UTILS_AVR_PATH is a PATH-style list containing a matching avr-gcc,
avr-g++, avr-gcc-ar, avr-gcc-ranlib, binutils, and their LTO plugin.
Without this variable, use PlatformIO's pinned toolchain normally.
"""
import os
import shutil

Import('env')

tool_paths = os.environ.get('EMBEDDED_UTILS_AVR_PATH')
if tool_paths:
    for path in reversed(tool_paths.split(os.pathsep)):
        if not os.path.isdir(path):
            raise RuntimeError('AVR tool directory does not exist: ' + path)
        env.PrependENVPath('PATH', path)
    for tool in ('avr-gcc', 'avr-g++', 'avr-gcc-ar', 'avr-gcc-ranlib', 'avr-objcopy', 'avr-size'):
        resolved = shutil.which(tool, path=tool_paths)
        if not resolved:
            raise RuntimeError('EMBEDDED_UTILS_AVR_PATH is missing ' + tool)
