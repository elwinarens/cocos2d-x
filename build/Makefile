PLATFORM ?= linux

COCOS_ROOT = ..
COCOS_LIB = $(COCOS_ROOT)/cocos

all:

chipmunk:
	$(MAKE) -C $(COCOS_ROOT)/external/chipmunk/proj.$(PLATFORM)
chipmunk-clean:
	$(MAKE) -C $(COCOS_ROOT)/external/chipmunk/proj.$(PLATFORM) clean

box2d:
	$(MAKE) -C $(COCOS_ROOT)/external/Box2D/proj.$(PLATFORM)
box2d-clean:
	$(MAKE) -C $(COCOS_ROOT)/external/Box2D/proj.$(PLATFORM) clean

libextensions: chipmunk cocosdenshion  box2d
	$(MAKE) -C $(COCOS_ROOT)/extensions/proj.$(PLATFORM)
libextensions-clean:
	$(MAKE) -C $(COCOS_ROOT)/extensions/proj.$(PLATFORM) clean

libcocos2dx: libextensions
	$(MAKE) -C $(COCOS_LIB)/2d/proj.$(PLATFORM)
libcocos2dx-clean:
	$(MAKE) -C $(COCOS_LIB)/2d/proj.$(PLATFORM) clean

cocosdenshion: libcocos2dx
	$(MAKE) -C $(COCOS_LIB)/audio/proj.$(PLATFORM)
cocosdenshion-clean:
	$(MAKE) -C $(COCOS_LIB)/audio/proj.$(PLATFORM) clean

lua: libextensions
	$(MAKE) -C $(COCOS_LIB)/scripting/lua/proj.$(PLATFORM)
lua-clean:
	$(MAKE) -C $(COCOS_LIB)/scripting/lua/proj.$(PLATFORM) clean

hellocpp: libcocos2dx
	$(MAKE) -C $(COCOS_ROOT)/samples/Cpp/HelloCpp/proj.$(PLATFORM)
hellocpp-clean:
	$(MAKE) -C $(COCOS_ROOT)/samples/Cpp/HelloCpp/proj.$(PLATFORM) clean

testcpp: libcocos2dx libextensions
	$(MAKE) -C $(COCOS_ROOT)/samples/Cpp/TestCpp/proj.$(PLATFORM)
testcpp-clean:
	$(MAKE) -C $(COCOS_ROOT)/samples/Cpp/TestCpp/proj.$(PLATFORM) clean

simplegame: libcocos2dx
	$(MAKE) -C $(COCOS_ROOT)/samples/Cpp/SimpleGame/proj.$(PLATFORM)
simplegame-clean:
	$(MAKE) -C $(COCOS_ROOT)/samples/Cpp/SimpleGame/proj.$(PLATFORM) clean

all: chipmunk cocosdenshion libextensions libcocos2dx lua hellocpp testcpp simplegame
clean: libcocos2dx-clean box2d-clean chipmunk-clean cocosdenshion-clean libextensions-clean lua-clean hellocpp-clean testcpp-clean simplegame-clean

# Haven't yet got the lua projects working with emscripten
ifneq ($(PLATFORM),emscripten)

hellolua: libcocos2dx lua
	$(MAKE) -C $(COCOS_ROOT)/samples/Lua/HelloLua/proj.$(PLATFORM)
hellolua-clean:
	$(MAKE) -C $(COCOS_ROOT)/samples/Lua/HelloLua/proj.$(PLATFORM) clean

testlua: libcocos2dx lua
	$(MAKE) -C $(COCOS_ROOT)/samples/Lua/TestLua/proj.$(PLATFORM)
testlua-clean:
	$(MAKE) -C $(COCOS_ROOT)/samples/Lua/TestLua/proj.$(PLATFORM) clean
	
all: hellolua testlua
clean: hellolua-clean testlua-clean
endif

.PHONY: all clean
