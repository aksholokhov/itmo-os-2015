all: cat/cat revwords/revwords lenwords/lenwords

cat/cat revwords/revwords lenwords/lenwords:
	$(MAKE) -C $(dir $@) $(notdir $@)
