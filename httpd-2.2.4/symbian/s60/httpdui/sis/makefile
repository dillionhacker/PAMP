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

include ../../../tools/defines.mk


TARGET=		httpdui

PKG_DEPEND=	../public/httpdui/versionhttpdui.hrh
SIS_DEPEND=    	$(EPOC32RELEASE)/httpdui.exe\
                $(EPOC32)/data/z/private/10003a3f/import/apps/httpdui_reg.rsc\
                $(EPOC32RESOURCE)/httpdui.rsc\
                $(EPOC32RESOURCE)/httpdui.mif\
                $(EPOC32RESOURCE)/httpdui_loc.rsc

CLEAN=		*.pkg


include ../../../tools/targets.mk
