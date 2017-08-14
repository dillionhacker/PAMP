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

include ../tools/defines.mk


TARGET=		httpd

PKG_DEPEND=	../public/mws/versionhttpd.hrh\
		httpd.files

SIS_DEPEND=	$(EPOC32RELEASE)/httpds22.exe\
		$(EPOC32RELEASE)/httpdc22.dll\
		$(EPOC32RELEASE)/libhttpd22.dll\
		$(EPOC32RELEASE)/libaprutil1.dll\
		$(EPOC32RELEASE)/libapr1.dll\
		httpdconf.$(MAKECMDGOALS)\
		htdocs.$(MAKECMDGOALS)\

CLEAN=		httpd*.pkg\
		httpdconf.pkg\
		htdocs.pkg

HTTPDCONF_DEPEND=\
		data\apache\conf\default\httpd-autoindex.conf\
		data\apache\conf\default\httpd-dav.conf\
		data\apache\conf\default\httpd-default.conf\
		data\apache\conf\default\httpd-info.conf\
		data\apache\conf\default\httpd-languages.conf\
		data\apache\conf\default\httpd-manual.conf\
		data\apache\conf\default\httpd-mpm.conf\
		data\apache\conf\default\httpd-multilang-errordoc.conf\
		data\apache\conf\default\httpd-ssl.conf\
		data\apache\conf\default\httpd-userdir.conf\
		data\apache\conf\default\httpd-vhosts.conf\
		data\apache\conf\default\httpd.conf\
		data\apache\conf\default\magic\
		data\apache\conf\default\mime.types\
		data\apache\conf\extra\httpd-autoindex.conf \
		data\apache\conf\extra\httpd-dav.conf\
		data\apache\conf\extra\httpd-default.conf\
		data\apache\conf\extra\httpd-info.conf\
		data\apache\conf\extra\httpd-languages.conf\
		data\apache\conf\extra\httpd-manual.conf\
		data\apache\conf\extra\httpd-mpm.conf\
		data\apache\conf\extra\httpd-multilang-errordoc.conf\
		data\apache\conf\extra\httpd-ssl.conf\
		data\apache\conf\extra\httpd-userdir.conf\
		data\apache\conf\extra\httpd-vhosts.conf\
		data\apache\conf\extra\httpd-s60.conf\
		data\apache\conf\charset.conv\
		data\apache\conf\httpd.conf\
		data\apache\conf\magic\
		data\apache\conf\mime.types\
		data\apache\conf\openssl.cnf\
		data\apache\conf\users.txt\
		data\apache\logs\error.log

include ../tools/targets.mk


httpdconf.sis httpdconf.sisx: $(HTTPDCONF_DEPEND)
