PKG_BUILD_DIR:=$(BUILDPATH)/ipk
PKGARCH:=arm
IPKG_BUILD:= $(BUILDPATH)/include/ipkg-build -c -o 0 -g 0
addfield=$(if $(strip $(2)),$(1): $(2))
IDIR_$(PKG_NAME):=$(PKG_BUILD_DIR)/packages_$(PKGARCH)/$(PKG_NAME)
PDIR_$(PKG_NAME):=$(PKG_BUILD_DIR)/ipk
IPKG_$(PKG_NAME):=$(PKG_BUILD_DIR)/ipk/$(PKG_NAME)_$(VERSION)_$(PKGARCH).ipk
INSTALL_BIN:=install -m0755
INSTALL_DIR:=install -d -m0755
INSTALL_DATA:=install -m0755
INSTALL_CONF:=install -m0600

empty:=
space:= $(empty) $(empty)
comma:=,
_addsep=$(word 1,$(1))$(foreach w,$(wordlist 2,$(words $(1)),$(1)),$(strip $(2) $(w)))
_cleansep=$(subst $(space)$(2)$(space),$(2)$(space),$(1))
addfield=$(if $(strip $(2)), echo "$(1): $(2)" >> $(3);)
mergelist=$(call _cleansep,$(call _addsep,$(1),$(comma)),$(comma))

define Package/$(PKG_NAME)/CONTROL
echo "Package: $(PKG_NAME)" > $(1); \
echo "Version: $(VERSION)" >> $(1);\
echo "Filename: $(PKG_NAME)_${VERSION}_${PKGARCH}.ipk" >> $(1);\
$(call addfield,Conflicts,$(call mergelist,$(CONFLICTS)),$(1))\
$(call addfield,Provides,$(PROVIDES),$(1))\
$(call addfield,Source,$(SOURCE),$(1))\
$(call addfield,License,$(PKG_LICENSE),$(1))\
$(call addfield,LicenseFiles,$(PKG_LICENSE_FILES),$(1))\
$(call addfield,Section,$(SECTION),$(1))\
$(call addfield,Require-User,$(USERID),$(1))\
$(if $(filter hold,$(PKG_FLAGS)),echo "Status: unknown hold not-installed" >> $(1);)\
$(if $(filter essential,$(PKG_FLAGS)),echo "Essential: yes" >> $(1);)\
$(if $(MAINTAINER),echo "Maintainer: $(MAINTAINER)" >> $(1);)\
$(call addfield,Architecture,$(PKGARCH),$(1))\
$(call addfield,Description,$(DESCRIPTION),$(1))\
$(call addfield,Depends,$(DEPENDS),$(1))\
$(call addfield,Installed-Size,0,$(1))\
$(call addfield,Tags,$(TAGS),$(1))
endef

ifndef Package/$(PKG_NAME)/postinst
define Package/$(PKG_NAME)/POSTINST
$(INSTALL_DATA) $(BUILDPATH)/include/postinst $(1);
endef
else
define Package/$(PKG_NAME)/POSTINST
echo "$(Package/$(PKG_NAME)/postinst)" > $(1);
chmod 755 $(1);
endef
endif
ifdef Package/$(PKG_NAME)/conffiles
define Package/$(PKG_NAME)/CONFFILES
$(if $(strip $(Package/$(PKG_NAME)/conffiles)), echo "$(Package/$(PKG_NAME)/conffiles)" > $(1);)
endef
endif
ifndef Package/$(PKG_NAME)/prerm
define Package/$(PKG_NAME)/PRERM
$(INSTALL_DATA) $(BUILDPATH)/include/prerm $(1);
endef
else
define Package/$(PKG_NAME)/PRERM
echo "$(Package/$(PKG_NAME)/prerm)" > $(1);
chmod 755 $(1);
endef
endif

define BuildTargetipkg
	-rm -rf $(IPKG_$(PKG_NAME)) $(IDIR_$(PKG_NAME)); \
	mkdir -p $(PKG_BUILD_DIR) $(PKG_BUILD_DIR)/packages_$(PKGARCH) $(IDIR_$(PKG_NAME));\
	mkdir -p $(PDIR_$(PKG_NAME)); \
  	mkdir -p $(IDIR_$(PKG_NAME))/CONTROL; \
  	$(call Package/$(PKG_NAME)/CONTROL,$(IDIR_$(PKG_NAME))/CONTROL/control) \
	printf "Description: "; echo "$(Package/$(PKG_NAME)/description)" | sed -e 's,^[[:space:]]*, ,g' >> $(IDIR_$(PKG_NAME))/CONTROL/control;\
	chmod 644 $(IDIR_$(PKG_NAME))/CONTROL/control; \
  	$(call Package/$(PKG_NAME)/install,$(IDIR_$(PKG_NAME))) \
	$(call Package/$(PKG_NAME)/POSTINST,$(IDIR_$(PKG_NAME))/CONTROL/postinst) \
	$(call Package/$(PKG_NAME)/PRERM,$(IDIR_$(PKG_NAME))/CONTROL/prerm) \
	$(call Package/$(PKG_NAME)/CONFFILES,$(IDIR_$(PKG_NAME))/CONTROL/conffiles) \
    $(IPKG_BUILD) $(IDIR_$(PKG_NAME)) $(PDIR_$(PKG_NAME));
endef
