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

#
# Perl-stuff originally contributed by Hunor Demeter.
#


.PRECIOUS: %.pkg

.PHONY: sis sisx

#
# cpp, which in this context is likely to be Symbian's cpp, writes 
# binary files - that is, no \r\n but only \n  - which silently causes
# makesis to create bogus SIS-files. 
# 

%.pkg : %.pkgi
	@-cpp -P -I $(EPOC32INCLUDE) -DSIS_TAIL=$(SIS_TAIL) $< 2> $@.errors |\
	  perl -pe "s/PLATFORM/$(PLATFORM)/g; s/BUILD/$(BUILD)/g; s@EPOC32@$(EPOC32)@g; s@EPOCPATH@$(EPOCPATH)@g;" > $@
	@perl -e "open(ERR, '$@.errors') || exit; while(<ERR>){ next if /invalid preprocessing directive name/; print; } close(ERR); unlink '$@.errors'"


%.sis : %.pkg
	makesis $< $*.sis

%.sisx : %.pkg
ifneq ($(CERTIFICATE),)
	makesis $< $*.sis
	signsis $*.sis $*.sisx $(CERTIFICATE) $(KEY) $(PASSPHRASE)
	@-del $*.sis
else
	@echo No certificate specified. Not signing.
endif


all:: usage


clean::
	@perl -e 'foreach(@ARGV){ unlink <$${_}>}' $(DEFAULT_CLEAN) $(CLEAN)


$(TARGET)_$(PLATFORM)_$(BUILD).pkg : $(TARGET).pkgi $(PKG_DEPEND)
	@-cpp -P -I $(EPOC32INCLUDE) -DSIS_TAIL=$(SIS_TAIL) $< 2> $@.errors |\
	 perl -pe "s/PLATFORM/$(PLATFORM)/g; s/BUILD/$(BUILD)/g; s@EPOC32@$(EPOC32)@g; s@EPOCPATH@$(EPOCPATH)@g;" > $@
	@perl -e "open(ERR, '$@.errors') || exit; while(<ERR>){ next if /invalid preprocessing directive name/; print; } close(ERR); unlink '$@.errors'"

$(TARGET)_$(PLATFORM)_$(BUILD).sisx : $(SIS_DEPEND)

$(TARGET)_$(PLATFORM)_$(BUILD).sis : $(SIS_DEPEND)

no_final_copy\
$(FINAL_COPY):
	perl -MFile::Copy -e "copy('$(TARGET)_$(PLATFORM)_$(BUILD).$(SIS_SUFFIX)', '$(FINAL_COPY)');"


sisx:: $(TARGET)_$(PLATFORM)_$(BUILD).sisx $(FINAL_COPY) $(FINAL_TARGET)

sis:: $(TARGET)_$(PLATFORM)_$(BUILD).sis $(FINAL_COPY) $(FINAL_TARGET)

pkg: $(TARGET)_$(PLATFORM)_$(BUILD).pkg

usage:
	@echo Usage: "make PLATFORM=(armv5|armv5_abiv2|gcce) [BUILD=(udeb|urel)] [CERTIFICATE=<path to certificate file>] [KEY=<path to key file>] [PASSPHRASE=passphrase] (sisx|sis|pkg|clean)"
	@cmd /c echo.
	@echo sis  : Creates unsigned .sis-file.
	@echo sisx : Creates and signs sis-file.
	@echo pkg  : Creates .pkg-file from which sis-file can be created.
	@echo clean: Cleans all files.
	@cmd /c echo.
	@echo PLATFORM, BUILD, CERTIFICATE, KEY and PASSPHRASE can also be specified
	@echo as environment variables.
	@cmd /c echo.
	@echo NOTE: The actual binaries must have previously been built.
