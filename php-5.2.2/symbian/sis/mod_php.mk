# Copyright 2006 Nokia Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include ../tools/defines.mk


TARGET=		mod_php


PKG_DEPEND=	mod_php.files
SIS_DEPEND=	$(EPOC32RELEASE)/httpd22-mod_php.so\
		data/php/php.ini

CLEAN=		mod_php*.pkg


include ../tools/targets.mk
