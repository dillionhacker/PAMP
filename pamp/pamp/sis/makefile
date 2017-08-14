# Copyright 2007 Nokia Corporation
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

TARGET=		pamp

PKG_DEPEND=	versionpamp.hrh
SIS_DEPEND=	mysql$(SIS_TAIL)\
	   	httpd$(SIS_TAIL)\
	   	httpdutil$(SIS_TAIL)\
	   	mod_php$(SIS_TAIL)\
		mod_python$(SIS_TAIL)\
	   	pampui$(SIS_TAIL)\
		httpdconf.$(MAKECMDGOALS)\
		htdocs.$(MAKECMDGOALS)

CLEAN=		pamp_*.pkg

include ../tools/targets.mk


.PHONY: mysqld httpd mod_php mod_python httpdviews pampui 


mysqld:
	$(MAKE) -C ../../mysql-5.0.67/symbian/sis $(MAKECMDGOALS)

httpd:
	$(MAKE) -C ../../httpd-2.2.4/symbian/sis $(MAKECMDGOALS)

httpdutil:
	$(MAKE) -C ../../httpd-2.2.4/symbian/s60/httpdutil/sis $(MAKECMDGOALS)

mod_php:
	$(MAKE) -C ../../php-5.2.2/symbian/sis $(MAKECMDGOALS)

mod_python:
	$(MAKE) -C ../../mod_python-3.3.1/symbian/sis $(MAKECMDGOALS)

pampui:
	$(MAKE) -C ../pampui/sis $(MAKECMDGOALS)



mysql$(SIS_TAIL):: mysqld

mysql$(SIS_TAIL):: ../../mysql-5.0.67/symbian/sis/mysql$(SIS_TAIL)
	perl -MFile::Copy -e "copy('$<', '.');"
	perl -MFile::Copy -e "copy('../../mysql-5.0.67/symbian/sis/mysql_bin$(SIS_TAIL)', '.');"
	perl -MFile::Copy -e "copy('../../mysql-5.0.67/symbian/sis/mysql_data$(SIS_TAIL)', '.');"
	perl -MFile::Copy -e "copy('../../mysql-5.0.67/symbian/sis/mysqldutil$(SIS_TAIL)', '.');"


httpd$(SIS_TAIL):: httpd 

httpd$(SIS_TAIL):: ../../httpd-2.2.4/symbian/sis/httpd$(SIS_TAIL)
	perl -MFile::Copy -e "copy('$<', '.');"
	perl -MFile::Copy -e "copy('../../httpd-2.2.4/symbian/sis/httpds$(SIS_TAIL)', '.');"
	perl -MFile::Copy -e "copy('../../httpd-2.2.4/symbian/sis/httpdc$(SIS_TAIL)', '.');"


httpdutil$(SIS_TAIL):: httpdutil

httpdutil$(SIS_TAIL):: ../../httpd-2.2.4/symbian/s60/httpdutil/sis/httpdutil$(SIS_TAIL)
	perl -MFile::Copy -e "copy('$<', '.');"


mod_php$(SIS_TAIL):: mod_php

mod_php$(SIS_TAIL):: ../../php-5.2.2/symbian/sis/mod_php$(SIS_TAIL)
	perl -MFile::Copy -e "copy('$<', '.');"


mod_python$(SIS_TAIL):: mod_python

mod_python$(SIS_TAIL):: ../../mod_python-3.3.1/symbian/sis/mod_python$(SIS_TAIL)
	perl -MFile::Copy -e "copy('$<', '.');"


pampui$(SIS_TAIL):: pampui

pampui$(SIS_TAIL):: ../pampui/sis/pampui$(SIS_TAIL)
	perl -MFile::Copy -e "copy('$<', '.');"


clean:: mysqld httpd mod_php mod_python\
	connector connectorviews connectorui\
	httpdviews pampui 
