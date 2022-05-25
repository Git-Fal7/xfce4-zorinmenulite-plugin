#!/bin/sh


BASEDIR=../panel-plugin
WDIR=`pwd`


echo -n 'Preparing desktop file...'
cd ${BASEDIR}
rm -f zorinmenulite.desktop.in.h
rm -f zorinmenulite.desktop.in
cp zorinmenulite.desktop zorinmenulite.desktop.in
sed -e '/Name\[/ d' \
	-e '/Comment\[/ d' \
	-e 's/Name/_Name/' \
	-e 's/Comment/_Comment/' \
	-i zorinmenulite.desktop.in
intltool-extract --quiet --type=gettext/ini zorinmenulite.desktop.in
cd ${WDIR}
echo ' DONE'


echo -n 'Extracting messages...'
# Sort alphabetically to match cmake
xgettext --from-code=UTF-8 --c++ --keyword=_ --keyword=N_:1 --sort-output \
	--package-name='Zorin Menu Lite' --copyright-holder='The Zorin Menu Lite authors' \
	--output=xfce4-zorinmenulite-plugin.pot ${BASEDIR}/*.cpp ${BASEDIR}/*.h
echo ' DONE'


echo -n 'Merging translations...'
for POFILE in *.po;
do
	POLANG="${POFILE%%.*}"
	echo -n " $POLANG"
	msgunfmt "/usr/share/locale/$POLANG/LC_MESSAGES/gtk30.mo" > "gtk30-$POFILE"
	msgmerge --quiet --update --backup=none --compendium="gtk30-$POFILE" $POFILE xfce4-zorinmenulite-plugin.pot
	rm -f "gtk30-$POFILE"
done
echo ' DONE'


echo -n 'Merging desktop file translations...'
cd ${BASEDIR}
intltool-merge --quiet --desktop-style ${WDIR} zorinmenulite.desktop.in zorinmenulite.desktop
rm -f zorinmenulite.desktop.in.h
rm -f zorinmenulite.desktop.in
echo ' DONE'
