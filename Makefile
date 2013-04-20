# Top-level Makefile

default:
# Make libraries
	cd lib/ann_1.1_char; $(MAKE) linux-g++
	cd lib/ann_1.1; $(MAKE) linux-g++
	cd lib/imagelib; $(MAKE)
# Make program
	cd VocabLib; $(MAKE)
	cd VocabLearn; $(MAKE)
	cd VocabBuildDB; $(MAKE)
	cd VocabMatch; $(MAKE)
	cd src; $(MAKE)

clean:
	cd lib/ann_1.1_char; $(MAKE) clean
	cd lib/ann_1.1; $(MAKE) clean
	cd lib/imagelib; $(MAKE) clean	
	cd VocabLib; $(MAKE) clean
	cd VocabLearn; $(MAKE) clean
	cd VocabBuildDB; $(MAKE) clean
	cd VocabMatch; $(MAKE) clean
	cd src; $(MAKE) clean
#	rm -f bin/bundler bin/KeyMatchFull
