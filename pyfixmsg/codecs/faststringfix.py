"""This codec implements a simpler repeating group logic where the first tag is seen as a marker
for repetition in a repeating group (rather than relying on the order of the tags) """
import re
import sys

import libpyfix

SEPARATOR = '\1'
"""
Standard separator for the StringFIX codec between tag value pairs.
However, the codec supports an arbitrary separator as using | or ; or "<SOH>" is
common.
"""

DELIMITER = '='
FIX_REGEX_STRING = r'([^{s}{d}]*)[{d}](.*?){s}(?!\w+{s})'
FIX_REGEX = re.compile(FIX_REGEX_STRING.format(d=DELIMITER, s=SEPARATOR).encode('UTF-8'), re.DOTALL)
MICROSECONDS = 0
MILLISECONDS = 1

if sys.version_info.major >= 3:
    unicode = str  # pylint: disable=W0622,C0103


class FixMsg:
    def __init__(self, buff):
        self._store = libpyfix.FixMsg(buff)

    def items(self):
        return ((k, self._store[k]) for k in list(self._store))

    def update(self, input):
        for key, value in input.items():
            self._store[key] = value

    def keys(self):
        for key in self._store:
            yield key

    def __getitem__(self, key):
        return self._store[key]


class Codec(object):
    """
    FIX codec.  initialise with a :py:class:`~pyfixmsg.reference.FixSpec` to support
    repeating groups.

    This class is used to transform the serialised FIX message into an instance of ``fragment_class``, default ``dict``
    Tags are assumed to be all of type ``int``, repeating groups are lists of ``fragment_class``

    Values can either bytes or unicode or a mix thereof, depending on the constructor arguments.

    """

    def __init__(self, spec=None, no_groups=False, fragment_class=dict, decode_as=None, decode_all_as_347=False):
        return

    def parse(self, buff, delimiter=DELIMITER, separator=SEPARATOR):
        return FixMsg(buff)

    def serialise(self, msg, separator=SEPARATOR, delimiter=DELIMITER, encoding=None):
        return
