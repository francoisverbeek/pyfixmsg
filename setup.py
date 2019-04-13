import glob
from distutils.core import setup, Extension

sources = glob.glob("pyfixmsg/cext/*.c")
print(sources)
sources.remove("pyfixmsg/cext/main.c")
module1 = Extension('libpyfix',
                    sources=sources
                    )

setup(name='PackageName',
      version='1.0',
      description='This is a demo package',
      ext_modules=[module1])
