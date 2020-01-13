
SUBDIR := XPolyMonk

.PHONY: $(SUBDIR) libxputty  recurse

$(MAKECMDGOALS) recurse: $(SUBDIR)

clean:
	@rm -f ./libxputty/xputty/resources/mandala.png

libxputty:
	@exec $(MAKE) -j 1 -C $@ $(MAKECMDGOALS)

$(SUBDIR): libxputty
	@exec $(MAKE) -j 1 -C $@ $(MAKECMDGOALS)

