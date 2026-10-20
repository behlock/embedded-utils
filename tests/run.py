#!/usr/bin/env python3
"""Compile and run host regressions without Arduino hardware or Python packages."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DRIVER = ROOT / 'MLX90614/lib/Adafruit MLX90614 Library'
compiler = shlex.split(os.environ.get('CXX', 'c++'))
flags = ['-std=c++11', '-Wall', '-Wextra', '-Werror', '-g']
if os.environ.get('SANITIZE', '1') == '1':
    flags += ['-fsanitize=address,undefined,float-cast-overflow', '-fno-sanitize-recover=all']
includes = [ROOT, ROOT / 'tests/stubs', DRIVER, ROOT / 'MLX90614/include']
base = compiler + flags + ['-I' + str(p) for p in includes]
checks = [('dsp', [], []), ('sensors', [DRIVER / 'Adafruit_MLX90614.cpp'], []),
          ('heater_sketch', [], []), ('pt100', [], []),
          ('pwm_audio', [], ['-D__AVR_ATmega328P__', '-DF_CPU=16000000UL']),
          ('pwm_audio', [], ['-D__AVR_ATmega2560__', '-DF_CPU=16000000UL'])]
with tempfile.TemporaryDirectory(prefix='embedded-utils-tests-') as output:
    for index, (name, extra_sources, defines) in enumerate(checks):
        binary = Path(output) / (name + str(index))
        subprocess.run(base + defines + [str(ROOT / 'tests' / (name + '.cpp')),
                       str(ROOT / 'tests/fakes.cpp')] + [str(p) for p in extra_sources]
                       + ['-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    sketches = sorted((ROOT / 'MLX90614/src').glob('*.cpp'))
    sketches += [ROOT / 'E3D PT100 Amplifier/src/read_temp.cpp']
    for sketch in sketches:
        subprocess.run(base + ['-fsyntax-only', str(sketch)], check=True)
    print(f'{len(checks)} regression executables and {len(sketches)} sketch syntax checks passed.')
