all:

README.html: README.md
	markdown $< > $@

README.md: dlib.h
	gendoc.pl $< > $@
