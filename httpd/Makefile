export CC=gcc
export CFLAGS=-std=c99 -Werror -Wall -Wextra -Wvla -pedantic

TARGET=httpd

.PHONY: all check clean

all: $(TARGET)

$(TARGET):
	$(MAKE) -C src

check: $(TARGET)
	@test -d env || python3 -m venv env
	@./env/bin/pip install -q -r tests/requirements.txt
	@cd tests && ../env/bin/pytest -v

clean:
	$(MAKE) -C src clean
	$(RM) $(TARGET)
	rm -rf env tests/__pycache__ tests/*.log tests/.pytest_cache HTTPd.log
