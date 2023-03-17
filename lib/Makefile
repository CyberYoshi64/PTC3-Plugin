all:
	@cd Library && $(MAKE) && cd ..
	@cp Library/lib/*.a .

release:
	@cd Library && $(MAKE) lib/libctrpf.a && cd ..
	@cp Library/lib/*.a .

clean:
	@rm -rf *.a
	@cd Library && $(MAKE) clean && cd ..
