SUBDIRS := ami 

all: subdirs

subdirs:
	for n in $(SUBDIRS); do $(MAKE) -C $$n all|| exit 1; done

distclean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n distclean; done

clean:
	rm -f libvendor.a
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean; done

.depend:
	for n in $(SUBDIRS); do $(MAKE) -C $(SUBDIRS) .depend
