# Makefile for NEdit text editor documentation
#
#  
#  NEdit help documentation and internal help code using one common
#  source, help.etx, a Structure Enhanced TEXT document.
#

instructions:
	@ echo ""
	@ echo "This make file is intended for NEdit developers only."
	@ echo ""
	@ echo "It uses a perl program (setext) to extract the various forms"
	@ echo "of the NEdit help documentation and internal help code using"
	@ echo "one common source, help.etx, a Structure Enhanced TEXT document."
	@ echo ""
	@ echo "The following are the main targets which generate files for"
	@ echo "NEdit development. They are:"
	@ echo ""
	@ echo "  help - generates NEdit help code (help.h, help_data.h)"
	@ echo "  doc  - generates various forms of NEdit documentation"
	@ echo "  all  - generates all the files"
	@ echo ""
	@ echo "Remember to specify the VERSION macro on the make command"
	@ echo "or as an environment variable so that the NEdit version"
	@ echo "gets placed appropriately. For example, the following"
	@ echo "command creates all the files for NEdit version 5.3"
	@ echo ""
	@ echo "   make VERSION='NEdit 5.3' all"
	@ echo ""
	@ echo "When the version is not specified, the default value will"
	@ echo "be 'NEdit release of <currentDate>'"
	@ echo ""
	
version:
	@ if [ "$(VERSION)" = "" ]; then \
	      echo "NEdit release of `date +'%b %e, %Y'`" > .version; \
	  else \
	      echo "$(VERSION)" > .version; \
	  fi
help: version help.etx
	@ echo "Creating NEdit help code..."
	@ setext -m -v version="`cat .version`" help.etx
	@ mv help.h ../source
	@ mv help_data.h ../source

nedit.html: help.etx
	@ echo "Creating NEdit HTML documentation..."
	@ setext -v version="`cat .version`" help.etx nedit.html

html/nedit.html: help.etx
	@ echo "Creating NEdit HTML website documentation..."
	@ if [ ! -d html ]; then mkdir html; fi
	@ if [ -d html ]; then \
	      cd html; \
	      ../setext -S -v version="`cat ../.version`" ../help.etx nedit.html; \
	  else \
	      echo "** Unable to create html directory to hold NEdit documentation"; \
	  fi

nedit.doc: help.etx
	@ echo "Creating NEdit plain text documenation..."
	@ setext -c NEDITDOC -v version="`cat .version`" help.etx nedit.doc

doc: version nedit.doc nedit.html html/nedit.html 

all: help doc

clean:
	@ echo "Removing generated NEdit documentation..."
	@ rm -rf nedit.html html nedit.doc .version

