#!/usr/bin/env python3
import os
import setuptools
import subprocess

from distutils.command.build_ext import build_ext


class kphys_build_ext(build_ext):
    def run(self):
        build_temp = os.path.abspath(self.build_temp)
        build_lib = os.path.abspath(self.build_lib)

        p = subprocess.Popen([
            'cmake',
            '-B', build_temp,
            '-DCMAKE_INSTALL_PREFIX='])
        p.wait()

        p = subprocess.Popen([
            'make',
            '-C', build_temp,
            'install', f'DESTDIR={build_lib}'])
        p.wait()


if __name__ == "__main__":
    setuptools.setup(
        name='panda3d-krender',
        version='0.0.11',
        description='Deferred Render Pipeline for Panda3D',
        long_description=(
            'Deferred Render Pipeline for Panda3D '
            'originally made for the KITSUNETSUKI project game'),
        url='https://github.com/kitsune-ONE-team/panda3d-krender',
        download_url='https://github.com/kitsune-ONE-team/panda3d-krender',
        author='Yonnji',
        license='MIT',
        packages=[
            'krender',
            'krender.shader',
        ],
        package_data={'krender.shader': ['*.glsl']},
        install_requires=['panda3d'],
        ext_modules=[
            setuptools.Extension('krender.core', sources=[]),
        ],
        cmdclass={
            'build_ext': kphys_build_ext,
        },
    )
