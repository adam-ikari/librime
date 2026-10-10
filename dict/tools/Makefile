# 词库生成工具。全部显式调用，不接进 CMake —— 接进 CMake 就把「生成词库」
# 和「配置引擎」重新绑回同一张编译图，而 librime 运行时只读 dict/ 里的数据。
#
#   make toolchain        构建 opencc 的生成器（host Python + host opencc_dict）
#   make essay            重新生成 essay.txt
#   make dict             重新生成 luna_pinyin.dict.yaml
#   make ocd2             重新生成 opencc 二进制字形表
#   make all              以上三个产物
#   make check            重新生成并与 dict/ 已提交内容逐字节比对
#   make package          打成发布 tarball
#
# check 是唯一的验收：diff 非空就是脚本错了，不许改数据去「对齐」。

TOOLS := $(CURDIR)
DICT  := $(abspath $(TOOLS)/..)
BUILD := $(TOOLS)/.build
OUT   ?= $(BUILD)/out
PKG   := $(BUILD)/pkg

OPENCC_SRC   := $(abspath $(DICT)/../deps/opencc)
OPENCC_BUILD := $(BUILD)/opencc
OPENCC_CLI   := $(OPENCC_BUILD)/src/tools/opencc
OPENCC_DICT  := $(OPENCC_BUILD)/src/tools/opencc_dict

export OPENCC_CLI
export OPENCC_BUILD
export OPENCC_SRC
export OPENCC_CONFIG_DIR := $(OPENCC_SRC)/data/config

PY ?= python3
VERSION := $(shell cat $(DICT)/VERSION)
NAME := term-ime-dict-$(VERSION)

.PHONY: all check clean dict essay ocd2 package toolchain

all: $(OUT)/essay.txt $(OUT)/luna_pinyin.dict.yaml $(OUT)/opencc

# The generator toolchain is the one place a host Python and a host-executed
# opencc_dict are allowed: it builds opencc WITH its data step.
toolchain: $(OPENCC_DICT)

$(OPENCC_DICT) $(OPENCC_CLI):
	cmake -S $(OPENCC_SRC) -B $(OPENCC_BUILD) \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DBUILD_SHARED_LIBS=OFF \
	  -DBUILD_TESTING=OFF -DENABLE_GTEST=OFF \
	  -DUSE_SYSTEM_MARISA=OFF \
	  -DOPENCC_BUILD_DATA=ON -DOPENCC_BUILD_TOOLS=ON
	cmake --build $(OPENCC_BUILD)

essay: $(OUT)/essay.txt
dict: $(OUT)/luna_pinyin.dict.yaml
ocd2: $(OUT)/opencc

$(OUT)/essay.txt: upgrade_essay.py protected_chars.txt t2s_char.json sources.lock | $(OUT) toolchain
	$(PY) upgrade_essay.py --out $@

$(OUT)/luna_pinyin.dict.yaml: convert_dict.py pinyin_fixes.txt protected_chars.txt t2s_char.json sources.lock | $(OUT) toolchain
	$(PY) convert_dict.py --out $@

$(OUT)/opencc: rebuild_ocd2.sh | $(OUT) toolchain
	sh rebuild_ocd2.sh $@

$(OUT):
	mkdir -p $(OUT)

# Byte-for-byte against what is committed. Three separate diffs so a failure
# says which product drifted.
check: all
	@fail=0; \
	for f in essay.txt luna_pinyin.dict.yaml; do \
	  if diff -u $(DICT)/$$f $(OUT)/$$f > $(BUILD)/$$f.diff; then \
	    echo "OK   $$f"; \
	  else \
	    echo "DIFF $$f -- see $(BUILD)/$$f.diff"; fail=1; \
	  fi; \
	done; \
	if diff -ru $(DICT)/opencc $(OUT)/opencc > $(BUILD)/opencc.diff; then \
	  echo "OK   opencc/"; \
	else \
	  echo "DIFF opencc/ -- see $(BUILD)/opencc.diff"; fail=1; \
	fi; \
	exit $$fail

package: all
	rm -rf $(PKG)
	mkdir -p $(PKG)/share/term-ime/rime-data
	cp $(OUT)/essay.txt $(OUT)/luna_pinyin.dict.yaml $(PKG)/share/term-ime/rime-data/
	cp -r $(OUT)/opencc $(PKG)/share/term-ime/rime-data/
	cp $(DICT)/LICENSE $(DICT)/VERSION $(PKG)/share/term-ime/rime-data/
	tar -czf $(BUILD)/$(NAME).tar.gz -C $(PKG) share
	cd $(BUILD) && sha256sum $(NAME).tar.gz > $(NAME).tar.gz.sha256
	@echo "$(BUILD)/$(NAME).tar.gz (+ .sha256)"

clean:
	rm -rf $(BUILD)
