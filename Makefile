CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
TARGET = tarsau
INSTALL_DIR = /usr/local/bin

all: $(TARGET) install

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o -lm

main.o: main.c functions.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean uninstall

install:
	@if ! grep -q '$(INSTALL_DIR)' ~/.bashrc; then \
		echo 'export PATH=$$PATH:$(INSTALL_DIR)' >> ~/.bashrc; \
		echo "Executable path added to bashrc. Restart your terminal or run 'source ~/.bashrc' to apply changes."; \
	else \
		echo "Path already in bashrc."; \
	fi
	cp $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "Installed $(TARGET) to $(INSTALL_DIR)"

uninstall:
	@sed -i '/export PATH=\$$PATH:$(subst /,\/,$(INSTALL_DIR))/d' ~/.bashrc
	@echo "Executable path removed from bashrc. Restart your terminal or run 'source ~/.bashrc' to apply changes."

