
OUT := out

subdirs := libremotecameracontrol frontend 

.PHONY: all clean $(subdirs)

all: $(OUT) $(subdirs)

clean: $(subdirs)

objs: $(subdirs)

$(subdirs):
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(OUT):
	mkdir -p $(OUT)
