find_package(Gettext REQUIRED)
file(GLOB _po_files *.po)
GETTEXT_CREATE_TRANSLATIONS(xfce4-zorinmenulite-plugin.pot ALL ${_po_files})
