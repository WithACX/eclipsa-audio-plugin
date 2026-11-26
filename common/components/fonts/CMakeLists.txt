# Copyright 2025 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# A separate namespace and header keep these clear of the icons' BinaryData.
juce_add_binary_data(font_data
    HEADER_NAME EclipsaFonts.h
    NAMESPACE EclipsaFonts
    SOURCES
        Audiowide-Regular.ttf
        Roboto-Regular.ttf)
