# Protocol Buffers - Google's data interchange format
# Copyright 2008 Google Inc.  All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd
"""Test that Kokoro is using the intended backend implementation of python."""

import os
import sys
import unittest

from google.protobuf.internal import api_implementation


class PythonApiTest(unittest.TestCase):

  def testExpectedBackend(self):
    """Test that a python test is using the expected backend."""
    backend_string = os.getenv('PROTOCOL_BUFFERS_EXPECTED_PYTHON_BACKEND')

    expected_api_type = "upb"
    if backend_string == "Pure":
      expected_api_type = "python"
    elif backend_string == "C++":
      expected_api_type = "cpp"
    elif backend_string == "upb":
      expected_api_type = "upb"

    self.assertEqual(expected_api_type, api_implementation.Type())



if __name__ == '__main__':
  unittest.main()
