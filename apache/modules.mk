mod_cool_framework.la: mod_cool_framework.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_cool_framework.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_cool_framework.la
