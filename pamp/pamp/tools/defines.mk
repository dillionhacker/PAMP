# Copyright 2008 Nokia Corporation
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

ifeq ($(EPOCROOT),)
$(error You must specify EPOCROOT)
endif

# Backslashes are problematic anywhere you do string-substitution, so
# we'll create another variable for that purpose
#
EPOCROOT_WITH_SLASHES=$(subst \,/,$(EPOCROOT))

# Current working directory. We assume the shell is Window's cmd.
#
CWD=$(shell cd)

# Current drive.
#
CDR=$(shell echo $(CWD) | perl -pe "s/:.*/:/g")

# The full path to the epoc and epoc32-directory.
#
# NOTE: Former with trailing slash, latter without.
#
EPOCPATH=$(CDR)$(EPOCROOT_WITH_SLASHES)
EPOC32=$(EPOCPATH)epoc32


#
# We don't care about macros if we are cleaning.
# 

ifneq ($(MAKECMDGOALS),clean)


ifeq ($(MAKECMDGOALS),sisx)

ifeq ($(PLATFORM),)
$(error Macro/env-var PLATFORM not specified. Must be wins, thumb, gcce, etc)
endif

ifeq ($(BUILD),)
# By default we assume urel.
BUILD=urel
endif

ifeq ($(CERTIFICATE),)
$(warning Macro/env-var CERTIFICATE not specified. It should refer to file containing signing certificate)
endif

ifeq ($(KEY),)
$(warning Macro/env-var KEY not specified. It should refer to file containing signing key)
endif

# ($(MAKECMDGOALS),sisx) 
endif


ifeq ($(PLATFORM),)
PLATFORM=unknown
endif

ifeq ($(BUILD),)
BUILD=unknown
endif

EPOC32INCLUDE=$(EPOC32)/include

ifeq ($(EPOC32RELEASE),)
EPOC32RELEASE=$(EPOC32)/release/$(PLATFORM)/$(BUILD)
endif

ifeq ($(EPOC32RESOURCE),)
EPOC32RESOURCE=$(EPOC32)/data/z/resource/apps
endif

# ($(MAKECMDGOALS), clean)
endif

DEFAULT_CLEAN=*~ *.sis *.SIS *.sisx *.SISX

ifeq ($(MAKECMDGOALS), sis)
SIS_SUFFIX=sis
else
SIS_SUFFIX=sisx
endif

TAIL=_$(PLATFORM)_$(BUILD)
SIS_TAIL=$(TAIL).$(SIS_SUFFIX)
